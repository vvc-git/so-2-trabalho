#include <machine/machine.h>
#include <machine/riscv/riscv_gem.h>
#include <system.h>
#include <time.h>

__BEGIN_SYS

SiFive_U_NIC::~SiFive_U_NIC()
{
    db<SiFive_U_NIC>(TRC) << "~SiFive_U_NIC()" << endl;
}

int SiFive_U_NIC::send(const Address & dst, const Protocol & prot, const void * data, unsigned int size)
{
    // Wait for a buffer to become free and seize it
    unsigned int i = _tx_cur;
    for(bool locked = false; !locked; ) {
        for(; _tx_ring[i].status & Tx_Desc::OWN; ++i %= TX_BUFS);
        locked = _tx_buffer[i]->lock();
    }
    _tx_cur = (i + 1) % TX_BUFS; // _tx_cur and _rx_cur are simple accelerators to avoid scanning the ring buffer from the beginning.
                                 // Losing a write in a race condition is assumed to be harmless. The FINC + CAS alternative seems too expensive.
    Tx_Desc * desc = &_tx_ring[i];
    Buffer * buf = _tx_buffer[i];

    db<SiFive_U_NIC>(TRC) << "SiFive_U_NIC::send(s=" << _configuration.address << ",d=" << dst << ",p=" << hex << prot << dec << ",d=" << data << ",s=" << size << ")" << endl;

    // Assemble the Ethernet frame
    new (buf->frame()) Frame(_configuration.address, dst, prot, data, size);

    desc->size = -(size + sizeof(Header)); // 2's comp.

    // Status must be set last, since it can trigger a send
    desc->status = Tx_Desc::OWN | Tx_Desc::STP | Tx_Desc::ENP;

    
    reg(NWCTRL) |= TXSTART;

    _statistics.tx_packets++;
    _statistics.tx_bytes += size;

    // Wait for packet to be sent
    // while(desc->status & Tx_Desc::OWN);

    db<SiFive_U_NIC>(INF) << "SiFive_U_NIC::send:desc[" << i << "]=" << desc << " => " << *desc << endl;

    buf->unlock();

    return size;
}

int SiFive_U_NIC::send(Buffer * buf)
{
    unsigned int size = 0;

    for(Buffer::Element * el = buf->link(); el; el = el->next()) {
        buf = el->object();
        Tx_Desc * desc = reinterpret_cast<Tx_Desc *>(buf->back());

        db<SiFive_U_NIC>(TRC) << "SiFive_U_NIC::send(buf=" << buf << ")" << endl;

        db<SiFive_U_NIC>(INF) << "SiFive_U_NIC::send:buf=" << buf << " => " << *buf << endl;

        desc->size = -(buf->size() + sizeof(Header)); // 2's comp.

        // Status must be set last, since it can trigger a send
        desc->status = Tx_Desc::OWN | Tx_Desc::STP | Tx_Desc::ENP;

        reg(NWCTRL) |= TXSTART;

        size += buf->size();

        _statistics.tx_packets++;
        _statistics.tx_bytes += buf->size();

        db<SiFive_U_NIC>(INF) << "SiFive_U_NIC::send:desc=" << desc << " => " << *desc << endl;

        // Wait for packet to be sent and unlock the respective buffer
        while(desc->status & Tx_Desc::OWN);
        buf->unlock();
    }

    return size;
}

SiFive_U_NIC::Buffer *
SiFive_U_NIC::alloc(const Address &dst, const Protocol &prot, unsigned int once,
                    unsigned int always, unsigned int payload) {
  db<SiFive_U_NIC>(TRC) << "SiFive_U_NIC::alloc(s=" << _configuration.address << ",d=" << dst << ",p=" << hex << prot << dec << ",on=" << once << ",al=" << always << ",pl=" << payload << ")" << endl;

    int max_data = MTU - always;

    if((payload + once) / max_data > TX_BUFS) {
        db<SiFive_U_NIC>(WRN) << "SiFive_U_NIC::alloc: sizeof(Network::Packet::Data) > sizeof(NIC::Frame::Data) * TX_BUFS!" << endl;
        return 0;
    }

    Buffer::List pool;

    // Calculate how many frames are needed to hold the transport PDU and allocate enough buffers
    for(int size = once + payload; size > 0; size -= max_data) {
        // Wait for the next buffer to become free and seize it
        unsigned int i = _tx_cur;
        for(bool locked = false; !locked; ) {
            for(; _tx_ring[i].status & Tx_Desc::OWN; ++i %= TX_BUFS);
            locked = _tx_buffer[i]->lock();
        }
        _tx_cur = (i + 1) % TX_BUFS;
        Tx_Desc * desc = &_tx_ring[i];
        Buffer * buf = _tx_buffer[i];

        // Initialize the buffer and assemble the Ethernet Frame Header
        buf->fill((size > max_data) ? MTU : size + always, _configuration.address, dst, prot);

        db<SiFive_U_NIC>(INF) << "SiFive_U_NIC::alloc:desc[" << i << "]=" << desc << " => " << *desc << endl;
        db<SiFive_U_NIC>(INF) << "SiFive_U_NIC::alloc:buf=" << buf << " => " << *buf << endl;

        pool.insert(buf->link());
    }

    return pool.head()->object();
}

void SiFive_U_NIC::reset() {
  reg(NWCFG) = 0x00080000;
  reg(NWSR) = 0x00000006;
  reg(DMACR) = 0x00020784;
  reg(IMR) = 0x07ffffff;
  reg(TXPAUSE) = 0x0000ffff;
  reg(DESCONF) = 0x02D00111;
  reg(DESCONF2) = 0x2ab10000 | reg(JUMBOMAXLEN);
  reg(DESCONF5) = 0x002f2045;
  reg(DESCONF6) = DESCONF6_64B_MASK;
  reg(ISR) = 0x00000000;
}

__END_SYS
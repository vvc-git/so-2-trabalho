#include <machine/machine.h>
#include <machine/riscv/riscv_gem.h>
#include <system.h>
#include <time.h>

__BEGIN_SYS

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
#include <machine/machine.h>
#include <machine/riscv/riscv_gem.h>
#include <system.h>
#include <time.h>

__BEGIN_SYS

SiFive_U_NIC::Device SiFive_U_NIC::_devices[UNITS];

SiFive_U_NIC::~SiFive_U_NIC() {
  db<SiFive_U_NIC>(TRC) << "~SiFive_U_NIC()" << endl;
}

int SiFive_U_NIC::send(const Address &dst, const Protocol &prot,
                       const void *data, unsigned int size) {

  db<SiFive_U_NIC>(TRC) << "SiFive_U_NIC::send(s=" << _configuration.address
                        << ",d=" << dst << ",p=" << hex << prot << dec
                        << ",d=" << data << ",s=" << size << ")" << endl;

  // Wait for a buffer to become free and seize it
  unsigned int i = _tx_cur;
  for (bool locked = false; !locked;) {
    for (; _tx_ring[i].ctrl & Tx_Desc::OWN; ++i %= TX_BUFS) {
      db<SiFive_U_NIC>(INF) << "SiFive_U_NIC::send:desc[" << i << "]=" << &_tx_ring[i]
                            << " => " << _tx_ring[i] << endl;
    }
    locked = _tx_buffer[i]->lock();
  }
  _tx_cur = (i + 1) %
            TX_BUFS; // _tx_cur and _rx_cur are simple accelerators to avoid
                     // scanning the ring buffer from the beginning. Losing a
                     // write in a race condition is assumed to be harmless. The
                     // FINC + CAS alternative seems too expensive.

  db<SiFive_U_NIC>(INF) << "SiFive_U_NIC::send:buf=" << _tx_buffer[i]
                        << " => " << *_tx_buffer[i] << endl;
  
  Tx_Desc *desc = &_tx_ring[i];
  Buffer *buf = _tx_buffer[i];

  db<SiFive_U_NIC>(TRC) << "SiFive_U_NIC::send(s=" << _configuration.address
                        << ",d=" << dst << ",p=" << hex << prot << dec
                        << ",d=" << data << ",s=" << size << ")" << endl;

  // Assemble the Ethernet frame
  new (buf->frame()) Frame(_configuration.address, dst, prot, data, size);

  desc->update_size(size + sizeof(Header));

  // Status must be set last, since it can trigger a send
  desc->ctrl |= Tx_Desc::OWN;

  reg(NWCTRL) |= TXSTART;

  _statistics.tx_packets++;
  _statistics.tx_bytes += size;

  // Wait for packet to be sent
  // while(desc->status & Tx_Desc::OWN);

  db<SiFive_U_NIC>(INF) << "SiFive_U_NIC::send:desc[" << i << "]=" << desc
                        << " => " << *desc << endl;

  buf->unlock();

  return size;
}

int SiFive_U_NIC::send(Buffer *buf) {
  unsigned int size = 0;

  for (Buffer::Element *el = buf->link(); el; el = el->next()) {
    buf = el->object();
    Tx_Desc *desc = reinterpret_cast<Tx_Desc *>(buf->back());

    db<SiFive_U_NIC>(TRC) << "SiFive_U_NIC::send(buf=" << buf << ")" << endl;

    db<SiFive_U_NIC>(INF) << "SiFive_U_NIC::send:buf=" << buf << " => " << *buf
                          << endl;

    desc->update_size(buf->size() + sizeof(Header)); // 2's comp.

    // Status must be set last, since it can trigger a send
    desc->ctrl |= Tx_Desc::OWN;

    reg(NWCTRL) |= TXSTART;

    size += buf->size();

    _statistics.tx_packets++;
    _statistics.tx_bytes += buf->size();

    db<SiFive_U_NIC>(INF) << "SiFive_U_NIC::send:desc=" << desc << " => "
                          << *desc << endl;

    // Wait for packet to be sent and unlock the respective buffer
    // DONE BY INTERRUPT HANDLER
  }

  return size;
}

SiFive_U_NIC::Buffer *
SiFive_U_NIC::alloc(const Address &dst, const Protocol &prot, unsigned int once,
                    unsigned int always, unsigned int payload) {
  db<SiFive_U_NIC>(TRC) << "SiFive_U_NIC::alloc(s=" << _configuration.address
                        << ",d=" << dst << ",p=" << hex << prot << dec
                        << ",on=" << once << ",al=" << always
                        << ",pl=" << payload << ")" << endl;

  int max_data = MTU - always;

  if ((payload + once) / max_data > TX_BUFS) {
    db<SiFive_U_NIC>(WRN)
        << "SiFive_U_NIC::alloc: sizeof(Network::Packet::Data) > "
           "sizeof(NIC::Frame::Data) * TX_BUFS!"
        << endl;
    return 0;
  }

  Buffer::List pool;

  // Calculate how many frames are needed to hold the transport PDU and allocate
  // enough buffers
  for (int size = once + payload; size > 0; size -= max_data) {
    // Wait for the next buffer to become free and seize it
    unsigned int i = _tx_cur;
    for (bool locked = false; !locked;) {
      for (; _tx_ring[i].ctrl & Tx_Desc::OWN; ++i %= TX_BUFS)
        ;
      locked = _tx_buffer[i]->lock();
    }
    _tx_cur = (i + 1) % TX_BUFS;
    Tx_Desc *desc = &_tx_ring[i];
    Buffer *buf = _tx_buffer[i];

    // Initialize the buffer and assemble the Ethernet Frame Header
    buf->fill((size > max_data) ? MTU : size + always, _configuration.address,
              dst, prot);

    db<SiFive_U_NIC>(INF) << "SiFive_U_NIC::alloc:desc[" << i << "]=" << desc
                          << " => " << *desc << endl;
    db<SiFive_U_NIC>(INF) << "SiFive_U_NIC::alloc:buf=" << buf << " => " << *buf
                          << endl;

    pool.insert(buf->link());
  }

  return pool.head()->object();
}

void SiFive_U_NIC::receive() {
  TSC::Time_Stamp ts =
      (Buffer::Metadata::collect_sfdts) ? TSC::time_stamp() : 0;

  db<SiFive_U_NIC>(TRC) << "SiFive_U_NIC::receive()" << endl;

  for (unsigned int count = RX_BUFS, i = _rx_cur;
       count && !(_rx_ring[i].ctrl & Rx_Desc::OWN);
       count--, ++i %= RX_BUFS, _rx_cur = i) {
    // NIC received a frame in _rx_buffer[_rx_cur], let's check if it has
    // already been handled
    if (_rx_buffer[i]->lock()) { // if it wasn't, let's handle it
      Buffer *buf = _rx_buffer[i];
      Rx_Desc *desc = &_rx_ring[i];
      Frame *frame = buf->frame();

      // For the upper layers, size will represent the size of frame->data<T>()
      buf->size((desc->ctrl & Rx_Desc::SIZE_MASK) - sizeof(Header) - sizeof(CRC));

      db<SiFive_U_NIC>(TRC)
          << "SiFive_U_NIC::receive: frame = " << *frame << endl;
      db<SiFive_U_NIC>(INF) << "SiFive_U_NIC::handle_int: desc[" << i
                            << "] = " << desc << " => " << *desc << endl;

      if ((Traits<SiFive_U_NIC>::EXPECTED_SIMULATION_TIME > 0) &&
          (frame->header()->src() ==
           _configuration.address)) { // Multicast on QEMU seems to replicate
                                      // all sent packets
        free(buf);
        continue;
      }

      if (Buffer::Metadata::collect_sfdts)
        buf->sfdts = ts;
      if (!notify(frame->header()->prot(),
                  buf)) // No one was waiting for this frame, so let it free for
                        // receive()
        free(buf);
    }
  }
}

bool SiFive_U_NIC::reconfigure(const Configuration *c = 0) {
  db<SiFive_U_NIC>(TRC) << "SiFive_U_NIC::reconfigure(c=" << c << ")" << endl;

  bool ret = false;

  if (!c) {
    db<SiFive_U_NIC>(TRC) << "SiFive_U_NIC::reconfigure: reseting!" << endl;
    CPU::int_disable();
    reset();
    new (&_statistics) Statistics; // reset statistics
    CPU::int_enable();
  } else {
    db<SiFive_U_NIC>(INF) << "SiFive_U_NIC::reconfigure: configuration = " << *c
                          << ")" << endl;

    if (c->selector & Configuration::ADDRESS) {
      db<SiFive_U_NIC>(WRN)
          << "SiFive_U_NIC::reconfigure: address changed only in the mediator!)"
          << endl;
      _configuration.address = c->address;
      ret = true;
    }

    if (c->selector & Configuration::TIMER) {
      if (c->timer_frequency) {
        db<SiFive_U_NIC>(WRN)
            << "SiFive_U_NIC::reconfigure: timer frequency cannot be changed!)"
            << endl;
        ret = false;
      }
    }
  }

  return ret;
}

void SiFive_U_NIC::free(Buffer *buf) {
  db<SiFive_U_NIC>(TRC) << "SiFive_U_NIC::free(buf=" << buf << ")" << endl;

  db<SiFive_U_NIC>(INF) << "SiFive_U_NIC::free:buf=" << buf << " => " << *buf
                        << endl;

  for (Buffer::Element *el = buf->link(); el; el = el->next()) {
    buf = el->object();
    Rx_Desc *desc = reinterpret_cast<Rx_Desc *>(buf->back());

    _statistics.rx_packets++;
    _statistics.rx_bytes += buf->size();

    // Release the buffer to the NIC
    desc->update_size(sizeof(Frame));
    desc->ctrl |= Rx_Desc::OWN;          // Owned by NIC

    // Release the buffer to the OS
    buf->unlock();

    db<SiFive_U_NIC>(INF) << "SiFive_U_NIC::free:desc=" << desc << " => "
                          << *desc << endl;
  }
}

void SiFive_U_NIC::reset() {
  // TODO: check if there more registers to reset
  reg(NWCFG) = _64_DBUS_WIDTH_SIZE;
  reg(NWCTRL) = CLEAR_STATS_REGS;
  reg(TXSTATUS) = TX_STAT_ALL;
  reg(RXSTATUS) = RX_STAT_ALL;
  reg(IDR) = INTR_ALL;
  reg(HASHL) = 0;
  reg(HASHH) = 0;
  reg(TXQBASE) = 0;
  reg(RXQBASE) = 0;

  reg(NWCFG) |= MDC_DIV_48;
  reg(NWCTRL) = CTRL_MGMT_PORT_EN;

  // Set the MAC address
  unsigned int low = reg(SPADDR1L);
  unsigned int high = reg(SPADDR1H);
  _configuration.address[0] = low & 0xFF;
  _configuration.address[1] = (low >> 8) & 0xFF;
  _configuration.address[2] = (low >> 16) & 0xFF;
  _configuration.address[3] = (low >> 24) & 0xFF;
  _configuration.address[4] = high & 0xFF;
  _configuration.address[5] = (high >> 8) & 0xFF;
  
  db<SiFive_U_NIC>(INF) << "SiFive_U_NIC::reset: MAC=" << _configuration.address << endl;
}

int SiFive_U_NIC::receive(Address * src, Protocol * prot, void * data, unsigned int size)
{
    db<SiFive_U_NIC>(TRC) << "SiFive_U_NIC::receive(s=" << *src << ",p=" << hex << *prot << dec << ",d=" << data << ",s=" << size << ") => " << endl;

    // Wait for a received frame and seize it
    unsigned int i = _rx_cur;
    for(bool locked = false; !locked; ) {
        for(; _rx_ring[i].ctrl & Rx_Desc::OWN; ++i %= RX_BUFS);
        locked = _rx_buffer[i]->lock();
    }
    _rx_cur = (i + 1) % RX_BUFS;
    Buffer * buf = _rx_buffer[i];
    Rx_Desc * desc = &_rx_ring[i];

    // Disassemble the Ethernet frame
    Frame * frame = buf->frame();
    *src = frame->src();
    *prot = frame->prot();

    // For the upper layers, size will represent the size of frame->data<T>()
    buf->size((desc->ctrl & Rx_Desc::SIZE_MASK) - sizeof(Header) - sizeof(CRC));

    // Copy the data
    memcpy(data, frame->data<void>(), (buf->size() > size) ? size : buf->size());

    // Release the buffer to the NIC
    desc->ctrl = Rx_Desc::OWN;

    _statistics.rx_packets++;
    _statistics.rx_bytes += buf->size();

    db<SiFive_U_NIC>(INF) << "SiFive_U_NIC::receive:desc[" << i << "]=" << desc << " => " << *desc << endl;

    int tmp = buf->size();

    buf->unlock();

    return tmp;
}

void SiFive_U_NIC::handle_int() {
  Reg32 status = reg(ISR);
  reg(ISR) = status;

  if ((status & INT_RXCMPL) != 0)
    receive();

  if ((status & INT_TXCMPL) != 0) {
    unsigned int i = _tx_cur;
    for (; _tx_ring[i].ctrl & Tx_Desc::OWN; ++i %= TX_BUFS) {
      _tx_ring[i].ctrl |= Tx_Desc::OWN;
      _tx_buffer[i]->unlock();
    };
  };
}

void SiFive_U_NIC::int_handler(unsigned int interrupt) {
  SiFive_U_NIC *dev = get_by_interrupt(interrupt);

  if (!dev) {
    db<SiFive_U_NIC>(WRN)
        << "SiFive_U_NIC::int_handler: handler not installed!\n";
    return;
  }

  dev->handle_int();
}

__END_SYS
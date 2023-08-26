#include <machine/machine.h>
#include <machine/riscv/riscv_gem.h>
#include <system.h>

__BEGIN_SYS

SiFive_U_NIC::SiFive_U_NIC(unsigned int unit, DMA_Buffer *dma_buf) {
  db<SiFive_U_NIC>(TRC) << "SiFive_U_NIC(dma=" << dma_buf << ")" << endl;

  _configuration.unit = unit;
  _configuration.timer_frequency = TSC::frequency();
  _configuration.timer_accuracy = TSC::accuracy() / 1000; // PPB -> PPM
  if (!_configuration.timer_accuracy)
    _configuration.timer_accuracy = 1;

  _dma_buf = dma_buf;

  // Distribute the DMA_Buffer allocated by init()
  Log_Addr log = _dma_buf->log_address();
  Phy_Addr phy = _dma_buf->phy_address();

  // Rx_Desc Ring
  _rx_cur = 0;
  _rx_ring = log;
  _rx_ring_phy = phy;
  log += RX_BUFS * (sizeof(Rx_Desc));
  phy += RX_BUFS * (sizeof(Rx_Desc));

  // Tx_Desc Ring
  _tx_cur = 0;
  _tx_ring = log;
  _tx_ring_phy = phy;
  log += TX_BUFS * (sizeof(Tx_Desc));
  phy += TX_BUFS * (sizeof(Tx_Desc));

  // Rx_Buffer Ring
  for (unsigned int i = 0; i < RX_BUFS; i++) {
    _rx_buffer[i] = new (log) Buffer(this, &_rx_ring[i]);
    _rx_ring[i].phy_addr = phy;
    _rx_ring[i].size = Reg16(-sizeof(Frame)); // 2's comp.
    _rx_ring[i].ctrl = Rx_Desc::OWN; // Owned by NIC

    log += (sizeof(Buffer));
    phy += (sizeof(Buffer));
  }

  // Tx_Buffer Ring
  for (unsigned int i = 0; i < TX_BUFS; i++) {
    _tx_buffer[i] = new (log) Buffer(this, &_tx_ring[i]);
    _tx_ring[i].phy_addr = phy;
    _tx_ring[i].size = 0;
    _tx_ring[i].ctrl = 0; // Owned by host

    log += (sizeof(Buffer));
    phy += (sizeof(Buffer));
  }

  // Reset device
  reset();
}

void SiFive_U_NIC::init(unsigned int unit) {
  db<Init, SiFive_U_NIC>(TRC) << "SiFive_U_NIC::init()" << endl;

  // Allocate a DMA Buffer for init block, rx and tx rings
  DMA_Buffer *dma_buf = new (SYSTEM) DMA_Buffer(DMA_BUFFER_SIZE);

  // Initialize the device
  SiFive_U_NIC *dev = new (SYSTEM) SiFive_U_NIC(unit, dma_buf);

  // Register the device
  _devices[unit].device = dev;
  _devices[unit].interrupt = INT_ID;

  // Install interrupt handler
  IC::int_vector(_devices[unit].interrupt, &int_handler);

  // Enable interrupts for device
  IC::enable(_devices[unit].interrupt);
}

__END_SYS
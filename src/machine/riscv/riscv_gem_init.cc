#include <machine/machine.h>
#include <machine/riscv/riscv_gem.h>
#include <system.h>

__BEGIN_SYS

SiFive_U_NIC::SiFive_U_NIC(DMA_Buffer *dma_buf) {
  db<SiFive_U_NIC>(TRC) << "SiFive_U_NIC(dma=" << dma_buf << ")" << endl;

  _dma_buf = dma_buf;

  // Distribute the DMA_Buffer allocated by init()
  Log_Addr log = _dma_buf->log_address();
  Phy_Addr phy = _dma_buf->phy_address();

  // Rx_Desc Ring
  _rx_cur = 0;
  _rx_ring = log;
  _rx_ring_phy = phy;
  log += RX_BUFS * align128(sizeof(Rx_Desc));
  phy += RX_BUFS * align128(sizeof(Rx_Desc));

  // Tx_Desc Ring
  _tx_cur = 0;
  _tx_ring = log;
  _tx_ring_phy = phy;
  log += TX_BUFS * align128(sizeof(Tx_Desc));
  phy += TX_BUFS * align128(sizeof(Tx_Desc));

  // Rx_Buffer Ring
  for (unsigned int i = 0; i < RX_BUFS; i++) {
    _rx_buffer[i] = new (log) Buffer(this, &_rx_ring[i]);
    _rx_ring[i].phy_addr = phy;
    _rx_ring[i].size = Reg16(-sizeof(Frame)); // 2's comp.
    _rx_ring[i].misc = 0;
    _rx_ring[i].status = Desc::OWN; // Owned by NIC

    log += align128(sizeof(Buffer));
    phy += align128(sizeof(Buffer));
  }

  // Tx_Buffer Ring
  for (unsigned int i = 0; i < TX_BUFS; i++) {
    _tx_buffer[i] = new (log) Buffer(this, &_tx_ring[i]);
    _tx_ring[i].phy_addr = phy;
    _tx_ring[i].size = 0;
    _tx_ring[i].misc = 0;
    _tx_ring[i].status = 0; // Owned by host

    log += align128(sizeof(Buffer));
    phy += align128(sizeof(Buffer));
  }

  // Reset device
  reset();
}

__END_SYS
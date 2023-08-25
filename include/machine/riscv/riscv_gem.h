// EPOS RISC-V VisionFive 2 (GEM) Ethernet NIC Mediator Declarations

#ifndef __risc_v_nic_h
#define __risc_v_nic_h

#include <architecture.h>
#include <network/ethernet.h>
#include <utility/convert.h>

__BEGIN_SYS

class GEM
{
protected:
  typedef CPU::Reg8 Reg8;
  typedef CPU::Reg16 Reg16;
  typedef CPU::Reg32 Reg32;
  typedef CPU::Reg64 Reg64;
  typedef CPU::Log_Addr Log_Addr;
  typedef CPU::Phy_Addr Phy_Addr;
  // typedef CPU::IO_Port IO_Port; // Not sure if used for RISC V?
  // typedef CPU::IO_Irq IO_Irq; // Not sure if used for RISC V?
  typedef MMU::DMA_Buffer DMA_Buffer;
  typedef Ethernet::Address MAC_Address;

public:
  // Register offsets
  enum
  {
    NWCTRL = 0x000,      /**< Network Control reg */
    NWCFG = 0x004,       /**< Network Config reg */
    NWSR = 0x008,        /**< Network Status reg */
    USERIO = 0x00C,      /**< User IO reg */
    DMACR = 0x010,       /**< DMA Control reg */
    TXSTATUS = 0x014,    /**< TX Status reg */
    RXQBASE = 0x018,     /**< RX Q Base address reg */
    TXQBASE = 0x01C,     /**< TX Q Base address reg */
    RXSTATUS = 0x020,    /**< RX Status reg */
    ISR = 0x024,         /**< Interrupt Status reg */
    IER = 0x028,         /**< Interrupt Enable reg */
    IDR = 0x02C,         /**< Interrupt Disable reg */
    IMR = 0x030,         /**< Interrupt Mask reg */
    PHYMNTNC = 0x034,    /**< Phy Maintaince reg */
    RXPAUSE = 0x038,     /**< RX Pause Time reg */
    TXPAUSE = 0x03C,     /**< TX Pause Time reg */
    TXPARTIALSF = 0x040, /**< TX Partial Store and Forward reg */
    RXPARTIALSF = 0x044, /**< RX Partial Store and Forward reg */
    JUMBOMAXLEN = 0x048, /**< Jumbo max length reg */
    RXWATERMARK = 0x07C, /**< RX watermark reg */
    HASHL = 0x080,       /**< Hash Low address reg */
    HASHH = 0x084,       /**< Hash High address reg */
    SPADDR1L = 0x088,    /**< Specific1 addr low reg */
    SPADDR1H = 0x08C,    /**< Specific1 addr high reg */
    SPADDR2L = 0x090,    /**< Specific2 addr low reg */
    SPADDR2H = 0x094,    /**< Specific2 addr high reg */
    SPADDR3L = 0x098,    /**< Specific3 addr low reg */
    SPADDR3H = 0x09C,    /**< Specific3 addr high reg */
    SPADDR4L = 0x0A0,    /**< Specific4 addr low reg */
    SPADDR4H = 0x0A4,    /**< Specific4 addr high reg */
    MATCH1 = 0x0A8,      /**< Type ID1 Match reg */
    MATCH2 = 0x0AC,      /**< Type ID2 Match reg */
    MATCH3 = 0x0B0,      /**< Type ID3 Match reg */
    MATCH4 = 0x0B4,      /**< Type ID4 Match reg */
    STRETCH = 0x0BC,     /**< IPG Stretch reg */
    OCTTXL = 0x100,      /**< Octects transmitted Low reg */
    OCTTXH = 0x104,      /**< Octects transmitted High reg */
    TXCNT = 0x108,       /**< Error-free Frames transmitted counter */
    TXBCCNT = 0x10C,     /**< Error-free Broadcast Frames counter */
    TXMCCNT = 0x110,     /**< Error-free Multicast Frame counter */
    TXPAUSECNT = 0x114,  /**< Pause Frames Transmitted Counter */
    TX64CNT = 0x118,     /**< Error-free 64 byte Frames Transmitted counter */
    TX65CNT = 0x11C,     /**< Error-free 65-127 byte Frames Transmitted counter */
    TX128CNT = 0x120,    /**< Error-free 128-255 byte Frames Transmitted counter */
    TX256CNT = 0x124,    /**< Error-free 256-511 byte Frames transmitted counter */
    TX512CNT =
        0x128, /**< Error-free 512-1023 byte Frames transmitted counter */
    TX1024CNT =
        0x12C,              /**< Error-free 1024-1518 byte Frames transmitted counter */
    TX1519CNT = 0x130,      /**< Error-free larger than 1519 byte Frames transmitted
                                counter */
    TXURUNCNT = 0x134,      /**< TX under run error counter */
    SNGLCOLLCNT = 0x138,    /**< Single Collision Frame Counter */
    MULTICOLLCNT = 0x13C,   /**< Multiple Collision Frame Counter */
    EXCESSCOLLCNT = 0x140,  /**< Excessive Collision Frame Counter */
    LATECOLLCNT = 0x144,    /**< Late Collision Frame Counter */
    TXDEFERCNT = 0x148,     /**< Deferred Transmission Frame Counter */
    TXCSENSECNT = 0x14C,    /**< Transmit Carrier Sense Error Counter */
    OCTRXL = 0x150,         /**< Octets Received register Low */
    OCTRXH = 0x154,         /**< Octets Received register High */
    RXCNT = 0x158,          /**< Error-free Frames Received Counter */
    RXBROADCNT = 0x15C,     /**< Error-free Broadcast Frames Received Counter */
    RXMULTICNT = 0x160,     /**< Error-free Multicast Frames Received Counter */
    RXPAUSECNT = 0x164,     /**< Pause Frames Received Counter */
    RX64CNT = 0x168,        /**< Error-free 64 byte Frames Received Counter */
    RX65CNT = 0x16C,        /**< Error-free 65-127 byte Frames Received Counter */
    RX128CNT = 0x170,       /**< Error-free 128-255 byte Frames Received Counter */
    RX256CNT = 0x174,       /**< Error-free 256-512 byte Frames Received Counter */
    RX512CNT = 0x178,       /**< Error-free 512-1023 byte Frames Received Counter */
    RX1024CNT = 0x17C,      /**< Error-free 1024-1518 byte Frames Received Counter */
    RX1519CNT = 0x180,      /**< Error-free 1519-max byte Frames Received Counter */
    RXUNDRCNT = 0x184,      /**< Undersize Frames Received Counter */
    RXOVRCNT = 0x188,       /**< Oversize Frames Received Counter */
    RXJABCNT = 0x18C,       /**< Jabbers Received Counter */
    RXFCSCNT = 0x190,       /**< Frame Check Sequence Error Counter */
    RXLENGTHCNT = 0x194,    /**< Length Field Error Counter */
    RXSYMBCNT = 0x198,      /**< Symbol Error Counter */
    RXALIGNCNT = 0x19C,     /**< Alignment Error Counter */
    RXRESERRCNT = 0x1A0,    /**< Receive Resource Error Counter */
    RXORCNT = 0x1A4,        /**< Receive Overrun Counter */
    RXIPCCNT = 0x1A8,       /**< IP header Checksum Error Counter */
    RXTCPCCNT = 0x1AC,      /**< TCP Checksum Error Counter */
    RXUDPCCNT = 0x1B0,      /**< UDP Checksum Error Counter */
    _1588_SEC = 0x1D0,      /**< 1588 second counter */
    _1588_NANOSEC = 0x1D4,  /**< 1588 nanosecond counter */
    _1588_ADJ = 0x1D8,      /**< 1588 nanosecond adjustment counter */
    _1588_INC = 0x1DC,      /**< 1588 nanosecond increment counter */
    PTP_TXSEC = 0x1E0,      /**< 1588 PTP transmit second counter */
    PTP_TXNANOSEC = 0x1E4,  /**< 1588 PTP transmit nanosecond counter */
    PTP_RXSEC = 0x1E8,      /**< 1588 PTP receive second counter */
    PTP_RXNANOSEC = 0x1EC,  /**< 1588 PTP receive nanosecond counter */
    PTPP_TXSEC = 0x1F0,     /**< 1588 PTP peer transmit second counter */
    PTPP_TXNANOSEC = 0x1F4, /**< 1588 PTP peer transmit nanosecond counter */
    PTPP_RXSEC = 0x1F8,     /**< 1588 PTP peer receive second counter */
    PTPP_RXNANOSEC = 0x1FC, /**< 1588 PTP peer receive nanosecond counter */
    PCS_CONTROL = 0x200,    /** PCS control register */
    PCS_STATUS = 0x204,     /** PCS status register */
    INTQ1_STS = 0x400,      /**< Interrupt Q1 Status reg */
    TXQ1BASE = 0x440,       /**< TX Q1 Base address reg */
    RXQ1BASE = 0x480,       /**< RX Q1 Base address reg */
    MSBBUF_TXQBASE = 0x4C8, /**< MSB Buffer TX Q Base reg */
    MSBBUF_RXQBASE = 0x4D4, /**< MSB Buffer RX Q Base reg */
    INTQ1_IER = 0x600,      /**< Interrupt Q1 Enable reg */
    INTQ1_IDR = 0x620,      /**< Interrupt Q1 Disable reg */
    INTQ1_IMR = 0x640       /**< Interrupt Q1 Mask reg */
  };

  // Design configuration registers
  enum
  {
    DESCONF = 0x280,
    DESCONF2 = 0x284,
    DESCONF3 = 0x288,
    DESCONF4 = 0x28C,
    DESCONF5 = 0x290,
    DESCONF6 = 0x294,
    DESCONF6_64B_MASK = 1U << 23,
    DESCONF7 = 0x298
  };

  // Network Control Register bits
  enum
  {
    TXSTART = 0x200, /**< Transmit start */
    TX_EN = 0x8,     /**< Transmit enable */
    RX_EN = 0x4,     /**< Receive enable */
    LOCALLOOP = 0x2, /**< Local loopback */
  };

  // Network Configuration Register bits
  enum
  {
    STRIP_FCS = 0x20000, /**< Strip FCS field */
    LERR_DISC = 0x10000, /**< Discard RX frames with len err */
    BUFF_OFST_M = 0xC00, /**< Receive buffer offset mask */
    BUFF_OFST_S = 14,    /**< Receive buffer offset shift */
    RCV_1538 = 0x100,    /**< Receive 1538 byte frames */
    UCAST_HASH = 0x80,   /**< Accept unicast hash match */
    MCAST_HASH = 0x40,   /**< Accept multicast hash match */
    BCAST_REJ = 0x20,    /**< Reject broadcast frames */
    PROMISC = 0x10,      /**< Promiscuous mode */
    JUMBO_FRAME = 0x8,   /**< Jumbo frame enable */
  };

  // RO register bits masks
  enum
  {
    NWCTRL_RO_MASK = 0xFFF80000,
    NWSTATUS_RO_MASK = 0xFFFFFFFF,
    DMACFG_RO_MASK = 0x8E00F000,
    TXSTATUS_RO_MASK = 0xFFFFFE08,
    RXQBASE_RO_MASK = 0x00000003,
    TXQBASE_RO_MASK = 0x00000003,
    RXSTATUS_RO_MASK = 0xFFFFFFF0,
    ISR_RO_MASK = 0xFFFFFFFF, // Clear on read
    IMR_RO_MASK = 0xFFFFFFFF,
    MODID_RO_MASK = 0x000000FF
  };

  // Write 1 to clear register bits masks
  enum
  {
    TXSTATUS_CLEAR_MASK = 0x000001F7,
    RXSTATUS_CLEAR_MASK = 0x0000000F
  };

  // WO register bits masks
  enum
  {
    NWCTRL_WO_MASK = 0x00073E60,
    IER_WO_MASK = 0x07FFFFFF,
    IDR_WO_MASK = 0x07FFFFFF
  };

  // Interrupt register bits
  enum
  {
    INT_TXCMPL = 0x80, /**< Transmit complete */
    INT_AMBA_ERR = 0x40,
    INT_TXUSED = 0x08,
    INT_RXUSED = 0x04,
    INT_RXCMPL = 0x02
  };

  // Transmit and Receive Descriptors (in the Ring Buffers)
  struct Desc
  {
    enum
    {
      OWN = 0x8000,
      ERR = 0x4000,
      STP = 0x0200,
      ENP = 0x0100,
      BPE = 0x0080
    };

    Reg32 phy_addr;
    volatile Reg16 size; // 2's complement
    volatile Reg16 status;
    volatile Reg32 misc;
    volatile Reg32 reserved;
  };

  // Receive Descriptor
  struct Rx_Desc : public Desc
  {
    enum
    {
      BUFF = 0x0400,
      CRC = 0x0800,
      OFLO = 0x1000,
      FRAM = 0x2000
    };

    friend Debug &operator<<(Debug &db, const Rx_Desc &d)
    {
      db << "{" << hex << d.phy_addr << dec << "," << 65536 - d.size << ","
         << hex << d.status << "," << d.misc << dec << "}";
      return db;
    }
  };

  // Transmit Descriptor
  struct Tx_Desc : public Desc
  {
    friend Debug &operator<<(Debug &db, const Tx_Desc &d)
    {
      db << "{" << hex << d.phy_addr << dec << "," << 65536 - d.size << ","
         << hex << d.status << "," << d.misc << dec << "}";
      return db;
    }
  };

  static volatile Reg32 &reg(unsigned int o)
  {
    return reinterpret_cast<volatile Reg32 *>(
        Memory_Map::ETH_BASE)[o / sizeof(Reg32)];
  }
};

class SiFive_U_NIC : public NIC<Ethernet>, public GEM
{
private:
  // Transmit and Receive Ring sizes
  static const unsigned int TX_BUFS = 1;
  static const unsigned int RX_BUFS = 1;

  // Size of the DMA Buffer that will host the ring buffers and the init block
  static const unsigned int DMA_BUFFER_SIZE = RX_BUFS * ((sizeof(Rx_Desc) + 15) & ~15U) + TX_BUFS * ((sizeof(Tx_Desc) + 15) & ~15U) +
                                              RX_BUFS * ((sizeof(Buffer) + 15) & ~15U) + TX_BUFS * ((sizeof(Buffer) + 15) & ~15U);

  // Interrupt dispatching binding
  struct Device
  {
    SiFive_U_NIC *device;
    unsigned int interrupt;
  };

public:
  SiFive_U_NIC(DMA_Buffer *dma_buf);

public:
  ~SiFive_U_NIC();
  int send(const Address &dst, const Protocol &prot, const void *data,
           unsigned int size);
  int receive(Address *src, Protocol *prot, void *data, unsigned int size);

  Buffer *alloc(const Address &dst, const Protocol &prot, unsigned int once,
                unsigned int always, unsigned int payload);
  int send(Buffer *buf);
  void free(Buffer *buf);

  virtual void attach(Observer *o, const Protocol &p)
  {
    db<SiFive_U_NIC>(TRC) << "GEM::attach(p=" << p << ")" << endl;
    NIC<Ethernet>::attach(o, p);
    reg(NWCTRL) = reg(NWCTRL) | RX_EN; // enable receive int
  }

  virtual void detach(Observer *o, const Protocol &p)
  {
    NIC<Ethernet>::detach(o, p);
    if (!observers())
    {
      reg(NWCTRL) = reg(NWCTRL) & ~RX_EN; // disable receive int
    };
  }

private:
  void reset();

private:
  Configuration _configuration;
  Statistics _statistics;

  DMA_Buffer *_dma_buf;

  int _rx_cur;
  Rx_Desc *_rx_ring;
  Phy_Addr _rx_ring_phy;

  int _tx_cur;
  Tx_Desc *_tx_ring;
  Phy_Addr _tx_ring_phy;

  Buffer *_rx_buffer[RX_BUFS];
  Buffer *_tx_buffer[TX_BUFS];
};

__END_SYS

#endif
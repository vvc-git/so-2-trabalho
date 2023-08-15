// EPOS RISC-V VisionFive 2 (GEM) Ethernet NIC Mediator Declarations

#ifndef __risc_v_nic_h
#define __risc_v_nic_h

#include <architecture.h>
#include <network/ethernet.h>
#include <utility/convert.h>

class GEM {
protected:
  typedef CPU::Reg8 Reg8;
  typedef CPU::Reg16 Reg16;
  typedef CPU::Reg32 Reg32;
  typedef CPU::Log_Addr Log_Addr;
  typedef CPU::Phy_Addr Phy_Addr;
  typedef CPU::IO_Port IO_Port;
  typedef CPU::IO_Irq IO_Irq;
  typedef MMU::DMA_Buffer DMA_Buffer;
  typedef Ethernet::Address MAC_Address;

public:
  // Constant Definitions
  enum {
    MAX_MAC_ADDR = 4,    // Maximum number of MAC addresses supported
    MAX_TYPE_ID = 4,     // Maximum number of Type IDs supported
    BD_ALIGNMENT = 4,    // Minimum Buffer Descriptor alignment on the local bus
    RX_BUF_ALIGNMENT = 4 /* Minimum buffer alignment when using
                                           options that impose alignment
                                           restrictions on the buffer data on
                                           the local bus */
  };

  // Direction identifiers
  enum {
    SEND = 1,
    RECV = 2,
  };

  // MDC clock division
  enum MdcDiv {
    MDC_DIV_8 = 0,
    MDC_DIV_16 = 1,
    MDC_DIV_32 = 2,
    MDC_DIV_48 = 3,
    MDC_DIV_64 = 4,
    MDC_DIV_96 = 5,
    MDC_DIV_128 = 6,
    MDC_DIV_224 = 7
  };

  // TODO: which category?
  enum {
    RX_BUF_SIZE = 1536, /**< Specify the receive buffer size in bytes, 64, 128,
                            ... 10240 */
    RX_BUF_SIZE_JUMBO = 10240, /**< Receive buffer size for jumbo frames */
    RX_BUF_UNIT =
        64U, /**< Number of receive buffer bytes as a unit, this is HW setup */
    MAX_RXBD = 128,    /**< Size of RX buffer descriptor queues */
    MAX_TXBD = 128,    /**< Size of TX buffer descriptor queues */
    MAX_HASH_BITS = 64 /**< Maximum value for hash bits. 2**6 */
  };

  // Register offsets
  enum {
    NWCTRL = 0x000,      /**< Network Control reg */
    NWCFG = 0x004,       /**< Network Config reg */
    NWSR = 0x008,        /**< Network Status reg */
    DMACR = 0x010,       /**< DMA Control reg */
    TXSR = 0x014,        /**< TX Status reg */
    RXQBASE = 0x018,     /**< RX Q Base address reg */
    TXQBASE = 0x01C,     /**< TX Q Base address reg */
    RXSR = 0x020,        /**< RX Status reg */
    ISR = 0x024,         /**< Interrupt Status reg */
    IER = 0x028,         /**< Interrupt Enable reg */
    IDR = 0x02C,         /**< Interrupt Disable reg */
    IMR = 0x030,         /**< Interrupt Mask reg */
    PHYMNTNC = 0x034,    /**< Phy Maintaince reg */
    RXPAUSE = 0x038,     /**< RX Pause Time reg */
    TXPAUSE = 0x03C,     /**< TX Pause Time reg */
    JUMBOMAXLEN = 0x048, /**< Jumbo max length reg */
    RXWATERMARK = 0x07C, /**< RX watermark reg */
    HASHL = 0x080,       /**< Hash Low address reg */
    HASHH = 0x084,       /**< Hash High address reg */
    LADDR1L = 0x088,     /**< Specific1 addr low reg */
    LADDR1H = 0x08C,     /**< Specific1 addr high reg */
    LADDR2L = 0x090,     /**< Specific2 addr low reg */
    LADDR2H = 0x094,     /**< Specific2 addr high reg */
    LADDR3L = 0x098,     /**< Specific3 addr low reg */
    LADDR3H = 0x09C,     /**< Specific3 addr high reg */
    LADDR4L = 0x0A0,     /**< Specific4 addr low reg */
    LADDR4H = 0x0A4,     /**< Specific4 addr high reg */
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
    TX65CNT = 0x11C,  /**< Error-free 65-127 byte Frames Transmitted counter */
    TX128CNT = 0x120, /**< Error-free 128-255 byte Frames Transmitted counter */
    TX256CNT = 0x124, /**< Error-free 256-511 byte Frames transmitted counter */
    TX512CNT =
        0x128, /**< Error-free 512-1023 byte Frames transmitted counter */
    TX1024CNT =
        0x12C, /**< Error-free 1024-1518 byte Frames transmitted counter */
    TX1519CNT = 0x130, /**< Error-free larger than 1519 byte Frames transmitted
                           counter */
    TXURUNCNT = 0x134, /**< TX under run error counter */
    SNGLCOLLCNT = 0x138,   /**< Single Collision Frame Counter */
    MULTICOLLCNT = 0x13C,  /**< Multiple Collision Frame Counter */
    EXCESSCOLLCNT = 0x140, /**< Excessive Collision Frame Counter */
    LATECOLLCNT = 0x144,   /**< Late Collision Frame Counter */
    TXDEFERCNT = 0x148,    /**< Deferred Transmission Frame Counter */
    TXCSENSECNT = 0x14C,   /**< Transmit Carrier Sense Error Counter */
    OCTRXL = 0x150,        /**< Octets Received register Low */
    OCTRXH = 0x154,        /**< Octets Received register High */
    RXCNT = 0x158,         /**< Error-free Frames Received Counter */
    RXBROADCNT = 0x15C,    /**< Error-free Broadcast Frames Received Counter */
    RXMULTICNT = 0x160,    /**< Error-free Multicast Frames Received Counter */
    RXPAUSECNT = 0x164,    /**< Pause Frames Received Counter */
    RX64CNT = 0x168,       /**< Error-free 64 byte Frames Received Counter */
    RX65CNT = 0x16C,   /**< Error-free 65-127 byte Frames Received Counter */
    RX128CNT = 0x170,  /**< Error-free 128-255 byte Frames Received Counter */
    RX256CNT = 0x174,  /**< Error-free 256-512 byte Frames Received Counter */
    RX512CNT = 0x178,  /**< Error-free 512-1023 byte Frames Received Counter */
    RX1024CNT = 0x17C, /**< Error-free 1024-1518 byte Frames Received Counter */
    RX1519CNT = 0x180, /**< Error-free 1519-max byte Frames Received Counter */
    RXUNDRCNT = 0x184, /**< Undersize Frames Received Counter */
    RXOVRCNT = 0x188,  /**< Oversize Frames Received Counter */
    RXJABCNT = 0x18C,  /**< Jabbers Received Counter */
    RXFCSCNT = 0x190,  /**< Frame Check Sequence Error Counter */
    RXLENGTHCNT = 0x194,    /**< Length Field Error Counter */
    RXSYMBCNT = 0x198,      /**< Symbol Error Counter */
    RXALIGNCNT = 0x19C,     /**< Alignment Error Counter */
    RXRESERRCNT = 0x1A0,    /**< Receive Resource Error Counter */
    RXORCNT = 0x1A4,        /**< Receive Overrun Counter */
    RXIPCCNT = 0x1A8,       /**< IP header Checksum Error Counter */
    RXTCPCCNT = 0x1AC,      /**< TCP Checksum Error Counter */
    RXUDPCCNT = 0x1B0,      /**< UDP Checksum Error Counter */
    LAST = 0x1B4,           /**< Last statistic counter offset, for clearing */
    1588_SEC = 0x1D0,       /**< 1588 second counter */
    1588_NANOSEC = 0x1D4,   /**< 1588 nanosecond counter */
    1588_ADJ = 0x1D8,       /**< 1588 nanosecond adjustment counter */
    1588_INC = 0x1DC,       /**< 1588 nanosecond increment counter */
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

  // Network Control Register bits
  enum {
    FLUSH_DPRAM = 0x40000, /**< Flush a packet from Rx SRAM */
    ZEROPAUSETX = 0x00800, /**< Transmit zero quantum pause frame */
    PAUSETX = 0x00800,     /**< Transmit pause frame */
    HALTTX = 0x00400,      /**< Halt transmission after current frame */
    STARTTX = 0x00200,     /**< Start tx (tx_go) */
    STATWEN = 0x00080,     /**< Enable writing to stat counters */
    STATINC = 0x00040,     /**< Increment statistic registers */
    STATCLR = 0x00020,     /**< Clear statistic registers */
    MDEN = 0x00010,        /**< Enable MDIO port */
    TXEN = 0x00008,        /**< Enable transmit */
    RXEN = 0x00004,        /**< Enable receive */
    LOOPEN = 0x00002       /**< local loopback */
  };

  // Network Configuration Register bits
  enum {
    BADPREAMBEN = 0x20000000, /**< disable rejection of non-standard preamble */
    IPDSTRETCH = 0x10000000,  /**< enable transmit IPG */
    SGMIIEN = 0x08000000,     /**< SGMII Enable */
    FCSIGNORE = 0x04000000,   /**< disable rejection of FCS error */
    HDRXEN = 0x02000000,      /**< RX half duplex */
    RXCHKSUMEN = 0x01000000,  /**< enable RX checksum offload */
    PAUSECOPYDI = 0x00800000, /**< Do not copy pause Frames to memory */
    DWIDTH_64 = 0x00200000,   /**< 64 bit Data bus width */
    MDC_SHIFT = 18,           /**< shift bits for MDC */
    MDCCLKDIV = 0x001C0000,   /**< MDC Mask PCLK divisor */
    FCSREM = 0x00020000,      /**< Discard FCS from received frames */
    LENERRDSCRD = 0x00010000, /**< RX length error discard */
    RXOFFS = 0x0000C000,      /**< RX buffer offset */
    PAUSEEN = 0x00002000,     /**< Enable pause RX */
    RETRYTESTEN = 0x00001000, /**< Retry test */
    XTADDMACHEN = 0x00000200, /**< External address match enable */
    PCSSEL = 0x00000800,      /**< PCS Select */
    _1000 = 0x00000400,       /**< 1000 Mbps */
    _1536RXEN = 0x00000100,   /**< Enable 1536 byte frames reception */
    UCASTHASHEN = 0x00000080, /**< Receive unicast hash frames */
    MCASTHASHEN = 0x00000040, /**< Receive multicast hash frames */
    BCASTDI = 0x00000020,     /**< Do not receive broadcast frames */
    COPYALLEN = 0x00000010,   /**< Copy all frames */
    JUMBO = 0x00000008,       /**< Jumbo frames */
    NVLANDISC = 0x00000004,   /**< Receive only VLAN frames */
    FDEN = 0x00000002,        /**< full duplex */
    _100 = 0x00000001,        /**< 100 Mbps */
    RESET = 0x00080000        /**< reset value */
  };
};

class VisionFive2_NIC : public NIC<Ethernet>, private GEM {};

#endif
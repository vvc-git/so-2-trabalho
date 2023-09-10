#ifndef __cadence_h
#define __cadence_h

#include <architecture.h>
#include <utility/convert.h>
#include <network/ethernet.h>
#include <utility/string.h>
#include <machine/riscv/if_cgem_hw.h>

__BEGIN_SYS

#define GEM_NWCTRL        (0x00000000 / 4) /* Network Control reg */
#define GEM_NWCFG         (0x00000004 / 4) /* Network Config reg */
#define GEM_NWSTATUS      (0x00000008 / 4) /* Network Status reg */
#define GEM_USERIO        (0x0000000C / 4) /* User IO reg */
#define GEM_DMACFG        (0x00000010 / 4) /* DMA Control reg */
#define GEM_TXSTATUS      (0x00000014 / 4) /* TX Status reg */
#define GEM_RXQBASE       (0x00000018 / 4) /* RX Q Base address reg */
#define GEM_TXQBASE       (0x0000001C / 4) /* TX Q Base address reg */
#define GEM_RXSTATUS      (0x00000020 / 4) /* RX Status reg */
#define GEM_ISR           (0x00000024 / 4) /* Interrupt Status reg */
#define GEM_IER           (0x00000028 / 4) /* Interrupt Enable reg */
#define GEM_IDR           (0x0000002C / 4) /* Interrupt Disable reg */
#define GEM_IMR           (0x00000030 / 4) /* Interrupt Mask reg */
#define GEM_PHYMNTNC      (0x00000034 / 4) /* Phy Maintenance reg */
#define GEM_RXPAUSE       (0x00000038 / 4) /* RX Pause Time reg */
#define GEM_TXPAUSE       (0x0000003C / 4) /* TX Pause Time reg */
#define GEM_TXPARTIALSF   (0x00000040 / 4) /* TX Partial Store and Forward */
#define GEM_RXPARTIALSF   (0x00000044 / 4) /* RX Partial Store and Forward */
#define GEM_JUMBO_MAX_LEN (0x00000048 / 4) /* Max Jumbo Frame Size */
#define GEM_HASHLO        (0x00000080 / 4) /* Hash Low address reg */
#define GEM_HASHHI        (0x00000084 / 4) /* Hash High address reg */
#define GEM_SPADDR1LO     (0x00000088 / 4) /* Specific addr 1 low reg */
#define GEM_SPADDR1HI     (0x0000008C / 4) /* Specific addr 1 high reg */
#define GEM_SPADDR2LO     (0x00000090 / 4) /* Specific addr 2 low reg */
#define GEM_SPADDR2HI     (0x00000094 / 4) /* Specific addr 2 high reg */
#define GEM_SPADDR3LO     (0x00000098 / 4) /* Specific addr 3 low reg */
#define GEM_SPADDR3HI     (0x0000009C / 4) /* Specific addr 3 high reg */
#define GEM_SPADDR4LO     (0x000000A0 / 4) /* Specific addr 4 low reg */
#define GEM_SPADDR4HI     (0x000000A4 / 4) /* Specific addr 4 high reg */
#define GEM_TIDMATCH1     (0x000000A8 / 4) /* Type ID1 Match reg */
#define GEM_TIDMATCH2     (0x000000AC / 4) /* Type ID2 Match reg */
#define GEM_TIDMATCH3     (0x000000B0 / 4) /* Type ID3 Match reg */
#define GEM_TIDMATCH4     (0x000000B4 / 4) /* Type ID4 Match reg */
#define GEM_WOLAN         (0x000000B8 / 4) /* Wake on LAN reg */
#define GEM_IPGSTRETCH    (0x000000BC / 4) /* IPG Stretch reg */
#define GEM_SVLAN         (0x000000C0 / 4) /* Stacked VLAN reg */
#define GEM_MODID         (0x000000FC / 4) /* Module ID reg */
#define GEM_OCTTXLO       (0x00000100 / 4) /* Octects transmitted Low reg */
#define GEM_OCTTXHI       (0x00000104 / 4) /* Octects transmitted High reg */
#define GEM_TXCNT         (0x00000108 / 4) /* Error-free Frames transmitted */
#define GEM_TXBCNT        (0x0000010C / 4) /* Error-free Broadcast Frames */
#define GEM_TXMCNT        (0x00000110 / 4) /* Error-free Multicast Frame */
#define GEM_TXPAUSECNT    (0x00000114 / 4) /* Pause Frames Transmitted */
#define GEM_TX64CNT       (0x00000118 / 4) /* Error-free 64 TX */
#define GEM_TX65CNT       (0x0000011C / 4) /* Error-free 65-127 TX */
#define GEM_TX128CNT      (0x00000120 / 4) /* Error-free 128-255 TX */
#define GEM_TX256CNT      (0x00000124 / 4) /* Error-free 256-511 */
#define GEM_TX512CNT      (0x00000128 / 4) /* Error-free 512-1023 TX */
#define GEM_TX1024CNT     (0x0000012C / 4) /* Error-free 1024-1518 TX */
#define GEM_TX1519CNT     (0x00000130 / 4) /* Error-free larger than 1519 TX */
#define GEM_TXURUNCNT     (0x00000134 / 4) /* TX under run error counter */
#define GEM_SINGLECOLLCNT (0x00000138 / 4) /* Single Collision Frames */
#define GEM_MULTCOLLCNT   (0x0000013C / 4) /* Multiple Collision Frames */
#define GEM_EXCESSCOLLCNT (0x00000140 / 4) /* Excessive Collision Frames */
#define GEM_LATECOLLCNT   (0x00000144 / 4) /* Late Collision Frames */
#define GEM_DEFERTXCNT    (0x00000148 / 4) /* Deferred Transmission Frames */
#define GEM_CSENSECNT     (0x0000014C / 4) /* Carrier Sense Error Counter */
#define GEM_OCTRXLO       (0x00000150 / 4) /* Octects Received register Low */
#define GEM_OCTRXHI       (0x00000154 / 4) /* Octects Received register High */
#define GEM_RXCNT         (0x00000158 / 4) /* Error-free Frames Received */
#define GEM_RXBROADCNT    (0x0000015C / 4) /* Error-free Broadcast Frames RX */
#define GEM_RXMULTICNT    (0x00000160 / 4) /* Error-free Multicast Frames RX */
#define GEM_RXPAUSECNT    (0x00000164 / 4) /* Pause Frames Received Counter */
#define GEM_RX64CNT       (0x00000168 / 4) /* Error-free 64 byte Frames RX */
#define GEM_RX65CNT       (0x0000016C / 4) /* Error-free 65-127B Frames RX */
#define GEM_RX128CNT      (0x00000170 / 4) /* Error-free 128-255B Frames RX */
#define GEM_RX256CNT      (0x00000174 / 4) /* Error-free 256-512B Frames RX */
#define GEM_RX512CNT      (0x00000178 / 4) /* Error-free 512-1023B Frames RX */
#define GEM_RX1024CNT     (0x0000017C / 4) /* Error-free 1024-1518B Frames RX */
#define GEM_RX1519CNT     (0x00000180 / 4) /* Error-free 1519-max Frames RX */
#define GEM_RXUNDERCNT    (0x00000184 / 4) /* Undersize Frames Received */
#define GEM_RXOVERCNT     (0x00000188 / 4) /* Oversize Frames Received */
#define GEM_RXJABCNT      (0x0000018C / 4) /* Jabbers Received Counter */
#define GEM_RXFCSCNT      (0x00000190 / 4) /* Frame Check seq. Error Counter */
#define GEM_RXLENERRCNT   (0x00000194 / 4) /* Length Field Error Counter */
#define GEM_RXSYMERRCNT   (0x00000198 / 4) /* Symbol Error Counter */
#define GEM_RXALIGNERRCNT (0x0000019C / 4) /* Alignment Error Counter */
#define GEM_RXRSCERRCNT   (0x000001A0 / 4) /* Receive Resource Error Counter */
#define GEM_RXORUNCNT     (0x000001A4 / 4) /* Receive Overrun Counter */
#define GEM_RXIPCSERRCNT  (0x000001A8 / 4) /* IP header Checksum Err Counter */
#define GEM_RXTCPCCNT     (0x000001AC / 4) /* TCP Checksum Error Counter */
#define GEM_RXUDPCCNT     (0x000001B0 / 4) /* UDP Checksum Error Counter */

class Cadence
{
protected:
    typedef CPU::Reg8 Reg8;
    typedef CPU::Reg16 Reg16;
    typedef CPU::Reg32 Reg32;
    typedef CPU::Log_Addr Log_Addr;
    typedef CPU::Phy_Addr Phy_Addr;
    // typedef CPU::IO_Port IO_Port;
    // typedef CPU::IO_Irq IO_Irq;
    // USAR NOSSO BUFFER
    typedef MMU::DMA_Buffer DMA_Buffer;
    // Ver como usar
    typedef Ethernet::Address MAC_Address;

public:
    enum : unsigned long {
        ETH_BASE        = 0x10090000   // SiFive-U Ethernet
    };

    Cadence();
    void set_value(Reg32 * offset, unsigned int value);  
};

Cadence::Cadence() {
    // ! Vamos ter que implementar a PLIC?
    // Clear network control register
    Cadence::set_value(reinterpret_cast<Reg32 *>(GEM_NWCTRL), 0x0);

    // Clear statics registers
    Cadence::set_value(reinterpret_cast<Reg32 *>(GEM_NWCTRL), CGEM_NET_CTRL_CLR_STAT_REGS);

    // Clear status registers
    Cadence::set_value(reinterpret_cast<Reg32 *>(GEM_NWSTATUS), 0x0F);
    Cadence::set_value(reinterpret_cast<Reg32 *>(GEM_TXSTATUS), 0xFF);

    // Disable all interrupts
    Cadence::set_value(reinterpret_cast<Reg32 *>(GEM_IDR), 0x7FFFEFF);

    // Clear the buffer queues
    // ! Checar o significado de { , 1}_ptr em gem.receive_q{ , 1}_ptr
    Cadence::set_value(reinterpret_cast<Reg32 *>(GEM_RXQBASE), 0x0);
    Cadence::set_value(reinterpret_cast<Reg32 *>(GEM_TXQBASE), 0x0);

    // Enable full-duplex | Enable Gigabit mode | Enable reception multicast frames | Enable promiscuous mode | Enable checksum offload | Enable pause frames | Set the MDC clock divisor | *** Set MAC address *** |
    // ! Enable pause frames: CGEM_NET_CFG_PAUSE_EN ou CGEM_NET_CFG_DIS_CP_PAUSE_FRAME?
    // ! Set the MDC clock divisor (como saber qual é o clock apropriado?)
    // ! Como setar o MAC address?
    Cadence::set_value(reinterpret_cast<Reg32 *>(GEM_NWCFG), (CGEM_NET_CFG_FULL_DUPLEX | CGEM_NET_CFG_MULTI_HASH_EN | CGEM_NET_CFG_COPY_ALL | CGEM_NET_CFG_RX_CHKSUM_OFFLD_EN | CGEM_NET_CFG_PAUSE_EN | CGEM_NET_CFG_MDC_CLK_DIV_64 | ));

    // Set the receive buffer size to 1.600 bytes
    // ! Checar porque ele fala para colocar 8h'019
    // ! Set the receive packet buffer memory size 
    // ! Set the transmitter packet buffer memory size 
    // ! Enable tcp/ip checksum geraration offload on the transmitter (não encontramos)
    // Configure for a little endian system. 
    // ! Configure AXI fixed burst length. Write 5'h10 to the gem.dma_config[amba_burst_length]: seria CGEM_DMA_CFG_AHB_FIXED_BURST_LEN_X (X é 1, 4, 8 ou 16)?
    Cadence::set_value(reinterpret_cast<Reg32 *>(GEM_DMACFG), (CGEM_DMA_CFG_RX_BUF_SIZE(0x19)));

    // !Enable the MDIO: gem.network_control[men_port_en] seria o CGEM_NET_CTRL_MGMT_PORT_EN? O que seria MGMT?
    // Enable the transmitter
    // Enable the receiver
    Cadence::set_value(reinterpret_cast<Reg32 *>(GEM_NWCTRL), (CGEM_NET_CTRL_MGMT_PORT_EN | CGEM_NET_CTRL_TX_EN | CGEM_NET_CTRL_RX_EN));

    // Outros...
}

void Cadence::set_value(Reg32 * offset, unsigned int value) {
    *(ETH_BASE + offset) = reinterpret_cast<Reg32>(value);  
}


__END_SYS

#endif

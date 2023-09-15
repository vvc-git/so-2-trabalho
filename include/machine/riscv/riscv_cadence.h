#ifndef __cadence_h
#define __cadence_h

#include <architecture.h>
#include <utility/convert.h>
#include <network/ethernet.h>
#include <utility/string.h>
#include <machine/riscv/if_cgem_hw.h>
#include <machine/riscv/sifive_u/sifive_u_memory_map.h>

__BEGIN_SYS

OStream cout;

#define GEM_NWCTRL        (0x00000000) /* Network Control reg */
#define GEM_NWCFG         (0x00000004) /* Network Config reg */
#define GEM_NWSTATUS      (0x00000008) /* Network Status reg */
#define GEM_USERIO        (0x0000000C) /* User IO reg */
#define GEM_DMACFG        (0x00000010) /* DMA Control reg */
#define GEM_TXSTATUS      (0x00000014) /* TX Status reg */
#define GEM_RXQBASE       (0x00000018) /* RX Q Base address reg */
#define GEM_TXQBASE       (0x0000001C) /* TX Q Base address reg */
#define GEM_RXSTATUS      (0x00000020) /* RX Status reg */
#define GEM_ISR           (0x00000024) /* Interrupt Status reg */
#define GEM_IER           (0x00000028) /* Interrupt Enable reg */
#define GEM_IDR           (0x0000002C) /* Interrupt Disable reg */
#define GEM_IMR           (0x00000030) /* Interrupt Mask reg */
#define GEM_PHYMNTNC      (0x00000034) /* Phy Maintenance reg */
#define GEM_RXPAUSE       (0x00000038) /* RX Pause Time reg */
#define GEM_TXPAUSE       (0x0000003C) /* TX Pause Time reg */
#define GEM_TXPARTIALSF   (0x00000040) /* TX Partial Store and Forward */
#define GEM_RXPARTIALSF   (0x00000044) /* RX Partial Store and Forward */
#define GEM_JUMBO_MAX_LEN (0x00000048) /* Max Jumbo Frame Size */
#define GEM_HASHLO        (0x00000080) /* Hash Low address reg */
#define GEM_HASHHI        (0x00000084) /* Hash High address reg */
#define GEM_SPADDR1LO     (0x00000088) /* Specific addr 1 low reg */
#define GEM_SPADDR1HI     (0x0000008C) /* Specific addr 1 high reg */
#define GEM_SPADDR2LO     (0x00000090) /* Specific addr 2 low reg */
#define GEM_SPADDR2HI     (0x00000094) /* Specific addr 2 high reg */
#define GEM_SPADDR3LO     (0x00000098) /* Specific addr 3 low reg */
#define GEM_SPADDR3HI     (0x0000009C) /* Specific addr 3 high reg */
#define GEM_SPADDR4LO     (0x000000A0) /* Specific addr 4 low reg */
#define GEM_SPADDR4HI     (0x000000A4) /* Specific addr 4 high reg */
#define GEM_TIDMATCH1     (0x000000A8) /* Type ID1 Match reg */
#define GEM_TIDMATCH2     (0x000000AC) /* Type ID2 Match reg */
#define GEM_TIDMATCH3     (0x000000B0) /* Type ID3 Match reg */
#define GEM_TIDMATCH4     (0x000000B4) /* Type ID4 Match reg */
#define GEM_WOLAN         (0x000000B8) /* Wake on LAN reg */
#define GEM_IPGSTRETCH    (0x000000BC) /* IPG Stretch reg */
#define GEM_SVLAN         (0x000000C0) /* Stacked VLAN reg */
#define GEM_MODID         (0x000000FC) /* Module ID reg */
#define GEM_OCTTXLO       (0x00000100) /* Octects transmitted Low reg */
#define GEM_OCTTXHI       (0x00000104) /* Octects transmitted High reg */
#define GEM_TXCNT         (0x00000108) /* Error-free Frames transmitted */
#define GEM_TXBCNT        (0x0000010C) /* Error-free Broadcast Frames */
#define GEM_TXMCNT        (0x00000110) /* Error-free Multicast Frame */
#define GEM_TXPAUSECNT    (0x00000114) /* Pause Frames Transmitted */
#define GEM_TX64CNT       (0x00000118) /* Error-free 64 TX */
#define GEM_TX65CNT       (0x0000011C) /* Error-free 65-127 TX */
#define GEM_TX128CNT      (0x00000120) /* Error-free 128-255 TX */
#define GEM_TX256CNT      (0x00000124) /* Error-free 256-511 */
#define GEM_TX512CNT      (0x00000128) /* Error-free 512-1023 TX */
#define GEM_TX1024CNT     (0x0000012C) /* Error-free 1024-1518 TX */
#define GEM_TX1519CNT     (0x00000130) /* Error-free larger than 1519 TX */
#define GEM_TXURUNCNT     (0x00000134) /* TX under run error counter */
#define GEM_SINGLECOLLCNT (0x00000138) /* Single Collision Frames */
#define GEM_MULTCOLLCNT   (0x0000013C) /* Multiple Collision Frames */
#define GEM_EXCESSCOLLCNT (0x00000140) /* Excessive Collision Frames */
#define GEM_LATECOLLCNT   (0x00000144) /* Late Collision Frames */
#define GEM_DEFERTXCNT    (0x00000148) /* Deferred Transmission Frames */
#define GEM_CSENSECNT     (0x0000014C) /* Carrier Sense Error Counter */
#define GEM_OCTRXLO       (0x00000150) /* Octects Received register Low */
#define GEM_OCTRXHI       (0x00000154) /* Octects Received register High */
#define GEM_RXCNT         (0x00000158) /* Error-free Frames Received */
#define GEM_RXBROADCNT    (0x0000015C) /* Error-free Broadcast Frames RX */
#define GEM_RXMULTICNT    (0x00000160) /* Error-free Multicast Frames RX */
#define GEM_RXPAUSECNT    (0x00000164) /* Pause Frames Received Counter */
#define GEM_RX64CNT       (0x00000168) /* Error-free 64 byte Frames RX */
#define GEM_RX65CNT       (0x0000016C) /* Error-free 65-127B Frames RX */
#define GEM_RX128CNT      (0x00000170) /* Error-free 128-255B Frames RX */
#define GEM_RX256CNT      (0x00000174) /* Error-free 256-512B Frames RX */
#define GEM_RX512CNT      (0x00000178) /* Error-free 512-1023B Frames RX */
#define GEM_RX1024CNT     (0x0000017C) /* Error-free 1024-1518B Frames RX */
#define GEM_RX1519CNT     (0x00000180) /* Error-free 1519-max Frames RX */
#define GEM_RXUNDERCNT    (0x00000184) /* Undersize Frames Received */
#define GEM_RXOVERCNT     (0x00000188) /* Oversize Frames Received */
#define GEM_RXJABCNT      (0x0000018C) /* Jabbers Received Counter */
#define GEM_RXFCSCNT      (0x00000190) /* Frame Check seq. Error Counter */
#define GEM_RXLENERRCNT   (0x00000194) /* Length Field Error Counter */
#define GEM_RXSYMERRCNT   (0x00000198) /* Symbol Error Counter */
#define GEM_RXALIGNERRCNT (0x0000019C) /* Alignment Error Counter */
#define GEM_RXRSCERRCNT   (0x000001A0) /* Receive Resource Error Counter */
#define GEM_RXORUNCNT     (0x000001A4) /* Receive Overrun Counter */
#define GEM_RXIPCSERRCNT  (0x000001A8) /* IP header Checksum Err Counter */
#define GEM_RXTCPCCNT     (0x000001AC) /* TCP Checksum Error Counter */
#define GEM_RXUDPCCNT     (0x000001B0) /* UDP Checksum Error Counter */

class Cadence
{
protected:
    typedef CPU::Reg8 Reg8;
    typedef CPU::Reg16 Reg16;
    typedef CPU::Reg32 Reg32;
    typedef CPU::Log_Addr Log_Addr;
    typedef CPU::Phy_Addr Phy_Addr;
    // USAR NOSSO BUFFER
    typedef MMU::DMA_Buffer DMA_Buffer;
    // Ver como usar
    typedef Ethernet::Address MAC_Address;

    // typedef CPU::IO_Port IO_Port;
    // typedef CPU::IO_Irq IO_Irq;
    
public:
    // Register offsets
    enum {
        NETWORK_CONTROL = 0x0000000000,
        NETWORK_CONFIG = 0x0000000004,
        NETWORK_STATUS =  0x0000000008,
        TRANSMIT_STATUS = 0x0000000014,
        RECEIVE_Q_PTR   = 0x0000000018,
        TRANSMIT_Q_PTR   = 0x000000001C,
        RECEIVE_STATUS =  0x0000000020,
        INT_DISABLE    =  0x000000002C, 
        SPEC_ADD1_BOTTOM = 0x0000000088

        
    };

    // Network Control Register bits
    enum {
        CLEAR_ALL_STATS_REGS = 1 << 5,
    };

    // Network Config Register bits
    enum {
        FULL_DUPLEX = 1 << 2,
        GIGABIT_MODE_ENABLE = 1 << 10,
        NO_BROADCAST = 1 << 5,
        MULTICAST_HASH_ENABLE = 1 << 6,
        COPY_ALL_FRAMES = 1 << 4,
        RECEIVE_CHECKSUM_OFFLOAD_ENABLE = 1 << 24,
        PAUSE_ENABLE = 1 << 13,
        MDC_CLOCK_DIVISION = 000 << 18,


    };

    // construtor
    Cadence();

    // função que seta os valores dos registradores mapeados em memória
    void set_reg(unsigned long int pointer, unsigned int value);

    // função que retorna or lógico dos valores passados por parâmetros
    void set_bits(unsigned long int pointer, unsigned int value);
};

Cadence::Cadence() {

    // 1. Clear the network control register
    //  Write 0x0 to the gem.network_control register.
    Cadence::set_reg(NETWORK_CONTROL, 0x0);

    // 2. Clear the statistics registers
    // Write a 1 to the gem.network_control [clear_all_stats_regs].
    Cadence::set_bits(NETWORK_CONTROL, CLEAR_ALL_STATS_REGS);

    // 3. Clear the status registers

    // ?? Não conseguimos fazer essa:
    // Write a 1 to the status registers. 
    
    // gem.receive_status = 0x0F
    Cadence::set_reg(RECEIVE_STATUS, 0x0F);

    // gem.transmit_status = 0xFF.
    Cadence::set_reg(TRANSMIT_STATUS, 0xFF);

    // 4.Disable all interrupts.
    // Write 0x7FF_FEFF to the gem. int_disable register.
    Cadence::set_reg(INT_DISABLE, 0x7FFFEFF);

    // 5. Clear the buffer queues.
    // Write 0x0 to the gem.receive_q{ , 1}_ptr 
    // ?? Duvida se esta mesmo sentando, pois testamos com os WR e mesmo tava dando errado
    Cadence::set_reg(RECEIVE_Q_PTR, 0X0);

    // Write 0x0 to the gem.transmit_q{ , 1}_ptr 
    // ?? Duvida se esta mesmo sentando, pois testamos com os WR e mesmo tava dando errado
    Cadence::set_reg(RECEIVE_Q_PTR, 0x0);



    // Configure the controller


    // 1. Enable full duplex. 
    // a. Write a 1 to the gem.network_config[full_duplex] bit.
    Cadence::set_bits(NETWORK_CONFIG, FULL_DUPLEX);

    // b. Enable gigabit mode. 
    // Write a 1 to the gem.network_config[gigabit_mode_enable] bit.
    Cadence::set_bits(NETWORK_CONFIG, GIGABIT_MODE_ENABLE);

    // c. Enable reception of broadcast or multicast frames. 
    // Write a 0 to the gem.network_config[no_broadcast] register to enable broadcast frames.
    Cadence::set_bits(NETWORK_CONFIG, NO_BROADCAST);

    // write a 1 to the gem.network_config[multicast_hash_en] bit to enable multicast frames.
    Cadence::set_bits(NETWORK_CONFIG, MULTICAST_HASH_ENABLE);

    // d. Enable promiscuous mode.
    // Write a 1 to the gem.network_config[copy_all_frames] bit.
    Cadence::set_bits(NETWORK_CONFIG, COPY_ALL_FRAMES);

    // e. Enable TCP/IP checksum offload feature on receive. 
    // Write a 1 to the gem.network_config[receive_checksum_offload_enable] bit.
    Cadence::set_bits(NETWORK_CONFIG, RECEIVE_CHECKSUM_OFFLOAD_ENABLE);

    // f.Enable pause frames. Write a 1 to gem.network_config[pause_enable] bit.
    Cadence::set_bits(NETWORK_CONFIG, PAUSE_ENABLE);

    // g. Set the MDC clock divisor. 
    // Write the appropriate MDC clock divisor to the gem.network_config[mdc_clock_division] bit.
    // ?? QUAL CLOCK ESCOLHER?
    Cadence::set_bits(NETWORK_CONFIG, MDC_CLOCK_DIVISION);


    // 2. Set the MAC address.
    // Write to the gem.spec_add1_bottom register.
    

    // Write to the gem.spec_add1_top register.
    




    
}

void Cadence::set_reg(unsigned long int pointer, unsigned int value) {
    Reg32 * p = reinterpret_cast<Reg32 *>(Memory_Map::ETH_BASE + pointer);
    Reg32 v = reinterpret_cast<Reg32>(value);
    *p = v;
    cout << v << endl;
}

void Cadence::set_bits(unsigned long int pointer, unsigned int value) {
    Reg32 * p = reinterpret_cast<Reg32 *>(Memory_Map::ETH_BASE + pointer);
    Reg32 v = reinterpret_cast<Reg32>(value);
    *p = *p | v;
}

__END_SYS

#endif
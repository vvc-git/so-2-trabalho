#ifndef __riscv_gem_h
#define __riscv_gem_h


#include <architecture/cpu.h>
#include <system.h>

__BEGIN_SYS

class Cadence_GEM
{
protected:
    // TODO: Talvez mudar para o tipo ADDRESS
    typedef CPU::Reg32 Reg32;
    typedef CPU::Reg16 Reg16;
    typedef CPU::Log_Addr Log_Addr;
    typedef CPU::Phy_Addr Phy_Addr;
    typedef Ethernet::Address Address;

public:
    // Register offsets
    enum
    {
        NETWORK_CONTROL = 0x00000000,
        NETWORK_CONFIG  = 0x00000004,
        NETWORK_STATUS  = 0x00000008,
        TRANSMIT_Q1_PTR = 0x00000440, 
        RECEIVE_Q1_PTR  = 0x00000480, 
        DMA_CONFIG      = 0x00000010,
        TRANSMIT_STATUS = 0x00000014,
        RECEIVE_Q_PTR   = 0x00000018,
        TRANSMIT_Q_PTR  = 0x0000001C,
        RECEIVE_STATUS  = 0x00000020,
        INT_DISABLE     = 0x0000002C,
        SPEC_ADD1_BOTTOM= 0x00000088,
        SPEC_ADD1_TOP   = 0x0000008c,
        INT_STATUS      = 0x00000024,
        INT_ENABLE      = 0X00000028,
    };

    // Network Control Register bits
    enum
    {
        CLEAR_ALL_STATS_REGS = 1 << 5,
        MAN_PORT_EN = 1 << 4, 
        ENABLE_TRANSMIT = 1 << 3,
        ENABLE_RECEIVE = 1 << 2,
        TX_START_PCLK = 1 << 9,

    };

    enum
    {
        // Network Config Register bits
        FULL_DUPLEX = 1 << 1,
        GIGABIT_MODE_ENABLE = 1 << 10,
        NO_BROADCAST = ~(1 << 5), // Bit que deve ser zero
        MULTICAST_HASH_ENABLE = 1 << 6,
        COPY_ALL_FRAMES = 1 << 4,
        RECEIVE_CHECKSUM_OFFLOAD_ENABLE = 1 << 24,
        PAUSE_ENABLE = 1 << 13,
        MDC_CLOCK_DIVISION = 0x1C0000,

        // DMA_CONFIG bits
        RX_BUF_SIZE  = 0x00190000,
        RX_PBUF_SIZE = 0x00000300,
        TX_PBUF_SIZE = 1 << 10,
        TX_PBUF_TCP_EN = 1 << 11,
        ENDIAN_SWAP_PACKET = ~(1 << 7), // Bit que deve ser zero
        AMBA_BURST_LENGTH = 0x10,

        // INT STATUS
        INT_TRASNMIT_COMPLETE = 1 << 7,  
        INT_RECEIVE_COMPLETE = 1 << 1,
        INT_RECEIVE_OVERRUN = 1 << 10, 

        // TRANSMIT STATUS
        TRANS_TRANSMIT_COMPLETE = 1 << 5,

    };

        // Descriptor RX
    enum : unsigned int
    {
        RX_WORD0_2_LSB = 0xfffffffc,
        RX_WORD0_LSB_WRP = 0x00000002,
        RX_OWN = (1 << 0), // 0 => NIC, 1 => Host
        GET_FRAME_LENGTH = 0x1FFF,
    };
    // Descriptor TX
    enum  
    {
        TX_WORD1_OWN_CONTROLLER = ~(1 << 31), 
        TX_WORD1_OWN_CPU = 1 << 31,
        TX_WORD1_WRP_BIT = 1 << 30,
    };



    // Utilizando modo de endereçamento de 64 bits
    struct Desc
    {
        // Word 0
        volatile Reg32 address;
        // Word 1
        volatile Reg32 control;

        void set_rx_address(Phy_Addr addr) { address = addr & RX_WORD0_2_LSB;};
        void set_rx_wrap() {address = address | RX_WORD0_LSB_WRP;};
        void reset_rx_control() {control = 0;};
        void set_rx_own_wrap(bool wrap) {
            address = address & RX_WORD0_2_LSB;
            if (wrap)
                set_rx_wrap();
        };

        void set_tx_address(Phy_Addr addr) {address = addr;};
        void set_tx_wrap() {control = control | TX_WORD1_WRP_BIT;};
        void set_tx_control() {control = TX_WORD1_OWN_CPU;};

        bool is_cpu_owned() {return (address & RX_OWN);}

        void set_ctrl_transmiting(unsigned int size) {

            // Seta o tamanho do buffer de dados a ser lido
            control = control | size;

            // For single buffer ehternet frame, bit[15] of word [1] must also be set.
            control = (1 << 15) | control;

            // Coloca o bit 31 como 0 (Bit que indica que a NIC poder ler)
            control = control & TX_WORD1_OWN_CONTROLLER;

        }

    };

    // construtor
    Cadence_GEM(){};
// 
    static void set_reg(unsigned long int pointer, unsigned int value)
    {
        Reg32 *p = reinterpret_cast<Reg32 *>(Memory_Map::ETH_BASE + pointer);
        Reg32 v = reinterpret_cast<Reg32>(value);
        *p = v;
    }

    static void set_bits(unsigned long int pointer, unsigned int value)
    {
        Reg32 *p = reinterpret_cast<Reg32 *>(Memory_Map::ETH_BASE + pointer);
        Reg32 v = reinterpret_cast<Reg32>(value);
        *p = *p | v;
    }

    // operador '&'
    static void set_bits_and(unsigned long int pointer, unsigned int value)
    {
        Reg32 *p = reinterpret_cast<Reg32 *>(Memory_Map::ETH_BASE + pointer);
        Reg32 v = reinterpret_cast<Reg32>(value);
        *p = *p & v;
    }

    static void set_receiver_ptr(Phy_Addr addr) {set_reg(RECEIVE_Q_PTR, addr);};
    static void set_transmiter_ptr(Phy_Addr addr) {set_reg(TRANSMIT_Q_PTR, addr);};
    

    void start_transmit() {set_bits(NETWORK_CONTROL, TX_START_PCLK);};

    static Address get_mac() {

        Address address;
        Reg32 * low = reinterpret_cast<Reg32*>(Memory_Map::ETH_BASE + SPEC_ADD1_BOTTOM);
        Reg32 * high = reinterpret_cast<Reg32*>(Memory_Map::ETH_BASE + SPEC_ADD1_TOP);

        address[0] = *low >> 0;
        address[1] = *low >> 8;
        address[2] = *low >> 16;
        address[3] = *low >> 24;
        address[4] = *high >> 0;
        address[5] = *high >> 8;

        return address;

    }
    
    static void init_regs() 
    {
        // 1. Clear the network control register
        //  Write 0x0 to the gem.network_control register.
        set_reg(NETWORK_CONTROL, 0x0);

        // 2. Clear the statistics registers
        // Write a 1 to the gem.network_control [clear_all_stats_regs].
        set_bits(NETWORK_CONTROL, CLEAR_ALL_STATS_REGS);

        // 3. Clear the status registers. Write a 1 to the status registers.
        // gem.receive_status = 0x0F
        set_reg(RECEIVE_STATUS, 0x0F);

        // gem.transmit_status = 0xFF.
        set_reg(TRANSMIT_STATUS, 0xFF);

        // 4.Disable all interrupts.
        // Write 0x7FF_FEFF to the gem. int_disable register.
        set_reg(INT_DISABLE, 0x7FFFEFF);

        // 5. Clear the buffer queues.
        // Write 0x0 to the gem.transmit_q{}_ptr
        set_reg(TRANSMIT_Q_PTR, 0x0);

        // Write 0x0 to the gem.transmit_q{1}_ptr
        set_reg(TRANSMIT_Q1_PTR, 0x0);

        // Write 0x0 to the gem.receive_q{}_ptr
        set_reg(RECEIVE_Q_PTR, 0x0);

        // Write 0x0 to the gem.receive_q{1}_ptr
        set_reg(RECEIVE_Q1_PTR, 0x0);

        // Configure the controller

        // 1. Enable full duplex.
        // a. Write a 1 to the gem.network_config[full_duplex] bit.
        set_bits(NETWORK_CONFIG, FULL_DUPLEX);

        // b. Enable gigabit mode.
        // Write a 1 to the gem.network_config[gigabit_mode_enable] bit.
        set_bits(NETWORK_CONFIG, GIGABIT_MODE_ENABLE);

        // c. Enable reception of broadcast or multicast frames.
        // Write a 0 to the gem.network_config[no_broadcast] register to enable broadcast frames.
        set_bits_and(NETWORK_CONFIG, NO_BROADCAST);


        // write a 1 to the gem.network_config[multicast_hash_en] bit to enable multicast frames.
        set_bits(NETWORK_CONFIG, MULTICAST_HASH_ENABLE);

        // d. Enable promiscuous mode.
        // Write a 1 to the gem.network_config[copy_all_frames] bit.
        set_bits(NETWORK_CONFIG, COPY_ALL_FRAMES);

        // e. Enable TCP/IP checksum offload feature on receive.
        // Write a 1 to the gem.network_config[receive_checksum_offload_enable] bit.
        set_bits(NETWORK_CONFIG, RECEIVE_CHECKSUM_OFFLOAD_ENABLE);

        // f.Enable pause frames. Write a 1 to gem.network_config[pause_enable] bit.
        set_bits(NETWORK_CONFIG, PAUSE_ENABLE);

        // g. Set the MDC clock divisor.
        // Write the appropriate MDC clock divisor to the gem.network_config[mdc_clock_division] bit.
        set_bits(NETWORK_CONFIG, MDC_CLOCK_DIVISION);

        // 2. Set the MAC address.
        // Write to the gem.spec_add1_bottom register.
        Reg32 * low = reinterpret_cast<Reg32*>(Memory_Map::ETH_BASE + SPEC_ADD1_BOTTOM);
        set_reg(SPEC_ADD1_BOTTOM, *low);
        
        
        // Write to the gem.spec_add1_top register. The
        // most significant 16 bits go to gem.spec_add1_top
        Reg32 * high = reinterpret_cast<Reg32*>(Memory_Map::ETH_BASE + SPEC_ADD1_TOP);
        set_reg(SPEC_ADD1_TOP, *high);


        // 3. Program the DMA configuration register (gem.dma_config)

        // a. Set the receive buffer size to 1,600 bytes. Write a value of 8'h19 to the
        // gem.dma_config[rx_buf_size] bit field. (escrevendo 24, pois 24*64 = 1536)
        set_reg(DMA_CONFIG, RX_BUF_SIZE);

        // b. Set the receiver packet buffer memory size to the full configured addressable space
        // of 32 KB. Write 2'b11 to the gem.dma_config[rx_pbuf_size] bit field
        set_bits(DMA_CONFIG, RX_PBUF_SIZE);

        // C. Set the transmitter packet buffer memory size to the full configured addressable
        // space of 32 KB. Write a 1 to the gem.dma_config[tx_pbuf_size] bit.
        set_bits(DMA_CONFIG, TX_PBUF_SIZE);

        // d. Enable TCP/IP checksum generation offload on the transmitter. Write a 1 to the
        // gem.dma_config[tx_pbuf_tcp_en] bit.
        set_bits(DMA_CONFIG, TX_PBUF_TCP_EN);

        // e. Configure for a little endian system. Write a 0 to the
        // gem.dma_config[endian_swap_packet] bit.
        set_bits_and(DMA_CONFIG, ENDIAN_SWAP_PACKET);

        // f. Configure AXI fixed burst length. Write 5'h10 to the
        // gem.dma_config[amba_burst_length] bit field to use INCR16 AXI burst for higher
        // performance.
        set_bits(DMA_CONFIG, AMBA_BURST_LENGTH);

        // 4. Program the network control register (gem.network_control).
        // a. Enable the MDIO. Write a 1 to the gem.network_control[man_port_en] bit.
        set_bits(NETWORK_CONTROL, MAN_PORT_EN);
        
        // b. Enable the transmitter. Write a 1 to the gem.network_control[enable_transmit] bit.
        set_bits(NETWORK_CONTROL, ENABLE_TRANSMIT);

        
        // !! ATENCAO
        Address address = get_mac();
        if (!(address[5] % 2)) {
            db<SiFiveU_NIC>(TRC) << "riscv::init_regs ENABLE_RECEIVE: "<< endl;
            db<SiFiveU_NIC>(TRC) << address << endl;

            // c. Enable the receiver. Write a 1 to the gem.network_control[enable_receive] bit.
            set_bits(NETWORK_CONTROL, ENABLE_RECEIVE);
        }
        
        //set_reg(INT_ENABLE, 0x2fffffff); // habilitando todas as interrupcoes
        set_reg(INT_ENABLE, INT_TRASNMIT_COMPLETE | INT_RECEIVE_OVERRUN | INT_RECEIVE_COMPLETE);

    }

};

__END_SYS

#endif
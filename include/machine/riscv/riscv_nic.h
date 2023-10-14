#ifndef __riscv_nic_h
#define __riscv_nic_h

#include <network/ethernet.h>
#include <utility/ct_buffer.h>
#include <architecture/cpu.h>
#include <system.h>
// #include <riscv_ic.h>


__BEGIN_SYS

class Cadence_GEM
{
protected:
    // TODO: Talvez mudar para o tipo ADDRESS
    typedef CPU::Reg32 Reg32;
    typedef CPU::Reg16 Reg16;
    typedef CPU::Log_Addr Log_Addr;
    typedef CPU::Phy_Addr Phy_Addr;

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

    // construtor
    Cadence_GEM(){};

    void set_reg(unsigned long int pointer, unsigned int value)
    {
        Reg32 *p = reinterpret_cast<Reg32 *>(Memory_Map::ETH_BASE + pointer);
        Reg32 v = reinterpret_cast<Reg32>(value);
        *p = v;
    }

    void set_bits(unsigned long int pointer, unsigned int value)
    {
        Reg32 *p = reinterpret_cast<Reg32 *>(Memory_Map::ETH_BASE + pointer);
        Reg32 v = reinterpret_cast<Reg32>(value);
        *p = *p | v;
    }

    // operador '&'
    void set_bits_and(unsigned long int pointer, unsigned int value)
    {
        Reg32 *p = reinterpret_cast<Reg32 *>(Memory_Map::ETH_BASE + pointer);
        Reg32 v = reinterpret_cast<Reg32>(value);
        *p = *p & v;
    }

};

// TODO: Mudara essa heranca CADENCE GEM
class SiFiveU_NIC : public Data_Observed<CT_Buffer, void>, Cadence_GEM
{

private:
    friend class Machine_Common;

    typedef CPU::Reg32 Reg32;
    typedef CPU::Reg32 Reg16;
    typedef CPU::Reg64 Reg64;
    typedef CPU::Phy_Addr Phy_Addr;
    typedef CPU::Log_Addr Log_Addr;
    typedef Ethernet::Frame Frame;
    typedef NIC<Ethernet>::Address Address;
    typedef IC::Interrupt_Id Interrupt_Id;

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
    };

public:
    SiFiveU_NIC();
    ~SiFiveU_NIC(){};
    void attach(Data_Observer<CT_Buffer, void> *o) { Data_Observed<CT_Buffer, void>::attach(o); };

   
    void receive(Address src, void* payload, unsigned int payload_size);
    void receive();
    void send(Address src, Address dst, char* payload, unsigned int payload_size);
    void init_regs();

     // Métododos para o tratamento de interrupções
    static void init();
    static void int_handler(Interrupt_Id interrupt = 1);
    void handle_interrupt();

public:


    // TODO: Talvez não armazenar esses CT_Buffer, porque o que importa dele é o endereço físico
    // TODO: que é obtido pelo DMA.
    CT_Buffer *tx_desc_buffer;
    CT_Buffer *tx_data_buffer;

    CT_Buffer *rx_desc_buffer;
    CT_Buffer *rx_data_buffer;

    // Endereço físico base
    Phy_Addr tx_desc_phy;
    Phy_Addr tx_data_phy;

    Phy_Addr rx_desc_phy;
    Phy_Addr rx_data_phy;

    // Endereço lógico base
    Log_Addr log_init_tx_desc;
    Log_Addr log_init_tx_data;

    Log_Addr log_init_rx_desc;
    Log_Addr log_init_rx_data;

    unsigned int DESC_SIZE = 8;
    unsigned int SLOTS_BUFFER = 64;
    unsigned int last_desc_idx = 0;

    // Atributos estáticos para serem acessados pelo tratador de interrupções
    static SiFiveU_NIC* _device;
    static Interrupt_Id _interrupt;

    // Address
    Address address;

    Reg32 mac_low;
    Reg16 mac_high;
};

__END_SYS

#endif
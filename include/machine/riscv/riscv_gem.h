#ifndef __riscv_gem_h
#define __riscv_gem_h

#include <architecture.h>
#include <utility/convert.h>
#include <network/ethernet.h>
#include <utility/string.h>
#include <machine/riscv/sifive_u/sifive_u_memory_map.h>

__BEGIN_SYS

OStream cout;

class Cadence_GEM
{
protected:
    typedef CPU::Reg32 Reg32;
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

    // Network Config Register bits
    enum
    {
        FULL_DUPLEX = 1 << 1,
        GIGABIT_MODE_ENABLE = 1 << 10,
        NO_BROADCAST = ~(1 << 5), // Bit que deve ser zero
        MULTICAST_HASH_ENABLE = 1 << 6,
        COPY_ALL_FRAMES = 1 << 4,
        RECEIVE_CHECKSUM_OFFLOAD_ENABLE = 1 << 24,
        PAUSE_ENABLE = 1 << 13,
        MDC_CLOCK_DIVISION = 0x1C0000,

        // DMA_CONFIG bits
        RX_BUF_SIZE  = 0x00180000,
        RX_PBUF_SIZE = 0x00000300,
        TX_PBUF_SIZE = 1 << 10,
        TX_PBUF_TCP_EN = 1 << 11,
        ENDIAN_SWAP_PACKET = ~(1 << 7), // Bit que deve ser zero
        AMBA_BURST_LENGTH = 0x10,

        // INT STATUS
        INT_TRASNMIT_COMPLETE = 1 << 7,   

        // TRANSMIT STATUS
        TRANS_TRANSMIT_COMPLETE = 1 << 5,

    };

    // construtor
    Cadence_GEM(){};

    // função que seta os valores dos registradores mapeados em memória
    void set_reg(unsigned long int pointer, unsigned int value);

    // função que retorna or lógico dos valores passados por parâmetros
    void set_bits(unsigned long int pointer, unsigned int value);

    void set_bits_and(unsigned long int pointer, unsigned int value);
};

void Cadence_GEM::set_reg(unsigned long int pointer, unsigned int value)
{
    Reg32 *p = reinterpret_cast<Reg32 *>(Memory_Map::ETH_BASE + pointer);
    Reg32 v = reinterpret_cast<Reg32>(value);
    *p = v;
}

void Cadence_GEM::set_bits(unsigned long int pointer, unsigned int value)
{
    Reg32 *p = reinterpret_cast<Reg32 *>(Memory_Map::ETH_BASE + pointer);
    Reg32 v = reinterpret_cast<Reg32>(value);
    *p = *p | v;
}

// operador '&'
void Cadence_GEM::set_bits_and(unsigned long int pointer, unsigned int value)
{
    Reg32 *p = reinterpret_cast<Reg32 *>(Memory_Map::ETH_BASE + pointer);
    Reg32 v = reinterpret_cast<Reg32>(value);
    *p = *p & v;
}



__END_SYS

#endif
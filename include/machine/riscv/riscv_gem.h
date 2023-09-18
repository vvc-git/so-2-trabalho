#ifndef __gem_h
#define __gem_h

#include <architecture.h>
#include <utility/convert.h>
#include <network/ethernet.h>
#include <utility/string.h>
#include <machine/riscv/if_cgem_hw.h>
#include <machine/riscv/sifive_u/sifive_u_memory_map.h>

__BEGIN_SYS

OStream cout;

class Cadence_GEM
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
    enum
    {
        NETWORK_CONTROL = 0x0000000000,
        NETWORK_CONFIG = 0x0000000004,
        NETWORK_STATUS = 0x0000000008,
        TRANSMIT_STATUS = 0x0000000014,
        RECEIVE_Q_PTR = 0x0000000018,
        TRANSMIT_Q_PTR = 0x000000001C,
        RECEIVE_STATUS = 0x0000000020,
        INT_DISABLE = 0x000000002C,
        SPEC_ADD1_BOTTOM = 0x0000000088,
        DMA_CONFIG = 0x0000000010,
    };

    // Network Control Register bits
    enum
    {
        CLEAR_ALL_STATS_REGS = 1 << 5,
    };

    // Network Config Register bits
    enum
    {
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
    Cadence_GEM();

    // função que seta os valores dos registradores mapeados em memória
    void set_reg(unsigned long int pointer, unsigned int value);

    // função que retorna or lógico dos valores passados por parâmetros
    void set_bits(unsigned long int pointer, unsigned int value);
};

Cadence_GEM::Cadence_GEM()
{

    // 1. Clear the network control register
    //  Write 0x0 to the gem.network_control register.
    Cadence_GEM::set_reg(NETWORK_CONTROL, 0x0);

    // 2. Clear the statistics registers
    // Write a 1 to the gem.network_control [clear_all_stats_regs].
    Cadence_GEM::set_bits(NETWORK_CONTROL, CLEAR_ALL_STATS_REGS);

    // 3. Clear the status registers

    // ?? Não conseguimos fazer essa:
    // Write a 1 to the status registers.

    // gem.receive_status = 0x0F
    Cadence_GEM::set_reg(RECEIVE_STATUS, 0x0F);

    // gem.transmit_status = 0xFF.
    Cadence_GEM::set_reg(TRANSMIT_STATUS, 0xFF);

    // 4.Disable all interrupts.
    // Write 0x7FF_FEFF to the gem. int_disable register.
    Cadence_GEM::set_reg(INT_DISABLE, 0x7FFFEFF);

    // 5. Clear the buffer queues.
    // Write 0x0 to the gem.receive_q{ , 1}_ptr
    // ?? Duvida se esta mesmo sentando, pois testamos com os WR e mesmo tava dando errado
    Cadence_GEM::set_reg(RECEIVE_Q_PTR, 0X0);

    // Write 0x0 to the gem.transmit_q{ , 1}_ptr
    // ?? Duvida se esta mesmo sentando, pois testamos com os WR e mesmo tava dando errado
    Cadence_GEM::set_reg(RECEIVE_Q_PTR, 0x0);

    // Configure the controller

    // 1. Enable full duplex.
    // a. Write a 1 to the gem.network_config[full_duplex] bit.
    Cadence_GEM::set_bits(NETWORK_CONFIG, FULL_DUPLEX);

    // b. Enable gigabit mode.
    // Write a 1 to the gem.network_config[gigabit_mode_enable] bit.
    Cadence_GEM::set_bits(NETWORK_CONFIG, GIGABIT_MODE_ENABLE);

    // c. Enable reception of broadcast or multicast frames.
    // Write a 0 to the gem.network_config[no_broadcast] register to enable broadcast frames.
    Cadence_GEM::set_bits(NETWORK_CONFIG, NO_BROADCAST);

    // write a 1 to the gem.network_config[multicast_hash_en] bit to enable multicast frames.
    Cadence_GEM::set_bits(NETWORK_CONFIG, MULTICAST_HASH_ENABLE);

    // d. Enable promiscuous mode.
    // Write a 1 to the gem.network_config[copy_all_frames] bit.
    Cadence_GEM::set_bits(NETWORK_CONFIG, COPY_ALL_FRAMES);

    // e. Enable TCP/IP checksum offload feature on receive.
    // Write a 1 to the gem.network_config[receive_checksum_offload_enable] bit.
    Cadence_GEM::set_bits(NETWORK_CONFIG, RECEIVE_CHECKSUM_OFFLOAD_ENABLE);

    // f.Enable pause frames. Write a 1 to gem.network_config[pause_enable] bit.
    Cadence_GEM::set_bits(NETWORK_CONFIG, PAUSE_ENABLE);

    // g. Set the MDC clock divisor.
    // Write the appropriate MDC clock divisor to the gem.network_config[mdc_clock_division] bit.
    // ?? QUAL CLOCK ESCOLHER?
    Cadence_GEM::set_bits(NETWORK_CONFIG, MDC_CLOCK_DIVISION);

    // 2. Set the MAC address.
    // Write to the gem.spec_add1_bottom register.

    // Write to the gem.spec_add1_top register.
}

void Cadence_GEM::set_reg(unsigned long int pointer, unsigned int value)
{
    Reg32 *p = reinterpret_cast<Reg32 *>(Memory_Map::ETH_BASE + pointer);
    Reg32 v = reinterpret_cast<Reg32>(value);
    *p = v;
    // cout << v << endl;
}

void Cadence_GEM::set_bits(unsigned long int pointer, unsigned int value)
{
    Reg32 *p = reinterpret_cast<Reg32 *>(Memory_Map::ETH_BASE + pointer);
    Reg32 v = reinterpret_cast<Reg32>(value);
    *p = *p | v;
}

__END_SYS

#endif
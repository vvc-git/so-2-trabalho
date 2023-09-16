#ifndef __pcnet32_h
#define __pcnet32_h

#include <network/ethernet.h>
#include <utility/ct_buffer.h>
#include <architecture/cpu.h>

__BEGIN_SYS

// OStream cout;

class Cadence_NIC {

private:
    friend class Machine_Common;

    //typedef CPU::Reg32 Reg32;
    typedef CPU::Reg64 Reg64;
    typedef CPU::Phy_Addr Phy_Addr;
    typedef CPU::Log_Addr Log_Addr;

    struct Desc {
        volatile Reg64 address;
        volatile Reg64 control;
    };

public:
    Cadence_NIC();
    ~Cadence_NIC() {};

public:
    CT_Buffer* buffer_tx_desc;
    CT_Buffer* buffer_tx_data;

    CT_Buffer* buffer_rx_desc;
    CT_Buffer* buffer_rx_data;

    Phy_Addr phy_init_tx_desc;
    Phy_Addr phy_init_tx_data;

    Log_Addr log_init_tx_desc;
    Log_Addr log_init_tx_data;

    unsigned int DATA_SIZE = 1500;
    unsigned int DESC_SIZE = 8;

};

Cadence_NIC::Cadence_NIC() {
    // Alocando memoria para os buffers tx de descritores e dados
    buffer_tx_desc = new CT_Buffer(16 * 10);
    buffer_tx_data = new CT_Buffer(1500 * 10);

    // Pegando endereço físico dos buffers para NIC
    phy_tx_desc = buffer_tx_desc->phy_address();
    phy_tx_data = buffer_tx_data->phy_address();

    // Pegando endereço lógico dos buffers para CPU
    // log_init_tx_desc = buffer_tx_desc->log_address();
    // log_init_tx_data = buffer_tx_data->log_address();

    Desc * desc = phy_init_tx_desc;
    desc->address = phy_init_tx_data;
    desc->control = 3;
    // Desc * tx_descriptor_log = log_init_tx_desc;

    ++desc;
    // ++tx_descriptor_log;
}

__END_SYS

#endif
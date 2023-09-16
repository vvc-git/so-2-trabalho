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

    typedef CPU::Reg32 Reg32;
    typedef CPU::Phy_Addr Phy_Addr;
    typedef CPU::Log_Addr Log_Addr;

    struct Desc {
        volatile Phy_Addr address;
        volatile Reg32 control;
    };

public:
    Cadence_NIC();
    ~Cadence_NIC() {};

public:
    CT_Buffer* tx_desc;
    CT_Buffer* tx_data;

    CT_Buffer* rx_desc;
    CT_Buffer* rx_data;

    int phy_init_tx_desc;
    int phy_init_tx_data;

    unsigned int DATA_SIZE = 1500;
    unsigned int DESC_SIZE = 8;

    // Log_Addr log_init_tx_desc;
    // Log_Addr log_init_tx_data;

};

Cadence_NIC::Cadence_NIC() {
    // Alocando memoria para os buffers tx
    tx_desc = new CT_Buffer(8 * 10);
    tx_data = new CT_Buffer(1500 * 10);

    // Pegando endereço físico dos buffers
    phy_init_tx_desc = tx_desc->phy_address();
    phy_init_tx_data = tx_data->phy_address();

    int * desc_addr = tx_desc->phy_address();

    *desc_addr = phy_init_tx_data;
    DATA_SIZE = 10;

    desc_addr += 4;
    *desc_addr = 2;


    DESC_SIZE = 9;
    
}

__END_SYS

#endif
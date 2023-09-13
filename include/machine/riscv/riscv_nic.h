#ifndef __pcnet32_h
#define __pcnet32_h

#include <network/ethernet.h>
#include <utility/ct_buffer.h>
#include <architecture/cpu.h>

__BEGIN_SYS

class Cadence_NIC {
    friend class Machine_Common;
    typedef CPU::Reg32 Reg32;

    struct Desc {
        volatile Reg32 adress;
        volatile Reg32 control;
    };

public:
    Cadence_NIC();
    ~Cadence_NIC();

public:
    CT_Buffer* tx_desc;
    CT_Buffer* tx_data;

    CT_Buffer* rx_desc;
    CT_Buffer* rx_data;

};

Cadence_NIC::Cadence_NIC() {
    tx_desc = new CT_Buffer(64 * 10);
    tx_data = new CT_Buffer(1500 * 10);

    // pegar endereco fisico de tx_data
    for (size_t i = 0; i < 1500 * 10; i += 1500)
    {
    }
    
}

__END_SYS

#endif
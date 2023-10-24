#ifndef __riscv_nic_h
#define __riscv_nic_h

#include <network/ethernet.h>
#include <architecture/cpu.h>
#include <system.h>
#include <machine/riscv/network_buffer.h>
#include <utility/ct_buffer.h>
#include <machine/riscv/riscv_gem.h>



__BEGIN_SYS

class SiFiveU_NIC : public Observed, private Cadence_GEM
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

public:
    SiFiveU_NIC();
    ~SiFiveU_NIC(){};
    void attach(Observer *o) { Observed::attach(o); };

    void receive();
    void send(Address dst, void* payload, unsigned int payload_size);
    void handle_interrupt();

    // Métodos estáticos
    static void init();
    static void int_handler(Interrupt_Id interrupt = 1);

public:

    // Atributos estáticos para serem acessados pelo tratador de interrupções
    static SiFiveU_NIC* _device;
    static Interrupt_Id _interrupt;

    // Endereço MAC da placa
    Address address;

};

__END_SYS

#endif
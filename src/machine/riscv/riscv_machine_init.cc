// EPOS RISC V Initialization

#include <machine.h>
#include <machine/riscv/riscv_nic.h>
#include <machine/riscv/network_buffer.h>
#include <machine/riscv/arp_manager.h>


__BEGIN_SYS

void Machine::pre_init(System_Info * si)
{
    CLINT::mtvec(CLINT::DIRECT, &IC::entry);

    Display::init();

    db<Init, Machine>(TRC) << "Machine::pre_init()" << endl;
}


void Machine::init()
{
    db<Init, Machine>(TRC) << "Machine::init()" << endl;

    if(Traits<IC>::enabled)
        IC::init();

    if(Traits<Timer>::enabled)
        Timer::init();

    Network_buffer::init();

    ARP_Manager::init();

    if(Traits<SiFiveU_NIC>::enabled)
        SiFiveU_NIC::init();
    
    ARP_Manager::_arp_mng->set_own_IP();
    
}

__END_SYS

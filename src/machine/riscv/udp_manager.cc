#include <machine/riscv/udp_manager.h>

__BEGIN_SYS

UDP_Manager* UDP_Manager::udp_mng;

void UDP_Manager::init() {
    db<UDP_Manager>(TRC) << "UDP_Manager::init()"<< endl;
    udp_mng = new (SYSTEM) UDP_Manager();
}

void UDP_Manager::hello() {
    db<UDP_Manager>(TRC) << "UDP_Manager::hello()"<< endl;
}

void UDP_Manager::update(Data_Observed<char, void> * o, char * d) {
    db<UDP_Manager>(TRC) << "UDP_Manager::update()"<< endl;
    db<UDP_Manager>(TRC) << *d << endl;
    hello();
}

__END_SYS
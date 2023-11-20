#include <machine/riscv/udp_manager.h>

__BEGIN_SYS

UDP_Manager* UDP_Manager::udp_mng;

void UDP_Manager::init() {
    db<UDP_Manager>(TRC) << "UDP_Manager::init()"<< endl;
    udp_mng = new (SYSTEM) UDP_Manager();
}

void UDP_Manager::hello() {
    db<UDP_Manager>(WRN) << "UDP_Manager::hello()"<< endl;
}

void UDP_Manager::update(Data_Observed<unsigned char, void> * o, unsigned char * d) {
    db<UDP_Manager>(WRN) << "UDP_Manager::update()"<< endl;
    for (unsigned int i = 20; i < 100; i++) {
        db<UDP_Manager>(WRN) << reinterpret_cast<char*>(d)[i];
    }
    db<UDP_Manager>(WRN) << endl;
    
    hello();
}

__END_SYS
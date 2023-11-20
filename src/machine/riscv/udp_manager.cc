#include <machine/riscv/udp_manager.h>

__BEGIN_SYS

UDP_Manager* UDP_Manager::udp_mng;

void UDP_Manager::init() {
    db<UDP_Manager>(TRC) << "UDP_Manager::init()"<< endl;
    udp_mng = new (SYSTEM) UDP_Manager();
}

void UDP_Manager::handle_package(char *data, unsigned int size) {
    db<UDP_Manager>(TRC) << "UDP_Manager::handle_package()"<< endl;
    for (unsigned int i = 20; i < size; i++) {
        db<UDP_Manager>(WRN) << data[i];
    }
    db<UDP_Manager>(WRN) << endl;
}

void UDP_Manager::update(Data_Observed<unsigned char, void> * o, unsigned char * d) {
    db<UDP_Manager>(TRC) << "UDP_Manager::update()"<< endl;
    IP::Header *header = reinterpret_cast<IP::Header*>(d);
    unsigned int size = ntohs(header->Total_Length) - 20;

    handle_package(reinterpret_cast<char*>(d), size);
    
}

__END_SYS
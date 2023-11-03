#include <machine/riscv/arp_manager.h>

__BEGIN_SYS

ARP_Manager* ARP_Manager::arp_mng;

void ARP_Manager::init() {
    db<ARP_Manager>(WRN) << "ARP_Manager::init()"<< endl;

    arp_mng = new (SYSTEM) ARP_Manager();
}

void ARP_Manager::arp_send() {
    db<ARP_Manager>(WRN) << "ARP_Manager::send()"<< endl;

}

void ARP_Manager::arp_receive() {
    db<ARP_Manager>(WRN) << "ARP_Manager::receive()"<< endl;

}

__END_SYS
#include <machine/riscv/arp_manager.h>

__BEGIN_SYS

ARP_Manager* ARP_Manager::_arp_mng;

void ARP_Manager::init() {
    db<ARP_Manager>(WRN) << "ARP_Manager::init()"<< endl;

    _arp_mng = new (SYSTEM) ARP_Manager();
}

void ARP_Manager::arp_send() {

    ARP_Packet* packet = new ARP_Packet();

    db<ARP_Manager>(WRN) << "ARP_Manager::send()"<< endl;
    
    // Setando os pacotes
    packet->_hw_type = 1;
    packet->_prot_type = 1;
    packet->_hw_length = 1;
    packet->_prot_length = 1;
    packet->_operation = 1;
    packet->_sender_hw = 1;
    packet->_sender_prot = 1;
    packet->_target_hw = 1;
    packet->_target_prot = 1;

    NIC<Ethernet>::Address dst;
    dst[0] = 0x00;
    dst[1] = 0x00;
    dst[2] = 0x00;
    dst[3] = 0x00;
    dst[4] = 0x00;
    dst[5] = 0x02;

    SiFiveU_NIC::_device->send(dst, (void*) &packet, 500, 0x0806);


}

void ARP_Manager::arp_receive() {
    db<ARP_Manager>(WRN) << "ARP_Manager::receive()"<< endl;

}

__END_SYS
#ifndef __arp_manager_h
#define __arp_manager_h

#include <system.h>
#include <network/arp.h>
#include <machine/riscv/riscv_nic.h>

__BEGIN_SYS

class ARP_Manager {

    typedef Arp::Packet ARP_Packet;
    typedef Ethernet::Protocol Protocol;
    typedef NIC_Common::Address<6> Address;

    public:
        void arp_send();
        void arp_receive();
        static void init();

    public:
        static ARP_Manager* _arp_mng;
};
__END_SYS

#endif
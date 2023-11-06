#ifndef __arp_manager_h
#define __arp_manager_h

#include <system.h>
#include <network/arp.h>
#include <machine/riscv/riscv_nic.h>
#include <utility/string.h>

__BEGIN_SYS

class ARP_Manager {

    typedef Arp::Packet ARP_Packet;
    typedef Ethernet::Protocol Protocol;
    typedef NIC_Common::Address<6> Address;

    public:
        void arp_send_request();
        void arp_send_reply(ARP_Packet* requester_packet);
        void arp_receive(ARP_Packet* packet);
        static void init();
        void set_own_IP();

    public:
        static ARP_Manager* _arp_mng;
        char IP_ADDR[4];
};
__END_SYS

#endif
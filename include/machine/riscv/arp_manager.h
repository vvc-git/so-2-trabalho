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

    private:
        Address * get_mac_in_table(unsigned char * ip);
        bool is_my_ip(unsigned char * ip);
        bool is_my_network(unsigned char * ip);

    public:
        bool arp_send_request();
        void arp_send_reply(ARP_Packet* requester_packet);
        void arp_receive(ARP_Packet* packet);
        static void init();
        void set_own_IP();

    public:
        static ARP_Manager* _arp_mng;
        unsigned char IP_ADDR[4];

        struct ARPTableEntry {
            unsigned char * ip; 
            Address mac;
        };

        typedef Simple_List<ARPTableEntry> List;
        typedef typename List::Element Element;

        // Lista de infos dos datagramas em construção
        List * ARP_Table = new List;

        // Rede com dois hosts (252)
        char submask[4] = {255, 255, 255, 252};

        int send =0 ;
        int reply =0 ;
};
__END_SYS

#endif
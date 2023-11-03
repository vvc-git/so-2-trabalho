#ifndef __arp_manager_h
#define __arp_manager_h

#include <system.h>

__BEGIN_SYS

class ARP_Manager {
    public:
        void arp_send();
        void arp_receive();
        static void init();

    public:
        static ARP_Manager* arp_mng;
};
__END_SYS

#endif
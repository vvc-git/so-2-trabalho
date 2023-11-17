#ifndef __icmp_manager_h
#define __icmp_manager_h

#include <system.h>
#include <network/ip.h>
#include <machine/riscv/riscv_nic.h>
#include <utility/string.h>
#include <synchronizer.h>
#include <utility/observer.h>

__BEGIN_SYS

class ICMP_Manager
{

    typedef CPU::Reg8 Reg8;
    typedef CPU::Reg16 Reg16;
    typedef IP::Echo Echo;
    typedef Ethernet::Frame Frame;
    typedef NIC_Common::Address<6> Address;

private:
    void set_header(Echo * echo, bool request);


public:
    static void init();
    void send(unsigned char * dst_ip, Address * dst_mac);
    void receive(void* request);


public:

    enum {
        MORE_FRAGS = 0X2000,
        LAST_FRAG = 0X1FFF,
        GET_OFFSET = 0x1fff,
        GET_FLAGS = 0xe000, 
    };


    static ICMP_Manager* _icmp_mng;
    int id_send = 0;

};
__END_SYS

#endif
// EPOS Buffer Declarations

#ifndef __arp_h
#define __arp_h

#include <system.h>
#include <network/ethernet.h>

__BEGIN_SYS

class 
Arp
{

    typedef CPU::Reg8 Reg8;
    typedef CPU::Reg16 Reg16;
    typedef CPU::Reg32 Reg32;
    typedef NIC_Common::Address<6> Address;


public:

    class Header
    {
    public:
        Header() {}

    public:
        Reg16 _hw_type;
        Reg16 _prot_type; 
        Reg8 _hw_length;
        Reg8 _prot_length;
        Reg16 _operation;
        Address _sender_hw;
        unsigned char _sender_prot[4];
        Address _target_hw;
        unsigned char _target_prot[4];

    } __attribute__((packed));
    
    class Packet: public Header
    {
    public:
        Packet() {db<Packet>(TRC) << "ARP::Packet"<< endl;};
        Header * header() { return this; }

    } __attribute__((packed));

};

__END_SYS

#endif


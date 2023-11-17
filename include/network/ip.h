// EPOS Buffer Declarations

#ifndef __ip_h
#define __ip_h

#include <system.h>
#include <network/ethernet.h>

__BEGIN_SYS

class IP
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
        Reg8 Version_IHL; // Version and Internet Header Length
        Reg8 Type_Service;
        Reg16 Total_Length;
        Reg16 Identification;
        Reg16 Flags_Offset;
        Reg8 TTL; // Time to live
        Reg8 Protocol; // This field indicates the next level protocol used in the data portion of the internet datagram.
        Reg16 Header_Checksum;
        unsigned char SRC_ADDR[4];
        unsigned char DST_ADDR[4];

    } __attribute__((packed));


    // Data and Trailer
    static const unsigned int MTU = 1480;
    typedef unsigned char Data[MTU];
    
    class Fragment: public Header
    {
    public:
        Fragment() {db<Fragment>(TRC) << "IP::Fragment"<< endl;};
        Header * header() { return this; }

        Reg8 Type;
        Reg8 Code;
        Reg16 Checksum;
        Reg16 Identifer;
        Reg16 Sequence_Number;
        unsigned char data[1480];

    } __attribute__((packed));

    class Echo: public Header
    {
    public:
        Echo() {db<Echo>(TRC) << "ICMP::Echo Message"<< endl;};
        Reg8 Type;
        Reg8 Code;
        Reg16 Checksum;
        Reg16 Identifer;
        Reg16 Sequence_Number;

    }__attribute__((packed));

};

__END_SYS

#endif


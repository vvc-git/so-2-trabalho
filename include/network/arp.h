// EPOS Buffer Declarations

#ifndef __arp_h
#define __arp_h

#include <system.h>


__BEGIN_SYS

class 
Arp
{

    typedef CPU::Reg8 Reg8;
    typedef CPU::Reg16 Reg16;
    typedef CPU::Reg32 Reg32;


// public:
// The Ethernet Header (RFC 894)
    class Header
    {
    public:
        Header() {}
        Header(Reg32 sender_hw, Reg32 sender_prot, Reg32 target_hw, Reg32 target_prot) : _sender_hw(sender_hw), _sender_prot(sender_prot), _target_hw(target_hw), _target_prot(target_hw) {}

        // const Reg32 & src() const { return _src; }
        // const Reg32 & dst() const { return _dst; }

        // Protocol prot() const { return ntohs(_prot); }

    protected:
        Reg16 _hw_type;
        Reg16 _prot_type;
        Reg8 _hw_length;
        Reg8 _prot_length;
        Reg16 _operation;
        Reg32 _sender_hw;
        Reg32 _sender_prot;
        Reg32 _target_hw;
        Reg32 _target_prot;

    }


protected:
    Arp() {}

public:

    // static const Address broadcast() { return Address::BROADCAST; }


};

__END_SYS

#endif


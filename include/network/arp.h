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


public:

    class Header
    {
    public:
        Header() {}
        Header(Reg32 sender_hw, Reg32 sender_prot, Reg32 target_hw, Reg32 target_prot) : _sender_hw(sender_hw), _sender_prot(sender_prot), _target_hw(target_hw), _target_prot(target_hw) {}

        // const Reg32 & src() const { return _src; }
        // const Reg32 & dst() const { return _dst; }

        // Protocol prot() const { return ntohs(_prot); }

    public:
        Reg16 _hw_type;
        Reg16 _prot_type;
        Reg8 _hw_length;
        Reg8 _prot_length;
        Reg16 _operation;
        Reg32 _sender_hw;
        Reg32 _sender_prot;
        Reg32 _target_hw;
        Reg32 _target_prot;

    };
    
    class Packet: public Header
    {
    public:
        Packet() {}
        // Packet(const Address & src, const Address & dst, const Protocol & prot): Header(src, dst, prot) {}
        Packet(Reg32 sender_hw, Reg32 sender_prot, Reg32 target_hw, Reg32 target_prot, const void * data, unsigned int size): Header(sender_hw, sender_prot, target_hw, target_prot) { memcpy(_data, data, size); }

        Header * header() { return this; }

        // template<typename T>
        // T * data() { return reinterpret_cast<T *>(&_data); }

        // friend Debug & operator<<(Debug & db, const Packet & f) {
        //     db << "{h=" << reinterpret_cast<const Header &>(f) << ",d=" << f._data << "}";
        //     return db;
        // }

    protected:
        char _data[1500];
    };

};

__END_SYS

#endif


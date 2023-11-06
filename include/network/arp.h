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
        // Header(const Address & sender_hw, Reg32 sender_prot, const Address & target_hw, Reg32 target_prot) : _sender_hw(sender_hw),  _sender_prot(sender_prot),_target_hw(target_hw),  _target_prot(target_prot) {}

        // const Reg32 & src() const { return _src; }
        // const Reg32 & dst() const { return _dst; }

        // Protocol prot() const { return ntohs(_prot); }

    public:
        Reg16 _hw_type;
        Reg16 _prot_type; 
        Reg8 _hw_length;
        Reg8 _prot_length;
        Reg16 _operation;
        Address _sender_hw;
        char _sender_prot[4];
        Address _target_hw;
        char _target_prot[4];

    } __attribute__((packed));;
    
    class Packet: public Header
    {
    public:
        Packet() {db<Packet>(WRN) << "ARP::Packet"<< endl;};
        // Packet(const Address & src, const Address & dst, const Protocol & prot): Header(src, dst, prot) {}
        //Packet(Reg32 sender_hw, Reg32 sender_prot, Reg32 target_hw, Reg32 target_prot) : Header(sender_hw, sender_prot, target_hw, target_prot) {db<Packet>(WRN) << "ARPr::Packet"<< endl;};
        //Packet(Reg32 sender_hw, Reg32 sender_prot, Reg32 target_hw, Reg32 target_prot, const void * data, unsigned int size): Header(sender_hw, sender_prot, target_hw, target_prot) { memcpy(_data, data, size); }

        Header * header() { return this; }

        // template<typename T>
        // T * data() { return reinterpret_cast<T *>(&_data); }

        // friend Debug & operator<<(Debug & db, const Packet & f) {
        //     db << "{h=" << reinterpret_cast<const Header &>(f) << ",d=" << f._data << "}";
        //     return db;
        // }

    // protected:
        // char _data[18];
    } __attribute__((packed));

};

__END_SYS

#endif


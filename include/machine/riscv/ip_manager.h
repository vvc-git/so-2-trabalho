#ifndef __ip_manager_h
#define __ip_manager_h

#include <system.h>
#include <network/ip.h>
#include <machine/riscv/riscv_nic.h>
#include <utility/string.h>

__BEGIN_SYS

// Informações de cada datagrama sendo montado
struct INFO 
{
    unsigned int id;
    unsigned int num_fragments = 0;
    unsigned int total_length;
    Simple_List<IP::Fragment>  * fragments;

};

struct IPTableEntry 
{
        unsigned char destination[4];
        unsigned char gateway[4];
        unsigned char genmask[4];

};

class IP_Manager {

    typedef CPU::Reg8 Reg8;
    typedef CPU::Reg16 Reg16;
    typedef IP::Fragment Fragment;
    typedef NIC_Common::Address<6> Address;
    typedef Simple_List<INFO> List;
    typedef typename List::Element Element;
    typedef Simple_List<IPTableEntry> IP_Table;
    typedef typename IP_Table::Element IP_Element;


public:
    static void init();
    void send(unsigned char* data, unsigned int data_size, unsigned char * dst_ip, Address * dst_mac);
    void receive(void* data, bool retransmit);
    Address * find_mac(unsigned char* dst_ip);
    bool is_my_network(unsigned char * dst_ip);
    bool is_localhost(unsigned char * dst_ip);
    void add_entry(unsigned char* dst, unsigned char* gateway, unsigned char* genmask);
    void populate_routing_table();
    void routing(unsigned char * ip, unsigned int total_length, unsigned char * data);


public:
    // https://www.rfc-editor.org/rfc/rfc791#page-11
    enum {
        MORE_FRAGS = 0X2000,
        LAST_FRAG = 0X1FFF,
        GET_OFFSET = 0x1fff,
        GET_FLAGS = 0xe000, 
    };

public:

    static IP_Manager* _ip_mng;

    // Datagrama counter
    unsigned int id_send = 1;
    
    // Lista de infos dos datagramas em construção
    List * dt_list = new List;

    // Lista de infos dos datagramas em construção
    IP_Table * routing_table = new IP_Table;

    //
    IP_Element * default_router;
    IP_Element * localhost;
    IP_Element * internal;
    IP_Element * external;
};
__END_SYS

#endif
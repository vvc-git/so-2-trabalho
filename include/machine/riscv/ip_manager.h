#ifndef __ip_manager_h
#define __ip_manager_h

#include <system.h>
#include <network/ip.h>
#include <machine/riscv/riscv_nic.h>
#include <utility/string.h>
#include <synchronizer.h>
#include <utility/observer.h>
#include <machine/riscv/udp_manager.h>

__BEGIN_SYS

// Informações de cada datagrama sendo montado
struct INFO 
{
    unsigned int id;
    unsigned int num_fragments = 0;
    unsigned int total_length;
    // Lista ordenada de fragmentos por offset
    Simple_List<IP::Fragment>  * fragments;
    Alarm * timer;
    Functor_Handler<INFO> * timeout_handler;
    Semaphore * sem;
};

struct IPTableEntry 
{
    unsigned char destination[4];
    unsigned char gateway[4];
    unsigned char genmask[4];

};

class IP_Manager: public Data_Observed<char, void> {

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
    void receive(void* data);
    Address * find_mac(unsigned char* dst_ip);
    bool is_my_network(unsigned char * dst_ip);
    bool is_localhost(unsigned char * dst_ip);
    void add_entry(unsigned char* dst, unsigned char* gateway, unsigned char* genmask);
    void populate_routing_table();
    void routing(void * datagram);
    void clear_dt_info(INFO * dt_info);
    static void timeout_handler(INFO * dt_info);
    void* defragmentation(INFO * dt_info);
    static int handler();
    // void attach(Data_Observer<char, void> *o) { Data_Observed<char, void>::attach(o); };


public:
    // https://www.rfc-editor.org/rfc/rfc791#page-11
    enum {
        MORE_FRAGS = 0X2000,
        LAST_FRAG = 0X1FFF,
        GET_OFFSET = 0x1fff,
        GET_FLAGS = 0xe000, 
        GET_IHL = 0x0F,
    };

public:

    static IP_Manager* _ip_mng;

    // Datagrama counter
    unsigned int id_send = 1;
    
    // Lista de infos dos datagramas em construção (Incompletos)
    List * dt_list = new List;

    // Lista de infos dos datagramas construidos (Completos)
    List * complete_dtgs = new List;

    // Lista de infos dos datagramas em construção
    IP_Table * routing_table = new IP_Table;

    // Semáforo para controlar acesso da Thread que remonta os datagramas
    Semaphore * sem_th = new Semaphore(0);

    // Thread que vai desfragmentar os pacotes e decidir se passa para camada de cima ou 
    // roteia
    // static Thread * _ip_thread;
    
    IP_Element * default_router;
    IP_Element * localhost;
    IP_Element * internal;
    IP_Element * external;
};
__END_SYS

#endif
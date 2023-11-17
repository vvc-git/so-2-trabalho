// EPOS Buffer Declarations

#ifndef __network_buffer_h
#define __network_buffer_h

#define FRAME_SIZE 1600
#define MAX_DMA_BUFFER_SIZE 1600*64

#include <utility/list.h>
#include <system.h>
#include <utility/heap.h>
#include <utility/string.h>
#include <architecture/mmu.h>
#include <utility/observer.h>
#include <utility/ct_buffer.h>
#include <synchronizer.h>
#include <network/ethernet.h>
#include <network/arp.h>
#include <machine/riscv/riscv_gem.h>
#include <machine/riscv/riscv_nic.h>
#include <time.h>
#include <machine/riscv/arp_manager.h>
#include <machine/riscv/ip_manager.h>
#include <machine/riscv/udp_manager.h>
#include <machine/riscv/icmp_manager.h>



__BEGIN_SYS


// Observador da camada do buffer da nic

class 
Network_buffer :  public Observer// , Data_Observed<DT_Buffer, void>
{
    friend class Cadence_GEM;

    typedef CPU::Reg8 Reg8;
    typedef CPU::Reg16 Reg16;
    typedef CPU::Reg32 Reg32;
    typedef CPU::Phy_Addr Phy_Addr;
    typedef CPU::Log_Addr Log_Addr;
    typedef Cadence_GEM::Desc Desc;
    typedef Heap DT_Buffer;
    typedef Ethernet::Protocol Protocol;
    typedef Arp::Packet ARP_Packet;
    typedef NIC_Common::Address<6> Address;
    typedef Ethernet::Frame Frame;
    typedef IP::Fragment Fragment;
    typedef IP::Echo Echo;

public:
    
    Network_buffer(); 
    ~Network_buffer() {};
    static void init();

    // Função que notifica o observador
    void update(Observed *obs);

    // Configuração dos buffer rx e tx
    void configure_tx_rx();

    // Gettr para um descritor livre
    Desc * get_free_tx_desc();

   // Função de execução da thread
   static int copy();

public:
    
    // Método estático de acesso para a nic
    static Network_buffer* net_buffer;
    Thread * thread;
    Semaphore * sem;
    CT_Buffer * buf;


    CT_Buffer *tx_desc_buffer;
    CT_Buffer *tx_data_buffer;

    CT_Buffer *rx_desc_buffer;
    CT_Buffer *rx_data_buffer;


    // Endereço físico base
    Phy_Addr tx_desc_phy;
    Phy_Addr tx_data_phy;

    Phy_Addr rx_desc_phy;
    Phy_Addr rx_data_phy;

    // Endereço lógico base
    Log_Addr log_init_tx_desc;
    Log_Addr log_init_tx_data;

    Log_Addr log_init_rx_desc;
    Log_Addr log_init_rx_data;

    DT_Buffer * dt;

    unsigned int DESC_SIZE = 8;
    unsigned int SLOTS_BUFFER = 64;
    unsigned int last_desc_idx = 0;


};


__END_SYS

#endif

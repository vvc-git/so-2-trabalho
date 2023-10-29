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
#include <machine/riscv/riscv_gem.h>
#include <machine/riscv/riscv_nic.h>
#include <time.h>



__BEGIN_SYS


// Observador da camada do buffer da nic
// Observado pela aplicação

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

    // typedef Ethernet::Frame Frame;
public:
    

    // https://www.rfc-editor.org/rfc/rfc791#page-11
    struct Datagram_Header {
        Reg8 Version_IHL; // Version and Internet Header Length
        Reg8 Type_Service;
        Reg16 Total_Length;
        Reg16 Identification;
        Reg16 Flags_Offset;
        Reg8 TTL; // Time to live
        Reg8 Protocol; // This field indicates the next level protocol used in the data portion of the internet datagram.
        Reg16 Header_Checksum;
        Reg32 SRC_ADDR;
        Reg32 DST_ADDR;
        // Reg32 Option_Padding;

        // Reg16 UDP1 = 1;
        // Reg16 UDP2 = 2;
        // Reg16 UDP3 = 3;
        // Reg16 UDP4 = 4;

        // setting some values in the constructor
        // Datagram_Header():
        //     Version_IHL(0x45),
        //     Type_Service(0x0),
        //     TTL(60),
        //     Protocol(253), // Testing - Conferir
        //     Header_Checksum(0x0),
        //     SRC_ADDR(0x0100007F), // 127.0.0.1
        //     DST_ADDR(0x0200007F) // 127.0.0.2
        //     // Option_Padding(0x0) // Conferir
        // {}
    };

    enum {
        // Const for logical operations. Deixar aqui?
        // TODO: MAY FRAGMENTS NAO DEVERIA SER SETADO?
        MORE_FRAGS = 0X2000,
        LAST_FRAG = 0X1FFF,
        GET_OFFSET = 0x1fff, 
        GET_FLAGS = ~GET_OFFSET & 0xFFFF, 
    };

    struct Datagram_Fragment {
        Datagram_Header header;
        char data[1480]; // tamanho fixo?
    };

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

   void IP_send(char* data, unsigned int data_size);
   void IP_receive(void* data);


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

    // Datagrama counter
    unsigned int id_send = 1;


    struct INFO 
    {
        unsigned int id;
        void * base;
        unsigned int num_frames;
    };

    typedef Simple_List<INFO> List;
    typedef typename List::Element Element;

    List * dt_list = new List;
    
    //!! APAGAR!
    bool dummy = true;


};


__END_SYS

#endif

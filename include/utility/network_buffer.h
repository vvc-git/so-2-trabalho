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


__BEGIN_UTIL


// Observador da camada do buffer da nic
// Observado pela aplicação

class 
Network_buffer :  public Observer // , Data_Observed<DT_Buffer, void>
{
    
    typedef CPU::Phy_Addr Phy_Addr;
    typedef CPU::Log_Addr Log_Addr;
    // typedef Ethernet::Frame Frame;


public:
    
    Network_buffer(); 
    ~Network_buffer() {};
    static void init();

    // Teste
   void update(Observed *obs);
   static int copy_for_upper_layer();



public:
    
    static Network_buffer* net_buffer;
    Thread * thread;
    Semaphore * sem;
    CT_Buffer * buf;
    char data[FRAME_SIZE];

};


__END_UTIL

#endif

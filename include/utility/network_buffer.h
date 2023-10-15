// EPOS Buffer Declarations

#ifndef __network_buffer_h
#define __network_buffer_h
#define FRAME_SIZE_ 1600
#define MAX_DMA_BUFFER_SIZE 1600*64

#include <utility/list.h>
#include <system.h>
#include <utility/heap.h>
#include <utility/string.h>
#include <architecture/mmu.h>


__BEGIN_UTIL


//OStream cout;

class Network_buffer
{


public:
    
    Network_buffer(void * addr, unsigned long bytes): app(addr, bytes), dma(MAX_DMA_BUFFER_SIZE){dma_ptr=dma.log_address();}; 
    ~Network_buffer() {};
    void * alloc(unsigned long int bytes);
    void free(void * ptr, unsigned long bytes) {app.free(ptr, bytes);};
    void alloc_frame(char frame[]);
    void get_dma_data(char * dma_data);
    void set_dma_data(char * dma_data, int amnt_frames);
    static void init();


public:

    Heap app;
    No_MMU::DMA_Buffer dma;
    char * dma_ptr;
    int count_frames = 0;
    
    static Network_buffer* net_buffer;

};


__END_UTIL

#endif

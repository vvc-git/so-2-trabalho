// EPOS Buffer Declarations

#ifndef __network_buffer_h
#define __network_buffer_h
#define MAX_DMA_BUFFER_SIZE 4500
#define FRAME_SIZE 1500

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
    void set_dma_data(char * dma_data);


public:

    Heap app;
    No_MMU::DMA_Buffer dma;
    char * dma_ptr;
    int count_frames = 0;

};

void * Network_buffer::alloc(unsigned long int bytes) {
    void * addr = app.alloc(bytes);

    return addr;

};

// alocando os frames no dma_buffer
void Network_buffer::alloc_frame(char frame[]) {
    if (count_frames <= MAX_DMA_BUFFER_SIZE/FRAME_SIZE) {
        memcpy(dma_ptr, frame, FRAME_SIZE);
        dma_ptr += FRAME_SIZE; 
        ++count_frames;
    }
};

void Network_buffer::get_dma_data(char * dma_data) {
    // serão copiados apenas os frames alocados, por enquanto o dma_buffer inteiro
    memcpy(dma_data, dma.log_address(), count_frames*FRAME_SIZE );
    dma_ptr = dma.log_address();
    count_frames = 0;
};

// Recebe os dados da rede.
// Por enquanto, preenche todo o dma buffer, seta o ponteiro dma para o final do buffer 
// e seta count_frames para o valor máximo
void Network_buffer::set_dma_data(char * dma_data) {
    memcpy(dma.log_address(), dma_data, MAX_DMA_BUFFER_SIZE );
    dma_ptr = dma.log_address()+MAX_DMA_BUFFER_SIZE;
    count_frames = MAX_DMA_BUFFER_SIZE/FRAME_SIZE;
};


__END_UTIL

#endif

// EPOS Buffer Declarations

#ifndef __CT_Buffer_h
#define __CT_Buffer_h
#define FRAME_SIZE 1500

#include <utility/list.h>
#include <system.h>
#include <utility/heap.h>
#include <utility/string.h>
#include <architecture/mmu.h>
#include <synchronizer.h>



__BEGIN_UTIL


//OStream cout;

class CT_Buffer
{


public:
    
    CT_Buffer(unsigned long bytes): dma(bytes), buffer_size(bytes) {dma_ptr=dma.log_address();}; 
    ~CT_Buffer() {};
    // void * alloc(unsigned long int bytes);
    // void free(void * ptr, unsigned long bytes) {app.free(ptr, bytes);};
    void alloc_frame(char frame[]);
    void get_dma_data(char * dma_data);
    void set_dma_data(char * dma_data, int amnt_frames);
    // Retornar endereço físico

public:
    No_MMU::DMA_Buffer dma;
    char * dma_ptr;
    int count_frames = 0;
    int buffer_size;
};

// alocando os frames no dma_buffer
void CT_Buffer::alloc_frame(char frame[]) {
    if (count_frames <= buffer_size/FRAME_SIZE) {
        memcpy(dma_ptr, frame, FRAME_SIZE);
        dma_ptr += FRAME_SIZE; 
        ++count_frames;
    }
};

void CT_Buffer::get_dma_data(char * dma_data) {
    // serão copiados apenas os frames alocados, por enquanto o dma_buffer inteiro
    memcpy(dma_data, dma.log_address(), count_frames*FRAME_SIZE );
    dma_ptr = dma.log_address();
    count_frames = 0;
};

// Recebe os dados da rede.
// Por enquanto, preenche todo o dma buffer, seta o ponteiro dma para o final do buffer 
// e seta count_frames para o valor máximo
void CT_Buffer::set_dma_data(char * dma_data, int amnt_frames) {
    unsigned int bytes = amnt_frames*FRAME_SIZE;
    memcpy(dma.log_address(), dma_data, bytes);
    dma_ptr = dma.log_address()+bytes;
    count_frames = amnt_frames;
};


__END_UTIL

#endif

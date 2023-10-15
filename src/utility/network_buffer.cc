#include <utility/network_buffer.h>

__BEGIN_UTIL

Network_buffer* Network_buffer::net_buffer;

void Network_buffer::init() {
    char buff[64*1600];
    net_buffer = new (SYSTEM) Network_buffer(buff, 64*1600);
}


void * Network_buffer::alloc(unsigned long int bytes) {
    void * addr = app.alloc(bytes);

    return addr;

};

// alocando os frames no dma_buffer
void Network_buffer::alloc_frame(char frame[]) {
    if (count_frames <= MAX_DMA_BUFFER_SIZE/FRAME_SIZE_) {
        memcpy(dma_ptr, frame, FRAME_SIZE_);
        dma_ptr += FRAME_SIZE_; 
        ++count_frames;
        db<Network_buffer>(WRN) << "allocated one frame in network_buffer" << endl;
    }
};

void Network_buffer::get_dma_data(char * dma_data) {
    // serão copiados apenas os frames alocados, por enquanto o dma_buffer inteiro
    memcpy(dma_data, dma.log_address(), count_frames*FRAME_SIZE_ );
    dma_ptr = dma.log_address();
    count_frames = 0;
};

// Recebe os dados da rede.
// Por enquanto, preenche todo o dma buffer, seta o ponteiro dma para o final do buffer 
// e seta count_frames para o valor máximo
void Network_buffer::set_dma_data(char * dma_data, int amnt_frames) {
    unsigned int bytes = amnt_frames*FRAME_SIZE_;
    memcpy(dma.log_address(), dma_data, bytes);
    dma_ptr = dma.log_address()+bytes;
    count_frames = amnt_frames;
};

__END_UTIL
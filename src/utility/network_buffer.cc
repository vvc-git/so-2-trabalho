#include <utility/network_buffer.h>

__BEGIN_UTIL

Network_buffer* Network_buffer::net_buffer;

// Network_buffer::Network_buffer(void * addr, unsigned long bytes) {
    
//     //thread = new Thread(&Network_buffer::copy_for_upper_layer);
    
//     // mutex.lock();

// }

// int Network_buffer::copy_for_upper_layer() {

    
    
//     net_buffer->thread->suspend();
//     while (true) {


//         net_buffer->thread->suspend();

//     }
//     // net_buffer->mutex.lock();
//     return 0;
// }

void Network_buffer::init() {
    char buff[64*1600];
    net_buffer = new (SYSTEM) Network_buffer(buff, 64*1600);

}

void Network_buffer::update(Data_Observed<CT_Buffer, void> *obs, CT_Buffer* buffer)
{
    // Faz a copia do buffer rx para data
    char  data[FRAME_SIZE];
    buffer->get_data_frame(data);

    db<SiFiveU_NIC>(WRN) << "Network buffer update: "<< endl;
    for (int i = 0; i < 1500; i++) {
        // data[i] = (*buffer)[i];
        db<SiFiveU_NIC>(WRN) << data[i];
    }
    db<SiFiveU_NIC>(WRN) << endl;


}





// void * Network_buffer::alloc(unsigned long int bytes) {
//     void * addr = app.alloc(bytes);

//     return addr;

// };

// // alocando os frames no dma_buffer
// void Network_buffer::alloc_frame(char frame[]) {
//     if (count_frames <= MAX_DMA_BUFFER_SIZE/FRAME_SIZE_) {
//         memcpy(dma_ptr, frame, FRAME_SIZE_);
//         dma_ptr += FRAME_SIZE_; 
//         ++count_frames;
//         db<Network_buffer>(WRN) << "allocated one frame in network_buffer" << endl;
//     }
// };

// void Network_buffer::get_dma_data(char * dma_data) {
//     // serão copiados apenas os frames alocados, por enquanto o dma_buffer inteiro
//     memcpy(dma_data, dma.log_address(), count_frames*FRAME_SIZE_ );
//     dma_ptr = dma.log_address();
//     count_frames = 0;
// };

// // Recebe os dados da rede.
// // Por enquanto, preenche todo o dma buffer, seta o ponteiro dma para o final do buffer 
// // e seta count_frames para o valor máximo
// void Network_buffer::set_dma_data(char * dma_data, int amnt_frames) {
//     unsigned int bytes = amnt_frames*FRAME_SIZE_;
//     memcpy(dma.log_address(), dma_data, bytes);
//     dma_ptr = dma.log_address()+bytes;
//     count_frames = amnt_frames;
// };

__END_UTIL
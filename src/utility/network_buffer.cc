#include <utility/network_buffer.h>

__BEGIN_UTIL

Network_buffer* Network_buffer::net_buffer;

Network_buffer::Network_buffer() {
    
    thread = new Thread(&Network_buffer::copy_for_upper_layer);
    sem = new Semaphore(1);
    
    

}

int Network_buffer::copy_for_upper_layer() {

    while (true) {

        // Bloqueia a execução da thread até que um paconte chegue
        // TODO: Não consegui usar o mutex
        net_buffer->sem->p();

        // Faz a copia do buffer rx para data
        // char  data[FRAME_SIZE];
        // net_buffer->buf->get_data_frame(data);

        db<SiFiveU_NIC>(WRN) << "Network buffer update: "<< endl;
        for (int i = 0; i < 1500; i++) {
            db<SiFiveU_NIC>(WRN) << net_buffer->data[i];
        }
        db<SiFiveU_NIC>(WRN) << endl;


    }
    return 0;
}

void Network_buffer::init() {
    // char buff[64*1600];
    net_buffer = new (SYSTEM) Network_buffer();

}

void Network_buffer::update(Data_Observed<CT_Buffer, void> *obs, CT_Buffer* buffer)
{
    net_buffer->buf->get_data_frame(net_buffer->data);
    net_buffer->sem->v();

}

__END_UTIL
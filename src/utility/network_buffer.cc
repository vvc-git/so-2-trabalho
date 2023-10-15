#include <utility/network_buffer.h>

__BEGIN_UTIL

Network_buffer* Network_buffer::net_buffer;

Network_buffer::Network_buffer() {
    
    // thread = new (SYSTEM) Thread(&copy_for_upper_layer);
    sem = new Semaphore(0);
    buf = new CT_Buffer(FRAME_SIZE*2);
    // thread->join();
    // 
    // new (SYSTEM) Thread(Thread::Configuration(Thread::READY, Thread::HIGH), &Network_buffer::copy_for_upper_layer);
    

}

int Network_buffer::copy_for_upper_layer() {

    db<SiFiveU_NIC>(WRN) << "Thread no copy" << endl;

    while (true) {

        db<SiFiveU_NIC>(WRN) << "Thread no while" << endl;

        // Bloqueia a execução da thread até que um paconte chegue
        // TODO: Não consegui usar o mutex
        net_buffer->sem->p();

        // Faz a copia do buffer rx para data
        char  data[FRAME_SIZE];
        net_buffer->buf->get_data_frame(data);

        db<SiFiveU_NIC>(WRN) << "Network buffer update: "<< endl;
        for (int i = 0; i < 1500; i++) {
            db<SiFiveU_NIC>(WRN) << data[i];
        }
        db<SiFiveU_NIC>(WRN) << endl;


    }
    return 0;
}

void Network_buffer::init() {
    // char buff[64*1600];
    net_buffer = new (SYSTEM) Network_buffer();
    // new (SYSTEM) Thread(Thread::Configuration(Thread::READY, Thread::LOW), &copy_for_upper_layer);

}

void Network_buffer::update(Data_Observed<CT_Buffer, void> *obs, CT_Buffer* buffer)
{
    // net_buffer->buf->get_data_frame(buffer->log_address());

    // TODO: DADO buffer do rx P/ buffer do network buffer
    net_buffer->sem->v();

}

__END_UTIL
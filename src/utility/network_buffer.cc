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

void Network_buffer::configure_tx_rx() {


    // TX Alocando memoria para os buffers tx de descritores e dados
    tx_desc_buffer = new CT_Buffer(DESC_SIZE * SLOTS_BUFFER);
    tx_data_buffer = new CT_Buffer(FRAME_SIZE * SLOTS_BUFFER);

    // RX Alocando memoria para os buffers rx de descritores e dados
    rx_desc_buffer = new CT_Buffer(DESC_SIZE * SLOTS_BUFFER);
    rx_data_buffer = new CT_Buffer(FRAME_SIZE * SLOTS_BUFFER);

    // Pegando endereço físico dos buffers para NIC
    // TX
    tx_desc_phy = tx_desc_buffer->phy_address();
    tx_data_phy = tx_data_buffer->phy_address();

    // RX
    rx_desc_phy = rx_desc_buffer->phy_address();
    rx_data_phy = rx_data_buffer->phy_address();

    // Pegando endereço lógico dos buffers para CPU
    log_init_tx_desc = tx_desc_buffer->log_address();
    log_init_tx_data = tx_data_buffer->log_address();

    // Pegando endereço lógico dos buffers para CPU
    log_init_rx_desc = rx_desc_buffer->log_address();
    log_init_rx_data = rx_data_buffer->log_address();

    Phy_Addr addr_desc;
    Phy_Addr addr_data;


    db<SiFiveU_NIC>(WRN) << "Endereços utilizados para o rx de dados (Construtor)"<< endl;
    // setting RX buffers
    for (unsigned int i = 0; i < SLOTS_BUFFER; i++)
    {
        addr_desc = rx_desc_phy + (i * DESC_SIZE);
        addr_data = rx_data_phy + (i * FRAME_SIZE);

        // Configure Buffer Descriptors, p. 1061
        // 3. Mark all entries in this list as owned by controller. Set bit [0] of word [0] of each buffer descriptor to 0.
        // 4. Mark the last descriptor in the buffer descriptor list with the wrap bit, (bit [1] in word [0]) set.
        // 5. Fill the addresses of the allocated buffers in the buffer descriptors (bits [31-2], Word [0])
        Cadence_GEM::Desc * rx_desc = addr_desc;

        rx_desc->set_rx_address(addr_data);

        // rx_desc->address = addr_data & RX_WORD0_2_LSB; // Os 2 últimos bits da palavra 0 estao sendo zerados
        // db<SiFiveU_NIC>(WRN) << "Endereço:  "<< i << " " << hex << rx_desc->address << endl;

        // Setando o bit WRP no último descritor
        if (i == (SLOTS_BUFFER - 1))  rx_desc->set_rx_wrap(); // rx_desc->address = rx_desc->address | RX_WORD0_LSB_WRP;

        rx_desc->reset_rx_control();
    }

    // setting TX buffers
    for (unsigned int i = 0; i < SLOTS_BUFFER; i++)
    {
        addr_desc = tx_desc_phy + (i * DESC_SIZE);
        addr_data = tx_data_phy + (i * FRAME_SIZE);

        // Configure BUffer Descriptors, p. 1062
        // 2. Mark all entries in this list as owned by the controller. Set bit [31] of word [1] to 0.  (TX_WORD1_OWN_CONTROLLER)
        // 3. Mark the last descriptor in the list with the wrap bit. Set bit [30] in word [1] to 1.
        // 4. Write the base address of transmit buffer descriptor list to Controller registers gem.transmit_q{ , 1}_ptr.
        Cadence_GEM::Desc *tx_desc = addr_desc;
        // tx_desc->address = addr_data;
        // tx_desc->control = 0;
        // tx_desc->control = TX_WORD1_OWN_CPU | tx_desc->control;
        tx_desc->set_tx_address(addr_desc);
        tx_desc->set_tx_control();

        // Setando o bit WRP no último descritor (item 3)
        if (i == (SLOTS_BUFFER - 1)) tx_desc->set_tx_wrap();//tx_desc->control = TX_WORD1_WRP_BIT | tx_desc->control;

    }

    // Configure Buffer Descriptor, p.1061
    // 6. Write the base address of this buffer descriptor list to the gem.receive_q{ , 1}_ptr
    // registers.
    // set_reg(RECEIVE_Q_PTR, rx_desc_phy);
    Cadence_GEM::set_receiver_ptr(rx_desc_phy);


    // Configure Buffer Descriptor, p.1062
    //4. Write the base address of transmit buffer descriptor list to Controller registers gem.transmit_q{ , 1}_ptr..
    // set_reg(TRANSMIT_Q_PTR, tx_desc_phy);
    Cadence_GEM::set_transmiter_ptr(tx_desc_phy);
}

int Network_buffer::copy_for_upper_layer() {

    while (true) {

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

void Network_buffer::update(Observed *obs)
{
    // net_buffer->buf->get_data_frame(buffer->log_address());

    // TODO: DADO buffer do rx P/ buffer do network buffer
    net_buffer->sem->v();

}

Cadence_GEM::Desc * Network_buffer::get_free_tx_desc() {

    db<Network_buffer>(WRN) << "antes do get  "  << endl;

    Cadence_GEM::Desc *tx_desc = nullptr;
    // Varrer descriptors de tx procurando buffer livre
    for (; last_desc_idx < SLOTS_BUFFER; )
    {
        tx_desc = tx_desc_phy + (last_desc_idx * DESC_SIZE);
        last_desc_idx = (last_desc_idx + 1) % SLOTS_BUFFER;
        
        db<Network_buffer>(WRN) << "control " << hex << tx_desc->control << endl;
        if (tx_desc->control >> 31) {
           
            db<Network_buffer>(WRN) << "Tx_desc no Network  " << hex << tx_desc->address << endl;
            break;
        }
    }

    return tx_desc;

}

__END_UTIL
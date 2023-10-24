#include <machine/riscv/network_buffer.h>

__BEGIN_SYS

Network_buffer* Network_buffer::net_buffer;

void Network_buffer::init() {
    net_buffer = new (SYSTEM) Network_buffer();

}

Network_buffer::Network_buffer() {
    
    sem = new Semaphore(0);
    buf = new CT_Buffer(FRAME_SIZE*64);
    char data[FRAME_SIZE*64*10];
    dt =  new DT_Buffer(data, FRAME_SIZE*64*10);
    

}

void Network_buffer::IP_send(char* data, unsigned int data_size) {
    db<Network_buffer>(WRN) << "Network_buffer::IP_send inicio"<< endl;
    // Fragmentar pacotes de 1500 bytes
    // 48 bytes - Header
    // 1452 - Dados
    
    // Irá avançar sobre data para fazer o memcpy
    char* data_pointer;

    unsigned int frag_size = 1452;
    unsigned int iter = data_size/frag_size;
    // unsigned int last = data_size%frag_size;

    for (unsigned int i = 0; i < iter; i++) {
        // Construindo Fragmento
        Datagram_Fragment payload;

        // Construindo Header
        payload.header = Datagram_Header();

        // Setar Total_Length, Identification, Flags e Offset
        payload.header.Total_Length = data_size;
        payload.header.Identification = 0x1234; // É aleatório?
        unsigned int offset = i*frag_size;
        payload.header.Flags_Offset = offset | payload.header.MORE_FRAGS;

        // Setando o ponteiro para o endereco especifico em data
        data_pointer = data + i*frag_size;

        // Copiando os dados para o payload que sera enviado
        memcpy(payload.data, data_pointer, frag_size);

        // print para testar se o dado foi copiado
        for (unsigned int i=0; i<frag_size; i++) {
             db<Network_buffer>(WRN) << payload.data[i];
        }
        db<Network_buffer>(WRN) << endl;

        // Print para verificar o header
        db<Network_buffer>(WRN) << "Total_Length: " << payload.header.Total_Length << endl;
        db<Network_buffer>(WRN) << "Identification: " << hex << payload.header.Identification << endl;
        db<Network_buffer>(WRN) << "Flags_Offset: " << hex << payload.header.Flags_Offset << endl;
    }
    

    
    db<Network_buffer>(WRN) << "Network_buffer::IP_send fim"<< endl;
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

    Phy_Addr desc;
    Phy_Addr data;

    // setting RX buffers
    for (unsigned int i = 0; i < SLOTS_BUFFER; i++)
    {
        desc = rx_desc_phy + (i * DESC_SIZE);
        data = rx_data_phy + (i * FRAME_SIZE);

        // Configure Buffer Descriptors, p. 1061
        // 3. Mark all entries in this list as owned by controller. Set bit [0] of word [0] of each buffer descriptor to 0.
        // 4. Mark the last descriptor in the buffer descriptor list with the wrap bit, (bit [1] in word [0]) set.
        // 5. Fill the addresses of the allocated buffers in the buffer descriptors (bits [31-2], Word [0])
        Desc * rx_desc = desc;

        // Os 2 últimos bits da palavra 0 estao sendo zerados
        rx_desc->set_rx_address(data);

        // Setando o bit WRP no último descritor
        if (i == (SLOTS_BUFFER - 1))  rx_desc->set_rx_wrap();

        rx_desc->reset_rx_control();
    }

    // setting TX buffers
    for (unsigned int i = 0; i < SLOTS_BUFFER; i++)
    {
        desc = tx_desc_phy + (i * DESC_SIZE);
        data = tx_data_phy + (i * FRAME_SIZE);

        // Configure BUffer Descriptors, p. 1062
        // 2. Mark all entries in this list as owned by the controller. Set bit [31] of word [1] to 0.  (TX_WORD1_OWN_CONTROLLER)
        // 3. Mark the last descriptor in the list with the wrap bit. Set bit [30] in word [1] to 1.
        // 4. Write the base address of transmit buffer descriptor list to Controller registers gem.transmit_q{ , 1}_ptr.
        Cadence_GEM::Desc *tx_desc = desc;
        tx_desc->set_tx_address(data);
        tx_desc->set_tx_control();

        // Setando o bit WRP no último descritor (item 3)
        if (i == (SLOTS_BUFFER - 1)) tx_desc->set_tx_wrap();

    }

    // Configure Buffer Descriptor, p.1061
    // 6. Write the base address of this buffer descriptor list to the gem.receive_q{ , 1}_ptr registers.
    Cadence_GEM::set_receiver_ptr(rx_desc_phy);


    // Configure Buffer Descriptor, p.1062
    //4. Write the base address of transmit buffer descriptor list to Controller registers gem.transmit_q{ , 1}_ptr..
    Cadence_GEM::set_transmiter_ptr(tx_desc_phy);
}

int Network_buffer::copy() {

    Desc *desc = net_buffer->rx_desc_phy;
    unsigned int idx = 0;
    Reg32 data = 0;
    
    while (true)
    {
        net_buffer->sem->p();
        
        for (int i = 0; !(desc->is_cpu_owned()); i=(i+1)%net_buffer->SLOTS_BUFFER) {
            
            desc = net_buffer->rx_desc_phy + i * net_buffer->DESC_SIZE;
            idx = i;
        }


        // Setando novamente desc->address, para recebimento de novos frames para eventuais casos de sobreescrita
        desc = net_buffer->rx_desc_phy + idx * net_buffer->DESC_SIZE;

        // Definindo endereço do buffer de dados a partir do índice salvo
        data = net_buffer->rx_data_phy + idx * FRAME_SIZE; 
        
        // Setando o no rx de dados
        desc->address = data;

        // Colocando o valor de RX data (addr) para o CT_buffer alocado
        Network_buffer::net_buffer->buf->save_data_frame(reinterpret_cast<char*>(desc->address));

        
        // Setando os 2 ultimos bits da word[0]
        // (O wrap bit caso seja necessário)
        desc->set_rx_own_wrap(idx == ( net_buffer->SLOTS_BUFFER - 1));

        // Faz a copia do buffer rx para data
        char  payload[FRAME_SIZE];
        net_buffer->buf->get_data_frame(payload);

        db<SiFiveU_NIC>(WRN) << "Network buffer update: "<< endl;
        for (int i = 0; i < 1500; i++) {
            db<SiFiveU_NIC>(WRN) << payload[i];
        }
        db<SiFiveU_NIC>(WRN) << endl;
    }
    
    return 0;
}

void Network_buffer::update(Observed *obs)
{

    net_buffer->sem->v();

}

Cadence_GEM::Desc * Network_buffer::get_free_tx_desc() {

    Cadence_GEM::Desc *tx_desc = nullptr;
    // Varrer descriptors de tx procurando buffer livre
    for (; last_desc_idx < SLOTS_BUFFER; )
    {
        tx_desc = tx_desc_phy + (last_desc_idx * DESC_SIZE);
        last_desc_idx = (last_desc_idx + 1) % SLOTS_BUFFER;
        
        // Encontrou um slot livre
        if (tx_desc->control >> 31) {
            break;
        }
    }

    return tx_desc;

}

__END_SYS
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
        db<Network_buffer>(TRC) << "Thread: " << endl;

        for (int i = 0; !(desc->is_cpu_owned()); i=(i+1)%net_buffer->SLOTS_BUFFER) {
            
            desc = net_buffer->rx_desc_phy + i * net_buffer->DESC_SIZE;
            idx = i;
        }


        // Setando novamente desc->address, para recebimento de novos frames para eventuais casos de sobreescrita
        desc = net_buffer->rx_desc_phy + idx * net_buffer->DESC_SIZE;

        // Definindo endereço do buffer de dados a partir do índice salvo
        data = net_buffer->rx_data_phy + idx * FRAME_SIZE; 
        
        // Setando o address no rx_desc
        desc->address = data;

        // Protocolo do frame que chegou
        short int protocol = ntohs(*(reinterpret_cast<short int *>(data) + 6));

        // Frame size
        unsigned int frame_size = (desc->control & Cadence_GEM::GET_FRAME_LENGTH); //- sizeof(Ethernet::CRC32) - sizeof(Ethernet::Header);
        db<Network_buffer>(TRC) << "Header: " <<  sizeof(Ethernet::Header)  << endl;
        db<Network_buffer>(TRC) << "CRC: " <<  sizeof(Ethernet::CRC)  << endl;
        db<Network_buffer>(TRC) << "Frame size: " << frame_size << endl;


        if  (protocol == 0x0806) {
            db<Network_buffer>(TRC) << "copy(): Pacote ARP"<< endl;
            
            unsigned int arp_hsize = 28; // arp header size
            ARP_Packet* packet = new ARP_Packet();

            // Copiando os dados do buffer RX
            memcpy(packet, reinterpret_cast<ARP_Packet*>(desc->address + sizeof(Ethernet::Header)), arp_hsize);
            
            // Liberando a o buffer RX para a NIC, 
            // Setando os 2 ultimos bits da word[0] (O wrap bit caso seja necessário)
            desc->set_rx_own_wrap(idx == ( net_buffer->SLOTS_BUFFER - 1));

            // ARP_Manager trata o pacote
            ARP_Manager::_arp_mng->arp_receive(packet);

        }
        
        if (protocol == 0x0800) {
            
            db<Network_buffer>(TRC) << "copy(): Pacote IP "<< endl;

            // Se MAC é igual e IP é diferente do pacote, é preciso retransmitir
            Frame * frame = (reinterpret_cast<Frame*>(desc->address));

            if (frame->dst() != SiFiveU_NIC::_device->address) {
                db<Network_buffer>(TRC) << "copy(): Não é meu MAC"<< frame->dst() << endl;
                continue;
            }

            // Verifica se eu sou o ip
            // Caso negativo, eh preciso retransmitir
            Fragment::Header * header = (reinterpret_cast<Fragment::Header*>(desc->address + 14));

            if (header->Protocol == 1) {
                db<Network_buffer>(WRN) << "Echo Message " << endl;

                // Faz a copia do buffer rx para data
                char  payload[frame_size-4];
                memcpy(payload, reinterpret_cast<void*>(desc->address), frame_size-4);
                ICMP_Manager::_icmp_mng->receive((void *)(payload));
            
            } else {
            
                bool retransmit = false;
                for (int i = 0; (!retransmit) && i < 4; i++) {
                    if ((ARP_Manager::_arp_mng->IP_ADDR[i] != header->DST_ADDR[i]) && (header->DST_ADDR[i] !=  IP_Manager::_ip_mng->localhost->object()->destination[i])) {
                        retransmit = true;
                    }

                }      
                
                // Faz a copia do buffer rx para data
                char  payload[frame_size - 4];
                // net_buffer->buf->get_data_frame(payload);
                memcpy(payload, reinterpret_cast<void*>(desc->address), frame_size - 4);
                
                // Setando os 2 ultimos bits da word[0]
                // (O wrap bit caso seja necessário)
                desc->set_rx_own_wrap(idx == ( net_buffer->SLOTS_BUFFER - 1));

                db<Network_buffer>(TRC) << "Retransmit " << retransmit << endl;
                IP_Manager::_ip_mng->receive((void *)(payload + 14), retransmit);
            }
        }
        
    }
    
    return 0;
}

void Network_buffer::update(Observed *obs)
{
    db<Network_buffer>(TRC) << "Network_buffer::update()" << endl;
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

// Aqui
__END_SYS
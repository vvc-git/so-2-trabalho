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
    // 20 bytes - Header
    // 1480 - Dados

    // Setando MAC de destino
    NIC<Ethernet>::Address dst;
    dst[0] = 0x00;
    dst[1] = 0x00;
    dst[2] = 0x00;
    dst[3] = 0x00;
    dst[4] = 0x00;
    dst[5] = 0x02;
    
    // Irá avançar sobre data para fazer o memcpy
    char* data_pointer;

    unsigned int header_size = 20;
    unsigned int mtu = 1500;
    unsigned int frag_data_size = mtu - header_size;
    unsigned int iter = (data_size/frag_data_size) + 1;
    unsigned int last_size = data_size%frag_data_size;

    unsigned int id = 0x1230 + id_send;
    id_send++;

    Datagram_Header dt_header = Datagram_Header();
    Reg8 version = 4 << 4;
    Reg8 IHL = (header_size/4);
    dt_header.Version_IHL = (version | IHL);
    dt_header.Type_Service = 0;
    dt_header.Total_Length = CPU_Common::htons(data_size + header_size);
    dt_header.Identification = CPU_Common::htons(id);
    dt_header.TTL = 60;
    dt_header.Protocol = 253;
    dt_header.Header_Checksum = 0;
    dt_header.SRC_ADDR = 0x0100007F; // 127.0.0.1
    dt_header.DST_ADDR = 0x0200007F; // 127.0.0.2

    db<Network_buffer>(WRN) << "Iter " << iter << endl;
    for (unsigned int i = 0; i < iter; i++) {
        if (i == iter - 1 && !last_size) break; // fragmentação já realizada

        db<Network_buffer>(WRN) << i << "° fragmento " << endl;

        // Construindo Fragmento
        Datagram_Fragment fragment;

        // Construindo Header
        fragment.header = dt_header;
        
        // Flags e Offset
        unsigned int offset = i*mtu;
        Reg16 flag_off = 0;
        if ((i == iter - 1) || (i == iter - 2 && last_size == 0)) {
            flag_off = (offset & LAST_FRAG);
        } else {
            flag_off = (offset | MORE_FRAGS);
        }
        fragment.header.Flags_Offset = CPU_Common::htons(flag_off);

        // Setando o ponteiro para o endereco especifico em data
        data_pointer = data + i*frag_data_size;

        // Copiando os dados para o fragment que sera enviado
        unsigned int size = (i == iter - 1) ? last_size : frag_data_size;
        memcpy(fragment.data, data_pointer, size);

        // Preenchimento dummy do último fragmento
        if (i == iter - 1) {
            data_pointer = fragment.data + last_size;   
            memset(data_pointer, '9', frag_data_size - last_size);
        }

        // TODO: Desacoplar o send do IP e o send da NIC (ethernet)
        SiFiveU_NIC::_device->send(dst, (void*) &fragment, mtu);
        if (!i) {
            // SiFiveU_NIC::_device->send(dst, (void*) &fragment, mtu);
        }

        // print para testar se o dado foi copiado
        for (unsigned int i=0; i<frag_data_size; i++) {
             db<Network_buffer>(WRN) << fragment.data[i];
        }
        db<Network_buffer>(WRN) << endl;

        // Print para verificar o header
        db<Network_buffer>(WRN) << "Total_Length: " << CPU_Common::htons(fragment.header.Total_Length) << endl;
        db<Network_buffer>(WRN) << "Identification: " << hex << CPU_Common::htons(fragment.header.Identification) << endl;
        db<Network_buffer>(WRN) << "Flags_Offset: " << hex << CPU_Common::htons(fragment.header.Flags_Offset) << endl;
    }
    
    db<Network_buffer>(WRN) << "Network_buffer::IP_send fim"<< endl;
}


void Network_buffer::IP_receive(void* data) {
    db<Network_buffer>(WRN) << "Network_buffer::IP_receive inicio"<< endl;
    // Fragmentar pacotes de 1500 bytes
    // 48 bytes - Header
    // 1452 - Dados

    // Setando MAC de destino
    // NIC<Ethernet>::Address dst;
    // dst[0] = 0x00;
    // dst[1] = 0x00;
    // dst[2] = 0x00;
    // dst[3] = 0x00;
    // dst[4] = 0x00;
    // dst[5] = 0x02;

    Datagram_Fragment * fragment = reinterpret_cast<Datagram_Fragment*>(data);
    db<Network_buffer>(WRN) << "Teste receive " << endl;
    db<Network_buffer>(WRN) << "Total_Length: " << fragment->header.Total_Length << endl;
    db<Network_buffer>(WRN) << "Identification: " << hex << fragment->header.Identification << endl;
    db<Network_buffer>(WRN) << "Flags_Offset: " << hex << fragment->header.Flags_Offset << endl;
    db<Network_buffer>(WRN) << "Passou " << endl;

    // Offset   / Flag 
    unsigned int offset = fragment->header.Flags_Offset & GET_OFFSET;
    unsigned int length = fragment->header.Total_Length;
    // unsigned int identification = fragment->header.Identification;
    db<Network_buffer>(WRN) << "Offset: " << offset << endl;

    if (!identification) {
        db<SiFiveU_NIC>(WRN) << "Primeiro frame"<<endl;
        teste = dt->alloc(length);
        identification = 0x1234;
        counter = length;
    
    } else {
        db<Network_buffer>(WRN) << "Outros frames"<<endl;
        // for (int i = 0; i < 1452; i++) {
        //     db<Network_buffer>(WRN) << fragment->data[i];
        // }
        // db<Network_buffer>(WRN) << endl;
        // db<Network_buffer>(WRN) << "teste " << hex << teste <<endl;
        // char * payload = (char*) teste; 
        // db<Network_buffer>(WRN) << "payload " << hex << payload <<endl;
        
        // payload = payload + (offset - 48*(offset/1500));
        // db<Network_buffer>(WRN) << "payload " << hex << payload <<endl;

        // memcpy(payload, fragment->data, 1452);

        // db<Network_buffer>(WRN) << "counter 1"<< counter <<endl;
        // if (fragment->header.Flags_Offset | fragment->header.MORE_FRAGS) {
        //     counter -= fragment->header.Total_Length - offset;
        // } else {
        //     counter -= 1500;
        // }

        // db<Network_buffer>(WRN) << "counter 2"<< counter <<endl;
        // if (!counter) {
        //     db<Network_buffer>(WRN) << "Funcionou a desfragmentação "<< endl;
        //     char * payload = (char*) teste;
        //     db<Network_buffer>(WRN) << "Funcionou a conversão "<< endl;
        //     for (int i = 0; i < 1500; i++) {
        //         db<Network_buffer>(WRN) << payload[i];
        //     }
        //     db<Network_buffer>(WRN) << endl;

        //     dt->free(teste, length);

        // }

    }



    // identification
    // lengt

    // Irá avançar sobre data para fazer o memcpy
    // char* data_pointer;

    // unsigned int mtu = 1452;
    // unsigned int iter = data_size/mtu;
    // unsigned int last_size = data_size%mtu;

    // for (unsigned int i = 0; i < iter; i++) {
    //     // Construindo Fragmento
    //     Datagram_Fragment fragment;

    //     // Construindo Header
    //     fragment.header = Datagram_Header();

    //     // Setar Total_Length, Identification, Flags e Offset
    //     fragment.header.Total_Length = data_size;
    //     fragment.header.Identification = 0x1234; // É aleatório?
    //     unsigned int offset = i*mtu;
    //     fragment.header.Flags_Offset = offset | fragment.header.MORE_FRAGS;

    //     // Setando o ponteiro para o endereco especifico em data
    //     data_pointer = data + i*mtu;

    //     // Copiando os dados para o fragment que sera enviado
    //     memcpy(fragment.data, data_pointer, mtu);

    //     // TODO: Desacoplar o send do IP e o send da NIC (ethernet)
    //     SiFiveU_NIC::_device->send(dst, (void*) &fragment, 1500);

    //     // print para testar se o dado foi copiado
    //     // for (unsigned int i=0; i<mtu; i++) {
    //     //      db<Network_buffer>(WRN) << fragment.data[i];
    //     // }
    //     // db<Network_buffer>(WRN) << endl;

    //     // Print para verificar o header
    //     db<Network_buffer>(WRN) << "Total_Length: " << fragment.header.Total_Length << endl;
    //     db<Network_buffer>(WRN) << "Identification: " << hex << fragment.header.Identification << endl;
    //     db<Network_buffer>(WRN) << "Flags_Offset: " << hex << fragment.header.Flags_Offset << endl;
    // }

    // // Último Fragmento
    // Datagram_Fragment fragment;

    // // Construindo Header
    // fragment.header = Datagram_Header();

    // // Setar Total_Length, Identification, Flags e Offset
    // fragment.header.Total_Length = data_size;
    // fragment.header.Identification = 0x1234; // É aleatório?
    // unsigned int offset = iter*mtu;
    // fragment.header.Flags_Offset = offset & fragment.header.LAST_FRAG;

    // // Setando o ponteiro para o endereco especifico em data
    // data_pointer = data + iter*mtu;

    // // Copiando os dados para o fragment que sera enviado
    // memcpy(fragment.data, data_pointer, last_size);

    // // Preenchimento dummy
    // data_pointer = fragment.data + last_size;   
    // memset(data_pointer, '9', mtu - last_size);

    // SiFiveU_NIC::_device->send(dst, (void*) &fragment, 1500);
    // // print para testar se o dado foi copiado
    // // for (unsigned int i=0; i<mtu; i++) {
    // //         db<Network_buffer>(WRN) << fragment.data[i];
    // // }
    // // db<Network_buffer>(WRN) << endl;

    // // Print para verificar o header
    // db<Network_buffer>(WRN) << "Total_Length: " << fragment.header.Total_Length << endl;
    // db<Network_buffer>(WRN) << "Identification: " << hex << fragment.header.Identification << endl;
    // db<Network_buffer>(WRN) << "Flags_Offset: " << hex << fragment.header.Flags_Offset << endl;
    
    
    // db<Network_buffer>(WRN) << "Network_buffer::IP_send fim"<< endl;
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

        net_buffer->IP_receive((void *)(payload+14));

        // db<SiFiveU_NIC>(WRN) << "Network buffer update: "<< endl;
        // for (int i = 0; i < 1500; i++) {
        //     db<SiFiveU_NIC>(WRN) << payload[i];
        // }
        // db<SiFiveU_NIC>(WRN) << endl;
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
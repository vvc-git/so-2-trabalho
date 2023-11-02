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
    unsigned int nic_mtu = 1500;
    unsigned int frag_data_size = nic_mtu - header_size;
    unsigned int iter = (data_size/frag_data_size) + 1;
    unsigned int last_size = data_size  % frag_data_size;

    unsigned int id = 0x1230 + id_send;
    id_send++;

    Datagram_Header dt_header = Datagram_Header();
    Reg8 version = 4 << 4;
    Reg8 IHL = (header_size/4);
    dt_header.Version_IHL = (version | IHL);
    dt_header.Type_Service = 0;
    dt_header.Total_Length = CPU_Common::htons(nic_mtu);
    dt_header.Identification = CPU_Common::htons(id);
    dt_header.TTL = 64;
    dt_header.Protocol = 4;
    dt_header.Header_Checksum = 0;
    dt_header.SRC_ADDR[0] = 127; // 127.0.0.1
    dt_header.SRC_ADDR[1] = 0;
    dt_header.SRC_ADDR[2] = 0;
    dt_header.SRC_ADDR[3] = 1;
    dt_header.DST_ADDR[0] = 127; // 127.0.0.2       
    dt_header.DST_ADDR[1] = 0;
    dt_header.DST_ADDR[2] = 0;
    dt_header.DST_ADDR[3] = 2;
    

    db<Network_buffer>(WRN) << "Identification: " << hex << CPU_Common::htons(dt_header.Identification) << endl;
    for (unsigned int i = 0; i < iter; i++) {
        if (i == iter - 1 && !last_size) break; // fragmentação já realizada
        db<Network_buffer>(WRN) << i << "° fragmento " << endl;

        // Construindo Fragmento    
        Datagram_Fragment fragment;

        // Construindo Header
        fragment.header = dt_header;
        
        // Flags e Offset
        unsigned int offset = (i*frag_data_size)/8;
        Reg16 flag_off = 0;
        if ((i == iter - 1) || (i == iter - 2 && last_size == 0)) {
            // Último fragmento
            flag_off = (offset & LAST_FRAG);
        } else {
            // Existem mais fragmentos
            flag_off = (offset | MORE_FRAGS);
        }
        fragment.header.Flags_Offset = CPU_Common::htons(flag_off);

        // Setando o ponteiro para o endereco especifico em data
        data_pointer = data + i*frag_data_size;

        // Copiando os dados para o fragment que sera enviado
        unsigned int size = (i == iter - 1) ? last_size : frag_data_size;
        memcpy(fragment.data, data_pointer, size);

        // Preenchimento do último fragmento e Seta total length
        if (i == iter - 1) {
            db<Network_buffer>(WRN) << "Last_siz " << last_size << endl;
            fragment.header.Total_Length = CPU_Common::htons(last_size + header_size);
            data_pointer = fragment.data + last_size;   
            memset(data_pointer, '9', frag_data_size - last_size);
        }

        db<Network_buffer>(WRN) << "data" << reinterpret_cast<char*>(data) << endl;
        SiFiveU_NIC::_device->send(dst, (void*) &fragment, nic_mtu);
        Delay (1000000);
        

        // Print para verificar o header
        db<Network_buffer>(TRC) << "Total_Length original " << CPU_Common::ntohs(fragment.header.Total_Length)<< endl;
        db<Network_buffer>(TRC) << "Total_Length sem o size: " << (CPU_Common::ntohs(fragment.header.Total_Length) * 8) - header_size << endl;
        db<Network_buffer>(TRC) << "Identification: " << hex << CPU_Common::htons(fragment.header.Identification) << endl;
        db<Network_buffer>(WRN) << "Offset: " << (CPU_Common::htons(fragment.header.Flags_Offset) & GET_OFFSET)*8 << endl;
    }
    
    db<Network_buffer>(WRN) << "---------------------"<< endl;
    db<Network_buffer>(TRC) << "Network_buffer::IP_send fim"<< endl;
}


void Network_buffer::IP_receive(void* data) {
    db<Network_buffer>(WRN) << "---------------------"<< endl;
    db<Network_buffer>(WRN) << "Network_buffer::IP_receive\n"<< endl;

    char * content = new char[1500];
    memcpy(content, data, 1500);

    Datagram_Fragment * fragment = reinterpret_cast<Datagram_Fragment*>(content);
    db<Network_buffer>(TRC) << "Pointer fragment: " << hex << fragment << endl;



    if (fragment->header.Total_Length == 0) return;
    
    // Capturando os valores do fragmento
    unsigned int length = CPU_Common::ntohs(fragment->header.Total_Length) - 20;
    short unsigned int identification = CPU_Common::ntohs(fragment->header.Identification);
    short unsigned int offset = (CPU_Common::ntohs(fragment->header.Flags_Offset) & GET_OFFSET) * 8;
    short unsigned int more_frags = (CPU_Common::ntohs(fragment->header.Flags_Offset) & MORE_FRAGS);
    short unsigned int flags = ((CPU_Common::ntohs(fragment->header.Flags_Offset) & GET_FLAGS));
    
    
    // Verificação se os valores estão certos
    db<Network_buffer>(TRC) << "length: " << hex << length << endl;
    db<Network_buffer>(WRN) << "identification: " << hex << identification << endl;
    db<Network_buffer>(WRN) << "offset: " << offset << endl;
    db<Network_buffer>(TRC) << "flags: " << flags << endl;


    List::Element * e;
    for (e = dt_list->head(); e && e->object()->id != identification; e = e->next()) {}   

    if (!e) {
        db<SiFiveU_NIC>(TRC) << "Primeiro frame"<<endl;
        
        // Criando uma nova estrutura para frames do mesmo datagrama
        Simple_List<Datagram_Fragment>  * frame_list = new Simple_List<Datagram_Fragment>();
        
        INFO * datagram_info = new INFO{identification, 0, 0, frame_list};
        Element * link2 = new Element(datagram_info);
        
        // Adicionana na lista de datagramas
        dt_list->insert(link2);
        e = link2; 
        
    }

    INFO * dt_info = e->object();
    Simple_List<Datagram_Fragment> * dt_list = e->object()->frame_list; 

    Simple_List<Datagram_Fragment>::Element * link1 = new  Simple_List<Datagram_Fragment>::Element(fragment);
    dt_list->insert(link1);
    
    
    // db<Network_buffer>(WRN) << "Size frame list " << e->object()->frame_list->size() <<endl; 
    db<Network_buffer>(TRC) << "head " << dt_list->head()->object()->data << endl; 
    db<Network_buffer>(TRC) << "tail " << dt_list->tail()->object()->data << endl;  
    
    // Incrementa o contador de frames
    dt_info->num_frames++;
    db<Network_buffer>(TRC) << "Number of frames: " << dt_info->num_frames << endl;  
    
    // Verificação de ultimo frame,
    // Caso positivo, seta para o tamanho adequando
    unsigned int total_frame = 0;
    if (!more_frags) {
        
        // Salva o tamanho total do datagrama
        dt_info->total_length = offset + length;
        
        // Salva quantos frames vão ter no datagrama
        total_frame = dt_info->total_length / 1480;
        if (length < 1480) total_frame++;
        
        
        db<Network_buffer>(WRN) << "Ultimo frame" << endl;
        db<Network_buffer>(TRC) << "Total length: " << dt_info->total_length << endl;
        db<Network_buffer>(TRC) << "Total frame:  " << total_frame << endl;

    }

    // Quando todos os frames chegaram, remonta.
    if (total_frame == dt_info->num_frames) {
        
        db<Network_buffer>(WRN) << "Remontagem" << endl;
        
        // Capturando o 1° e ultimo frame 
        Simple_List<Datagram_Fragment>::Element * h = e->object()->frame_list->head();
        // Simple_List<Datagram_Fragment>::Element * t = e->object()->frame_list->tail();

        db<Network_buffer>(TRC) << "head " << dt_list->head()->object()->data <<endl;
        db<Network_buffer>(TRC) << "tail " << dt_list->tail()->object()->data <<endl;

        db<Network_buffer>(TRC) << "Pointer head " << dt_list->head()->object() <<endl;
        db<Network_buffer>(TRC) << "Pointer tail " << dt_list->tail()->object() <<endl;
        
        // Aloca um espaço na heap para o datagrama
        void * base = dt->alloc(dt_info->total_length);
        
        
        for (; h; h = h->next()) {

            // Fragment to be reasemble
            Datagram_Fragment * f = h->object();

            // Captura o offset do fragmento
            short unsigned int offset = (CPU_Common::ntohs(f->header.Flags_Offset) & GET_OFFSET) * 8;
            
            // O tamnaho daquele frame (sem o header)
            unsigned int size = CPU_Common::ntohs(f->header.Total_Length) - 20;

            // Remonta o frame na heap
            db<Network_buffer>(TRC) << "Data " << f->data <<endl;
            db<Network_buffer>(TRC) << "Size " << size << endl;
            db<Network_buffer>(TRC) << "Offset do frame " << offset <<endl;
            char * next = reinterpret_cast<char*>(base) + offset;
            memcpy(next, f->data,  size);

        }  

        db<Network_buffer>(WRN) << "Datagrama final " <<reinterpret_cast<char*>(base) << endl;

    }

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

        db<Network_buffer>(TRC) << "payload: " << hex << (void *)payload << endl;
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
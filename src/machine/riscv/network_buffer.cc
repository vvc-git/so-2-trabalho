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

void Network_buffer::IP_send(char* data, unsigned int data_size, unsigned char * dst_ip, Address * dst_mac) {
    db<Network_buffer>(TRC) << "Network_buffer::IP_send inicio"<< endl;

    // Setando MAC de destino
    Address dst = *dst_mac;
    
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

    for (int i = 0; i < 4; i++) {
        dt_header.SRC_ADDR[i] = ARP_Manager::_arp_mng->IP_ADDR[i];
        dt_header.DST_ADDR[i] = dst_ip[i];
    }

    db<Network_buffer>(TRC) << "Identification: " << hex << CPU_Common::htons(dt_header.Identification) << endl;
    for (unsigned int i = 0; i < iter; i++) {
        if (i == iter - 1 && !last_size) break; // fragmentação já realizada
        db<Network_buffer>(TRC) << i << "° fragmento " << endl;

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
            db<Network_buffer>(TRC) << "Last_siz " << last_size << endl;
            fragment.header.Total_Length = CPU_Common::htons(last_size + header_size);
            data_pointer = fragment.data + last_size;   
            memset(data_pointer, '9', frag_data_size - last_size);
        }

        SiFiveU_NIC::_device->send(dst, (void*) &fragment, nic_mtu, 0x0800);
        Delay (1000000);
        

        // Print para verificar o header
        db<Network_buffer>(TRC) << "Total_Length original " << CPU_Common::ntohs(fragment.header.Total_Length)<< endl;
        db<Network_buffer>(TRC) << "Total_Length sem o size: " << (CPU_Common::ntohs(fragment.header.Total_Length) * 8) - header_size << endl;
        db<Network_buffer>(TRC) << "Identification: " << hex << CPU_Common::htons(fragment.header.Identification) << endl;
        db<Network_buffer>(TRC) << "Offset: " << (CPU_Common::htons(fragment.header.Flags_Offset) & GET_OFFSET)*8 << endl;
    }
    
    db<Network_buffer>(TRC) << "---------------------"<< endl;
    db<Network_buffer>(TRC) << "Network_buffer::IP_send fim"<< endl;
}


void Network_buffer::IP_receive(void* data, bool retransmit) {
 
    // Cria um novo ponteiro para adicionar na lista de fragmentos que estão chegando
    char * content = new char[1500];
    memcpy(content, data, 1500);

    Datagram_Fragment * fragment = reinterpret_cast<Datagram_Fragment*>(content);

    db<Network_buffer>(TRC) << "---------------------"<< endl;
    db<Network_buffer>(TRC) << "Network_buffer::IP_receive\n"<< endl;

    
    // Capturando os valores do fragmento
    unsigned int length = CPU_Common::ntohs(fragment->header.Total_Length) - 20;
    short unsigned int identification = CPU_Common::ntohs(fragment->header.Identification);
    short unsigned int offset = (CPU_Common::ntohs(fragment->header.Flags_Offset) & GET_OFFSET) * 8;
    short unsigned int more_frags = (CPU_Common::ntohs(fragment->header.Flags_Offset) & MORE_FRAGS);
    short unsigned int flags = ((CPU_Common::ntohs(fragment->header.Flags_Offset) & GET_FLAGS));
    
    
    // Verificação se os valores estão certos
    db<Network_buffer>(TRC) << "length: " << hex << length << endl;
    db<Network_buffer>(TRC) << "identification: " << hex << identification << endl;
    db<Network_buffer>(TRC) << "offset: " << offset << endl;
    db<Network_buffer>(TRC) << "flags: " << flags << endl;


    List::Element * e;
    for (e = dt_list->head(); e && e->object()->id != identification; e = e->next()) {}   

    // Verifica se já temos informações do fragmento que chegou
    // Se não tiver, inicia um datagrama novo
    if (!e) {
        db<SiFiveU_NIC>(TRC) << "Primeiro frame"<<endl;
        
        // Criando uma nova estrutura para frames do mesmo datagrama
        Simple_List<Datagram_Fragment>*  fragments = new Simple_List<Datagram_Fragment>();
        
        INFO * datagram_info = new INFO{identification, 0, 0, fragments};
        Element * link2 = new Element(datagram_info);
        
        // Adicionana na lista de datagramas
        dt_list->insert(link2);
        e = link2; 
        
    }


    // Informações do datagrama em que o frgamentos que chegou se encontra
    INFO * dt_info = e->object();

    // Lista de fragmentos  de um mesmo datagram
    Simple_List<Datagram_Fragment> * dt_list = e->object()->fragments; 

    // Inserindo na lista de fragmentos
    Simple_List<Datagram_Fragment>::Element * link1 = new  Simple_List<Datagram_Fragment>::Element(fragment);
    dt_list->insert(link1);
    
    // Incrementa o contador de fragmentos
    dt_info->num_fragments++;
    db<Network_buffer>(TRC) << "Numero de frames " << dt_info->num_fragments << endl;  
    
    // Verificação se é o ultimo fragmento,
    // Caso positivo, seta para o tamanho adequando
    unsigned int total_frame = 0;
    if (!more_frags) {
        
        // Salva o tamanho total do datagrama
        dt_info->total_length = offset + length;
        
        // Salva quantos frames vão ter no datagrama
        total_frame = dt_info->total_length / 1480;
        if (length < 1480) total_frame++;
        
        db<Network_buffer>(TRC) << "Total length: " << dt_info->total_length << endl;
        db<Network_buffer>(TRC) << "Total frame:  " << total_frame << endl;

    }

    // Quando todos os framentos chegaram, remonta.
    if (total_frame == dt_info->num_fragments) {
        
        db<Network_buffer>(TRC) << "Remontagem" << endl;
        
        // Capturando o 1° fragmento 
        Simple_List<Datagram_Fragment>::Element * h = e->object()->fragments->head();
        db<Network_buffer>(TRC) << "head " << dt_list->head()->object()->data <<endl;
        
        // Aloca um espaço na heap para o datagrama
        void * base = dt->alloc(dt_info->total_length);
        
        // Percorre a lista de fragmentos para a remontagem
        for (; h; h = h->next()) {

            // Fragmento que será colocado no datagrama
            Datagram_Fragment * f = h->object();

            // Offset do fragmento
            short unsigned int offset = (CPU_Common::ntohs(f->header.Flags_Offset) & GET_OFFSET) * 8;
            
            // Tamanho do fragmento
            unsigned int size = CPU_Common::ntohs(f->header.Total_Length) - 20;

            db<Network_buffer>(TRC) << "Size " << size << endl;
            db<Network_buffer>(TRC) << "Offset do frame " << offset <<endl;
            db<Network_buffer>(TRC) << "Data " << f->data << endl;

            // Remontagem do fragmento na heap
            char * next = reinterpret_cast<char*>(base) + offset;
            memcpy(next, f->data,  size);

        }  

        db<Network_buffer>(WRN) << "Datagrama: " <<reinterpret_cast<char*>(base) << endl;

        if (retransmit) {
            // TODO: fazer lógica...
            unsigned char ip_final[4];
            for (int i=0; i < 4; i++) {
                ip_final[i] = fragment->header.DST_ADDR[i];
            }

            db<ARP_Manager>(TRC) << "ARP_Manager::receive() - Retransmissao: " << static_cast<int>(ip_final[0]) << ".";
            db<ARP_Manager>(TRC) << static_cast<int>(ip_final[1]) << ".";
            db<ARP_Manager>(TRC) << static_cast<int>(ip_final[2]) << ".";
            db<ARP_Manager>(TRC) << static_cast<int>(ip_final[3]) <<  endl;

            Address * mac_next_hop = IP_find_mac(ip_final);
            unsigned int total_length = dt_info->total_length;

            db<Network_buffer>(TRC) << "total length" << total_length << endl;
            db<Network_buffer>(WRN) << "ARP_Manager::receive() mac_next_hop: " << *mac_next_hop << endl;
            IP_send(reinterpret_cast<char*>(base), total_length, ip_final, mac_next_hop);
            db<Network_buffer>(WRN) << "Retransmitiu" << endl;
        }

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
            Datagram_Header * header = (reinterpret_cast<Datagram_Header*>(desc->address + 14));
            
            bool retransmit = false;
            for (int i = 0; (!retransmit) && i < 4; i++) {
                if ((ARP_Manager::_arp_mng->IP_ADDR[i] != header->DST_ADDR[i]) && (header->DST_ADDR[i] !=  Network_buffer::net_buffer->localhost->object()->destination[i])) {
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
            net_buffer->IP_receive((void *)(payload + 14), retransmit);

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

Network_buffer::Address * Network_buffer::IP_find_mac(unsigned char* dst_ip)
 {

    db<Network_buffer>(WRN) << "IP_find_mac()" << endl;
    
    if (IP_is_localhost(dst_ip) || IP_is_my_network(dst_ip)) {
        db<Network_buffer>(WRN) << "IP_find_mac()::É localhost() ou Minha rede " << endl;
        return ARP_Manager::_arp_mng->get_mac(dst_ip);
        
    } else { // é externa

        db<Network_buffer>(WRN) << "IP_find_mac():: É external" << endl;
        
        // Varrer todas a entradas da tabela de roteamento
        IP_Element * e = external;

        unsigned char subnetwork1[4];
        unsigned char subnetwork2[4];

        bool find = true;
        for (; e; e = e->next()) {

            for (int i = 0; i < 4; i++) {
                
                subnetwork1[i] = dst_ip[i] & e->object()->genmask[i];
                subnetwork2[i] = e->object()->destination[i] & e->object()->genmask[i];

                db<Network_buffer>(TRC) << "IP_find_mac::submask1[" << i << "]: " << static_cast<int>(subnetwork1[i]) << endl;
                db<Network_buffer>(TRC) << "IP_find_mac::submask2[" << i << "]: " << static_cast<int>(subnetwork2[i]) << endl;

                
                if (subnetwork1[i] != subnetwork2[i]) {
                    find = false;
                    break;
                }
            }

            if (find) break;
        }

        // b. Não encontrou a rede
        if (!find) e = default_router;

        unsigned char* gateway = e->object()->gateway;

        db<ARP_Manager>(TRC) << "gateway (IP=" << static_cast<int>(gateway[0]) << ".";
        db<ARP_Manager>(TRC) << static_cast<int>(gateway[1]) << ".";
        db<ARP_Manager>(TRC) << static_cast<int>(gateway[2]) << ".";
        db<ARP_Manager>(TRC) << static_cast<int>(gateway[3]) << ")" <<endl;

        
        // ii. Pega o ip vindo de 'a' ou 'b'
        Address * mac = ARP_Manager::_arp_mng->get_mac(gateway);
        return mac;
            // i. Vai na tabela ARP
                // Tem na tabela ARP -> Pega o Mac
                // Não tem na tabela -> Faz o ARP Request
    }

    return nullptr;

}


bool Network_buffer::IP_is_localhost(unsigned char * dst_ip) {
    db<Network_buffer>(TRC) << "IP_is_localhost(IP=" << static_cast<int>(dst_ip[0]) << ".";
    db<Network_buffer>(TRC) << static_cast<int>(dst_ip[1]) << ".";
    db<Network_buffer>(TRC) << static_cast<int>(dst_ip[2]) << ".";
    db<Network_buffer>(TRC) << static_cast<int>(dst_ip[3]) << ")" <<endl;


    unsigned char* submask = localhost->object()->genmask;
    unsigned char* dst_table = localhost->object()->destination;
    
    // Aplicando a máscara de sub-rede no endereço IP
    unsigned char subnetwork1[4];
    unsigned char subnetwork2[4];


    for (int i = 0; i < 4; i++) {
        subnetwork1[i] = dst_ip[i] & submask[i];
        subnetwork2[i] = dst_table[i] & submask[i];

        db<ARP_Manager>(TRC) << "is_my_network::submask[" << i << "]: " << static_cast<int>(submask[i]) << endl;
        db<ARP_Manager>(TRC) << "is_my_network::dst_table[" << i << "]: " << static_cast<int>(dst_table[i]) << endl;
        db<ARP_Manager>(TRC) << "is_my_network::subnetwork1[" << i << "]: " << static_cast<int>(subnetwork1[i]) << endl;
        db<ARP_Manager>(TRC) << "is_my_network::subnetwork2[" << i << "]: " << static_cast<int>(subnetwork2[i]) << endl;

        if (subnetwork1[i] != subnetwork2[i]) {
            return false;
        }
    }

    return true;
}

bool Network_buffer::IP_is_my_network(unsigned char * dst_ip) {
     
    db<Network_buffer>(TRC) << "is_my_network::ip(IP=" << static_cast<int>(dst_ip[0]) << ".";
    db<Network_buffer>(TRC) << static_cast<int>(dst_ip[1]) << ".";
    db<Network_buffer>(TRC) << static_cast<int>(dst_ip[2]) << ".";
    db<Network_buffer>(TRC) << static_cast<int>(dst_ip[3]) << ")" <<endl;

    unsigned char* submask =  internal->object()->genmask;
    unsigned char* dst_table =  internal->object()->destination;

    // Aplicando a máscara de sub-rede no endereço IP
    unsigned char subnetwork1[4];
    unsigned char subnetwork2[4];


    for (int i = 0; i < 4; i++) {
        subnetwork1[i] = dst_ip[i] & submask[i];
        subnetwork2[i] = dst_table[i] & submask[i];

        db<ARP_Manager>(TRC) << "is_my_network::submask1[" << i << "]: " << static_cast<int>(subnetwork1[i]) << endl;
        db<ARP_Manager>(TRC) << "is_my_network::submask2[" << i << "]: " << static_cast<int>(subnetwork2[i]) << endl;

        if (subnetwork1[i] != subnetwork2[i]) {
            return false;
        }
    }

    return true;

}


void Network_buffer::IP_add_entry(unsigned char* dst, unsigned char* gateway, unsigned char* genmask) {

    // Criando uma nova entrada na tabela
    IPTableEntry * entry = new IPTableEntry;

    for (int i = 0; i < 4; i++) {
        entry->destination[i] = dst[i];
        entry->gateway[i] = gateway[i];
        entry->genmask[i] = genmask[i];

    }    
    // Adicionando na ARP List
    IP_Element * link = new IP_Element(entry);
    routing_table->insert(link);

}

void Network_buffer::IP_populate_routing_table() {

    // Default

    if (SiFiveU_NIC::_device->address[5] % 2) {
        unsigned char dst1[4] = {0, 0, 0, 0};
        unsigned char gateway1[4] = {150, 162, 60, 2};
        unsigned char genmask1[4] = {0, 0, 0, 0};
        IP_add_entry(dst1, gateway1, genmask1);
    } else {
        unsigned char dst1[4] = {0, 0, 0, 0};
        unsigned char gateway1[4] = {144, 121, 100, 2};
        unsigned char genmask1[4] = {0, 0, 0, 0};
        IP_add_entry(dst1, gateway1, genmask1);
    }
    
    // localhost
    unsigned char dst2[4] = {127, 0, 0, 1};
    unsigned char gateway2[4] = {127, 0, 0, 1};
    unsigned char genmask2[4] = {255, 255, 255, 255};
    IP_add_entry(dst2, gateway2, genmask2);

    // internal
    unsigned char dst3[4] = {150, 162, 60, 0};
    unsigned char gateway3[4] = {0, 0, 0, 0};
    unsigned char genmask3[4] = {255, 255, 255, 0};
    IP_add_entry(dst3, gateway3, genmask3);

    // external
    unsigned char dst4[4] = {210, 154, 80, 0};
    unsigned char gateway4[4] = {144, 121, 100, 2};
    unsigned char genmask4[4] = {255, 255, 0, 0};
    IP_add_entry(dst4, gateway4, genmask4);
       
    // external
    unsigned char dst5[4] = {180, 155, 0, 0};
    unsigned char gateway5[4] = {169, 225, 236, 3};
    unsigned char genmask5[4] = {255, 255, 255, 0};
    IP_add_entry(dst5, gateway5, genmask5);


    default_router = routing_table->head();
    localhost  = default_router->next();
    internal   = localhost->next();
    external   = internal->next();

    IP_Element * e;
    for (e = routing_table->head(); e; e = e->next()) {

        db<ARP_Manager>(TRC) << "destination(=" << static_cast<int>(e->object()->destination[0]) << ".";
        db<ARP_Manager>(TRC) << static_cast<int>(e->object()->destination[1]) << ".";
        db<ARP_Manager>(TRC) << static_cast<int>(e->object()->destination[2]) << ".";
        db<ARP_Manager>(TRC) << static_cast<int>(e->object()->destination[3]) << ")\n" <<endl;

        db<ARP_Manager>(TRC) << "gateway(=" << static_cast<int>(e->object()->gateway[0]) << ".";
        db<ARP_Manager>(TRC) << static_cast<int>(e->object()->gateway[1]) << ".";
        db<ARP_Manager>(TRC) << static_cast<int>(e->object()->gateway[2]) << ".";
        db<ARP_Manager>(TRC) << static_cast<int>(e->object()->gateway[3]) << ")\n" <<endl;

        db<ARP_Manager>(TRC) << "genmask(=" << static_cast<int>(e->object()->genmask[0]) << ".";
        db<ARP_Manager>(TRC) << static_cast<int>(e->object()->genmask[1]) << ".";
        db<ARP_Manager>(TRC) << static_cast<int>(e->object()->genmask[2]) << ".";
        db<ARP_Manager>(TRC) << static_cast<int>(e->object()->genmask[3]) << ")\n" <<endl;
    }

}

__END_SYS
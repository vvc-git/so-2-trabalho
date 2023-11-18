#include <machine/riscv/ip_manager.h>

__BEGIN_SYS

IP_Manager* IP_Manager::_ip_mng;

void IP_Manager::init() {
    db<IP_Manager>(TRC) << "IP_Manager::init()"<< endl;
    _ip_mng = new (SYSTEM) IP_Manager();
    _ip_mng->attach(UDP_Manager::udp_mng);
}



bool IP_Manager::is_localhost(unsigned char * dst_ip) {
    db<IP_Manager>(TRC) << "IP_is_localhost(IP=" << static_cast<int>(dst_ip[0]) << ".";
    db<IP_Manager>(TRC) << static_cast<int>(dst_ip[1]) << ".";
    db<IP_Manager>(TRC) << static_cast<int>(dst_ip[2]) << ".";
    db<IP_Manager>(TRC) << static_cast<int>(dst_ip[3]) << ")" <<endl;


    unsigned char* submask = localhost->object()->genmask;
    unsigned char* dst_table = localhost->object()->destination;
    
    // Aplicando a máscara de sub-rede no endereço IP
    unsigned char subnetwork1[4];
    unsigned char subnetwork2[4];


    for (int i = 0; i < 4; i++) {
        subnetwork1[i] = dst_ip[i] & submask[i];
        subnetwork2[i] = dst_table[i] & submask[i];

        db<IP_Manager>(TRC) << "is_my_network::submask[" << i << "]: " << static_cast<int>(submask[i]) << endl;
        db<IP_Manager>(TRC) << "is_my_network::dst_table[" << i << "]: " << static_cast<int>(dst_table[i]) << endl;
        db<IP_Manager>(TRC) << "is_my_network::subnetwork1[" << i << "]: " << static_cast<int>(subnetwork1[i]) << endl;
        db<IP_Manager>(TRC) << "is_my_network::subnetwork2[" << i << "]: " << static_cast<int>(subnetwork2[i]) << endl;

        if (subnetwork1[i] != subnetwork2[i]) {
            return false;
        }
    }

    return true;
}

bool IP_Manager::is_my_network(unsigned char * dst_ip) {
     
    db<IP_Manager>(TRC) << "is_my_network::ip(IP=" << static_cast<int>(dst_ip[0]) << ".";
    db<IP_Manager>(TRC) << static_cast<int>(dst_ip[1]) << ".";
    db<IP_Manager>(TRC) << static_cast<int>(dst_ip[2]) << ".";
    db<IP_Manager>(TRC) << static_cast<int>(dst_ip[3]) << ")" <<endl;

    unsigned char* submask =  internal->object()->genmask;
    unsigned char* dst_table =  internal->object()->destination;

    // Aplicando a máscara de sub-rede no endereço IP
    unsigned char subnetwork1[4];
    unsigned char subnetwork2[4];


    for (int i = 0; i < 4; i++) {
        subnetwork1[i] = dst_ip[i] & submask[i];
        subnetwork2[i] = dst_table[i] & submask[i];

        db<IP_Manager>(TRC) << "is_my_network::submask1[" << i << "]: " << static_cast<int>(subnetwork1[i]) << endl;
        db<IP_Manager>(TRC) << "is_my_network::submask2[" << i << "]: " << static_cast<int>(subnetwork2[i]) << endl;

        if (subnetwork1[i] != subnetwork2[i]) {
            return false;
        }
    }

    return true;

}

void IP_Manager::add_entry(unsigned char* dst, unsigned char* gateway, unsigned char* genmask) {

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

void IP_Manager::populate_routing_table() {

    // Default

    if (SiFiveU_NIC::_device->address[5] % 2) {
        unsigned char dst1[4] = {0, 0, 0, 0};
        unsigned char gateway1[4] = {150, 162, 60, 2};
        unsigned char genmask1[4] = {0, 0, 0, 0};
        add_entry(dst1, gateway1, genmask1);
    } else {
        unsigned char dst1[4] = {0, 0, 0, 0};
        unsigned char gateway1[4] = {144, 121, 100, 2};
        unsigned char genmask1[4] = {0, 0, 0, 0};
        add_entry(dst1, gateway1, genmask1);
    }
    
    // localhost
    unsigned char dst2[4] = {127, 0, 0, 1};
    unsigned char gateway2[4] = {127, 0, 0, 1};
    unsigned char genmask2[4] = {255, 255, 255, 255};
    add_entry(dst2, gateway2, genmask2);

    // internal
    unsigned char dst3[4] = {150, 162, 60, 0};
    unsigned char gateway3[4] = {0, 0, 0, 0};
    unsigned char genmask3[4] = {255, 255, 255, 0};
    add_entry(dst3, gateway3, genmask3);

    // external
    unsigned char dst4[4] = {210, 154, 80, 0};
    unsigned char gateway4[4] = {144, 121, 100, 2};
    unsigned char genmask4[4] = {255, 255, 0, 0};
    add_entry(dst4, gateway4, genmask4);
       
    // external
    unsigned char dst5[4] = {180, 155, 0, 0};
    unsigned char gateway5[4] = {169, 225, 236, 3};
    unsigned char genmask5[4] = {255, 255, 255, 0};
    add_entry(dst5, gateway5, genmask5);


    default_router = routing_table->head();
    localhost  = default_router->next();
    internal   = localhost->next();
    external   = internal->next();

    IP_Element * e;
    for (e = routing_table->head(); e; e = e->next()) {

        db<IP_Manager>(TRC) << "destination(=" << static_cast<int>(e->object()->destination[0]) << ".";
        db<IP_Manager>(TRC) << static_cast<int>(e->object()->destination[1]) << ".";
        db<IP_Manager>(TRC) << static_cast<int>(e->object()->destination[2]) << ".";
        db<IP_Manager>(TRC) << static_cast<int>(e->object()->destination[3]) << ")\n" <<endl;

        db<IP_Manager>(TRC) << "gateway(=" << static_cast<int>(e->object()->gateway[0]) << ".";
        db<IP_Manager>(TRC) << static_cast<int>(e->object()->gateway[1]) << ".";
        db<IP_Manager>(TRC) << static_cast<int>(e->object()->gateway[2]) << ".";
        db<IP_Manager>(TRC) << static_cast<int>(e->object()->gateway[3]) << ")\n" <<endl;

        db<IP_Manager>(TRC) << "genmask(=" << static_cast<int>(e->object()->genmask[0]) << ".";
        db<IP_Manager>(TRC) << static_cast<int>(e->object()->genmask[1]) << ".";
        db<IP_Manager>(TRC) << static_cast<int>(e->object()->genmask[2]) << ".";
        db<IP_Manager>(TRC) << static_cast<int>(e->object()->genmask[3]) << ")\n" <<endl;
    }

}

void IP_Manager::routing(void * datagram) {
    db<IP_Manager>(WRN) << "Routing " << endl;

    IP::Header* header = reinterpret_cast<IP::Header*>(datagram); 

    unsigned char ip_final[4];
    for (int i=0; i < 4; i++) {
        ip_final[i] = header->DST_ADDR[i];
    }

    db<ARP_Manager>(TRC) << "\nRetransmissao para: " << static_cast<int>(ip_final[0]) << ".";
    db<ARP_Manager>(TRC) << static_cast<int>(ip_final[1]) << ".";
    db<ARP_Manager>(TRC) << static_cast<int>(ip_final[2]) << ".";
    db<ARP_Manager>(TRC) << static_cast<int>(ip_final[3]) <<  endl;

    Address * mac_next_hop = find_mac(ip_final);

    unsigned int total_length = ntohs(header->Total_Length);
    unsigned int header_length = (header->Version_IHL & GET_IHL)*4;
    unsigned int data_size = total_length - header_length;

    db<IP_Manager>(TRC) << "total length" << total_length << endl;
    db<IP_Manager>(TRC) << "header length: " << header_length << endl;
    db<IP_Manager>(TRC) << "MAC do próximo gateway: " << *mac_next_hop << endl;
    db<IP_Manager>(TRC) << "Iniciando envio de dados IP" << endl;

    // IP::Header * header = new IP::Header;
    // IP_Manager::default_header(header);
    send(header, reinterpret_cast<unsigned char*>(datagram) + header_length, data_size, *mac_next_hop);
    db<IP_Manager>(TRC) << "Retransmitiu" << endl;

}


void IP_Manager::send(Header * header, void* data, unsigned int size,  Address mac) {
    db<IP_Manager>(WRN) << "IP_Manager::IP_send inicio"<< endl;
    
    // Cria o datagram para ser enviado
    unsigned char datagram[sizeof(IP::Header) + size];
    memcpy(datagram, header, sizeof(IP::Header));
    memcpy(datagram + sizeof(IP::Header), data, size);

    // Lista de fragmentos
    FList * fragments = new FList;

    if (size > (Ethernet::MTU - sizeof(IP::Header))) {

        // Fragmentação
        fragments =  fragmentation(datagram, size);

        // ** Teste
        FList::Element * e = fragments->head();
        for (; e; e = e->next() ){
            db<IP_Manager>(TRC) << "Frame[]"<< endl;
            for (long unsigned i = 0; i < ntohs(e->object()->Total_Length)-sizeof(IP::Header); i++) {
                db<IP_Manager>(TRC) <<e->object()->data[i];
            }
            db<IP_Manager>(TRC) << endl;
            
        }
    
    } else {

        // Datagramas menores que o MTU (1480)
        Fragment * fragment = new Fragment;
        fragment->header(header); 

        fragment->Total_Length = CPU_Common::htons(size + sizeof(IP::Header));

        // Copiando para a area de dados do fragmento
        memcpy(fragment->data, data, size);

        // Insere na lista de envio
        FList::Element * link = new FList::Element(fragment);
        fragments->insert(link);

        // SiFiveU_NIC::_device->send(*mac, (void*)fragment, size + sizeof(IP::Header), 0x800);

    }

    FList::Element * e = fragments->head();
    for (; e; e = e->next() ) {
        db<IP_Manager>(WRN) << "IP_Manager:: chegou na lista id: " << hex << ntohs(e->object()->Identification) << endl;
        unsigned int offset = ntohs(e->object()->Flags_Offset) & GET_OFFSET;
        db<IP_Manager>(WRN) << "IP_Manager:: offset: " << offset << endl;
        db<IP_Manager>(WRN) << "IP_Manager:: Identification: " << hex << ntohs(e->object()->Identification) << endl;
        if (ntohs(e->object()->Identification) != 0x1231 && offset >= 185) continue;
        SiFiveU_NIC::_device->send(mac, (void*)e->object(),  ntohs(e->object()->Total_Length), 0x800);
        Delay(1000000);
    }
}

void IP_Manager::receive(void* data) {
 
    
    // Cria um novo ponteiro para adicionar na lista de fragmentos que estão chegando
    // char * content = new char[Ethernet::MTU];
    // memcpy(content, data, Ethernet::MTU);

    Fragment * fragment = reinterpret_cast<Fragment*>(data);

    db<IP_Manager>(TRC) << "---------------------"<< endl;
    db<IP_Manager>(WRN) << "IP_Manager::IP_receive\n"<< endl;
    
    // Capturando os valores do fragmento
    short unsigned int header_length = (fragment->Version_IHL & GET_IHL)*4;
    unsigned int data_length = CPU_Common::ntohs(fragment->Total_Length) - header_length;
    short unsigned int identification = CPU_Common::ntohs(fragment->Identification);
    short unsigned int offset = (CPU_Common::ntohs(fragment->Flags_Offset) & GET_OFFSET) * 8;
    short unsigned int more_frags = (CPU_Common::ntohs(fragment->Flags_Offset) & MORE_FRAGS);
    short unsigned int flags = ((CPU_Common::ntohs(fragment->Flags_Offset) & GET_FLAGS));
    
    
    // Verificação se os valores estão certos
    db<IP_Manager>(TRC) << "header_length: " << header_length << endl;
    db<IP_Manager>(TRC) << "data_length: "  << data_length << endl;
    db<IP_Manager>(WRN) << "identification: " << hex << identification << endl;
    db<IP_Manager>(TRC) << "offset: " << offset << endl;
    db<IP_Manager>(TRC) << "flags: " << flags << endl;


    List::Element * e;
    // Varredura na lista de informações de datagramas
    for (e = dt_list->head(); e && e->object()->id != identification; e = e->next()) {}   

    // Verifica se já temos informações do fragmento que chegou
    // Se não tiver, inicia um datagrama novo
    if (!e) {
        db<SiFiveU_NIC>(TRC) << "Primeiro frame"<<endl;
        
        // Criando uma nova estrutura para frames do mesmo datagrama
        Simple_List<Fragment>*  fragments = new Simple_List<Fragment>();

        INFO * datagram_info = new INFO;

        // Alarme
        Functor_Handler<INFO> * functor = new Functor_Handler<INFO>(&timeout_handler, datagram_info);
        Alarm * timer = new Alarm(Microsecond(Second(10)), functor, 1);

        datagram_info->fragments = fragments;
        datagram_info->id = identification;
        datagram_info->num_fragments = 0;
        datagram_info->total_length = 0;
        datagram_info->timer = timer;
        datagram_info->timeout_handler = functor;
        datagram_info->sem = new Semaphore(1);

        // INFO * datagram_info = new INFO{identification, 0, 0, fragments, timer};
        Element * link2 = new Element(datagram_info);
        
        // Adiciona na lista de datagramas
        dt_list->insert(link2);
        e = link2; 
        
    }
    db<IP_Manager>(WRN) << "setando fragmento: " << endl;
    
    // Informações do datagrama em que o frgamentos que chegou se encontra
    INFO * dt_info = e->object();

    dt_info->sem->p();

    // Lista de fragmentos  de um mesmo datagram
    Simple_List<Fragment> * fragments = e->object()->fragments; 

    // Inserindo na lista de fragmentos
    Simple_List<Fragment>::Element * link1 = new  Simple_List<Fragment>::Element(fragment);
    fragments->insert(link1);
    
    // Incrementa o contador de fragmentos
    dt_info->num_fragments++;
    db<IP_Manager>(TRC) << "Numero de frames " << dt_info->num_fragments << endl;  
    
    // Verificação se é o ultimo fragmento,
    // Caso positivo, seta para o tamanho adequando
    unsigned int total_frame = 0;
    if (!more_frags) {
        
        // Salva o tamanho total do datagrama
        dt_info->total_length = offset + data_length;
        
        // Salva quantos frames vão ter no datagrama
        total_frame = dt_info->total_length / (IP::MTU);
        if (data_length < (IP::MTU)) total_frame++;
        
        db<IP_Manager>(TRC) << "Total length: " << dt_info->total_length << endl;
        db<IP_Manager>(TRC) << "Total frame:  " << total_frame << endl;

    }
 
    // Quando todos os framentos chegaram, remonta.
    if (total_frame == dt_info->num_fragments) {
        db<IP_Manager>(WRN) << "Inserir datagrama completo e liberar thread 2" << endl;
        complete_dtgs->insert(dt_list->remove(e));
        delete dt_info->timer;
        sem_th->v();

    } else {
        dt_info->timer->reset();
        dt_info->sem->v();
    }
    

}

IP_Manager::Address * IP_Manager::find_mac(unsigned char* dst_ip)
 {

    db<IP_Manager>(TRC) << "IP_find_mac()" << endl;
    
    if (is_localhost(dst_ip) || is_my_network(dst_ip)) {
        db<IP_Manager>(TRC) << "IP_find_mac()::É localhost() ou Minha rede " << endl;
        return ARP_Manager::_arp_mng->get_mac(dst_ip);
        
    } else { // é externa

        db<IP_Manager>(TRC) << "IP_find_mac():: É external" << endl;
        
        // Varrer todas a entradas da tabela de roteamento
        IP_Element * e = external;

        unsigned char subnetwork1[4];
        unsigned char subnetwork2[4];

        bool find = true;
        for (; e; e = e->next()) {

            for (int i = 0; i < 4; i++) {
                
                subnetwork1[i] = dst_ip[i] & e->object()->genmask[i];
                subnetwork2[i] = e->object()->destination[i] & e->object()->genmask[i];

                db<IP_Manager>(TRC) << "IP_find_mac::submask1[" << i << "]: " << static_cast<int>(subnetwork1[i]) << endl;
                db<IP_Manager>(TRC) << "IP_find_mac::submask2[" << i << "]: " << static_cast<int>(subnetwork2[i]) << endl;

                
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

        db<IP_Manager>(TRC) << "gateway (IP=" << static_cast<int>(gateway[0]) << ".";
        db<IP_Manager>(TRC) << static_cast<int>(gateway[1]) << ".";
        db<IP_Manager>(TRC) << static_cast<int>(gateway[2]) << ".";
        db<IP_Manager>(TRC) << static_cast<int>(gateway[3]) << ")" <<endl;

        
        // ii. Pega o ip vindo de 'a' ou 'b'
        Address * mac = ARP_Manager::_arp_mng->get_mac(gateway);
        return mac;
            // i. Vai na tabela ARP
                // Tem na tabela ARP -> Pega o Mac
                // Não tem na tabela -> Faz o ARP Request
    }

    return nullptr;

}

void IP_Manager::timeout_handler(INFO * dt_info) {

    db<IP_Manager>(WRN) << "Timeout handler id: " << hex << dt_info->id <<endl;
    dt_info->sem->p();
    IP_Manager::_ip_mng->clear_dt_info(dt_info);

} 

void IP_Manager::clear_dt_info(INFO * dt_info) {

    db<IP_Manager>(WRN) << "Clear dt_info id: " << hex << dt_info->id <<endl;

    db<IP_Manager>(WRN) << "Deletando fragmentos " << endl;
    unsigned long size = dt_info->fragments->size();
    while (size > 0)
    {
        Simple_List<IP::Fragment>::Element * e =  dt_info->fragments->remove();
        delete e->object();
        delete e;
        size--;

    }

    db<IP_Manager>(WRN) << "Deletando timer " << endl;
    if (dt_info->timer) delete dt_info->timer;
    delete dt_info->timeout_handler;

    db<IP_Manager>(WRN) << "Deletando Elemento linkagem, INFO e semaforo " << endl;
    List::Element * d = dt_list->search(dt_info);
    complete_dtgs->remove(d);
    dt_info->sem->v();
    delete dt_info->sem;
    delete dt_info;
}

void* IP_Manager::defragmentation(INFO * dt_info) {

    db<IP_Manager>(WRN) << "Remontagem" << endl;
        
    // Capturando o 1° fragmento 
    Simple_List<Fragment>::Element * h = dt_info->fragments->head();

    // Tamanho do cabeçalho IP
    short unsigned int header_length = (h->object()->Version_IHL & GET_IHL)*4;
    db<IP_Manager>(TRC) << "Header Length: " << header_length << endl;

    // Aloca um espaço no buffer não contíguo (heap) para o datagrama
    void * base = Network_buffer::net_buffer->dt->alloc(dt_info->total_length + header_length);

    // Copiando header
    memcpy(base, reinterpret_cast<void*>(h->object()), header_length);
    
    // Percorre a lista de fragmentos para a remontagem
    for (; h; h = h->next()) {

        // Fragmento que será colocado no datagrama
        Fragment * f = h->object();

        // Offset do fragmento
        short unsigned int offset = (CPU_Common::ntohs(f->Flags_Offset) & GET_OFFSET) * 8;
        
        // Tamanho do fragmento
        unsigned int size = CPU_Common::ntohs(f->Total_Length) - header_length;

        db<IP_Manager>(TRC) << "Size " << size << endl;
        db<IP_Manager>(TRC) << "Offset do frame " << offset <<endl;
        db<IP_Manager>(TRC) << "Data " << f->data << endl;

        // Remontagem do fragmento na heap
        char * next = reinterpret_cast<char*>(base) + offset + header_length;
        memcpy(next, f->data,  size);

    }  

    // Setando Total Length como tamanho de todo o datagrama e não de um fragmento individual
    IP::Header* header = reinterpret_cast<IP::Header*>(base);
    header->Total_Length = htons(dt_info->total_length + header_length);
    db<IP_Manager>(TRC) << "Total Length: " << htons(header->Total_Length) << endl;

    db<IP_Manager>(WRN) << "\nRecebido datagrama:" << endl;
    for (unsigned int i = 20; i < dt_info->total_length; i++) {
        db<IP_Manager>(WRN) << reinterpret_cast<char*>(base)[i];
    }
    db<IP_Manager>(WRN) << endl;

    return base;

    // TODO: Refazer a retransmissao fora (No handler da thread)
    // if (retransmit) {
    //     routing(h->object()->DST_ADDR, dt_info->total_length, reinterpret_cast<unsigned char*>(base));
    // } else {
    //     db<IP_Manager>(TRC) << "Sou o destino final deste datagrama" << endl;
    // }


    // TODO: Mover para outro lugar
    // clear_dt_info(dt_info);

}

int IP_Manager::handler() {
       
    db<IP_Manager>(WRN) << "Chegou na thread" << endl;
    while (true) {
        db<IP_Manager>(WRN) << "Requisitando semáforo" << endl;
        IP_Manager::_ip_mng->sem_th->p();
        db<IP_Manager>(WRN) << "Semáforo concedido" << endl;

        // Lista de datagramas com todos os fragmentos
        List * complete_dtgs = IP_Manager::_ip_mng->complete_dtgs;
        
        List::Element * e = complete_dtgs->head();
        
        if (!e) {
            db<IP_Manager>(WRN) << "head() é nullptr" << endl;
            continue;
        }

        db<IP_Manager>(WRN) << "Objeto inserido " << hex << e->object()->id << endl;
        
        // Inicia a remontagem dos fragmentos (completos) que chegaram
        void* datagram = IP_Manager::_ip_mng->defragmentation(e->object()); 

        IP::Header* header = reinterpret_cast<IP::Header*>(datagram);

        // Verifica se é necessário retransmitir (IP é meu)
        bool retransmit = false;
        for (int i = 0; (!retransmit) && i < 4; i++) {
            if ((IP_Manager::_ip_mng->my_ip[i] != header->DST_ADDR[i]) && (header->DST_ADDR[i] !=  IP_Manager::_ip_mng->localhost->object()->destination[i])) {
                retransmit = true;
            }
        }

        if (retransmit) {
            IP_Manager::_ip_mng->routing(datagram);
        } else {
            db<IP_Manager>(WRN) << "Notificando UDP" << endl;
            IP_Manager::_ip_mng->notify(reinterpret_cast<unsigned char*>(datagram));
        }

        IP_Manager::_ip_mng->clear_dt_info(e->object());

        // complete_dtgs->remove(e);
        // TODO: limpar memória (clean) do INFO remontado
        

    }
    return 0;   
}

 void IP_Manager::default_header(IP::Header * header) {

    db<IP_Manager>(WRN) << "Começo do default " << endl;

    // Internet Protocol Version (IPV4)   
    Reg8 version = 4 << 4;

    // Internet Header length (in words)
    Reg8 IHL = (sizeof(IP::Header) / 4);

    // Default values
    header->Version_IHL = (version | IHL);
    header->Type_Service = 0; 
    header->TTL = 64;
    header->Protocol = 253;

    // Campos setados para datagramas menores que 1480
    header->Identification = 0x3210;
    unsigned int offset = 0;
    header->Flags_Offset = (offset & LAST_FRAG);
    
    // Tamanho padrão de cada fragmento
    header->Total_Length = CPU_Common::htons(Ethernet::MTU);
    db<IP_Manager>(WRN) << "Começo do default " << endl;

    // header->fragment_Checksum = 0;
 }

IP_Manager::FList * IP_Manager::fragmentation(void * datagram, unsigned int size) {

    FList * fragments = new FList;

    // Header do datagrama
    IP::Header * header = new IP::Header;
    memcpy(header, datagram, sizeof(IP::Header));

    // Dado do datagrama
    unsigned char data[size];
    memcpy(data, reinterpret_cast<unsigned char*>(datagram) + sizeof(IP::Header), size);

    // Ponteiro que irá avançar sobre datagrama para fazer o memcpy
    unsigned char * next;

    // Tamanho do fragmento sem o cabeçalho
    unsigned int frag_data_size = Ethernet::MTU - sizeof(IP::Header);
    db<IP_Manager>(TRC) << "frag_data_size" << frag_data_size << endl;

    // Identificador de cada fragmento
    unsigned int id = 0x1230 + id_send;

    // A divisão de fragmentos é exata ou não
    unsigned int exact = size  % frag_data_size;
    db<IP_Manager>(TRC) << "exact " << exact << endl;

    // Numero de fragmentos
    // (Soma 1 quando são numeros quebrados, pois é pego o menor)
    unsigned int num_fragments = exact ? (size / frag_data_size) + 1 : (size / frag_data_size);
    db<IP_Manager>(TRC) << "num_fragments " << num_fragments << endl;

    // ** ASSUMINDO QUE NUM_FRAGRMENTS É MAIOR QUE 1 (SIZE > 1480)
    unsigned offset = 0;
    unsigned i = 0;
    Reg16 flag_off = 0;
    for (; i < num_fragments; i++) {

        Fragment * fragment = new Fragment;
        fragment->header(header); 

        // Campos específico da fragmentação (Identification, Flags, Fragment Offset)
        fragment->Identification = CPU_Common::htons(id);
        id_send++;

        // Flags e Fragment ofsset
        offset = (i * frag_data_size) / 8;
        flag_off = (offset | MORE_FRAGS);
        fragment->Flags_Offset = CPU_Common::htons(flag_off);

        // Setando total_length
        fragment->Total_Length = CPU_Common::htons(frag_data_size + sizeof(IP::Header));

        //  Setando o ponteiro para o proximo endereço fragmentado
        next = data + i * frag_data_size ;

        // Quando for o utlimo
        if (i == (num_fragments-1)) {
            // Restante de bytes mais o Header
            frag_data_size = size  % frag_data_size; // last_size
            fragment->Total_Length = CPU_Common::htons(frag_data_size + sizeof(IP::Header));

            // Flags e Fragment offset
            flag_off = (offset & LAST_FRAG);
            fragment->Flags_Offset = CPU_Common::htons(flag_off);

        }

        // Copiando para a area de dados do fragmento
        memcpy(fragment->data, next, frag_data_size);

        // Inserindo na lista de fragmentos
        FList::Element * link = new FList::Element(fragment);
        fragments->insert(link);
    
    }

    return fragments;
}

void IP_Manager::set_own_IP() {
    // Setando o proprio endereco IP a partir do MAC definido no makefile 150, 162, 60, 0
    my_ip[0] = 150;      
    my_ip[1] = 162;
    my_ip[2] = 60;
    my_ip[3] = SiFiveU_NIC::_device->address[5];
}

__END_SYS
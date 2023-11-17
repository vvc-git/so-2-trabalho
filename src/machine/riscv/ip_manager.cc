#include <machine/riscv/ip_manager.h>

__BEGIN_SYS

IP_Manager* IP_Manager::_ip_mng;

void IP_Manager::init() {
    db<IP_Manager>(TRC) << "IP_Manager::init()"<< endl;
    _ip_mng = new (SYSTEM) IP_Manager();
    _ip_mng->attach(UDP_Manager::udp_mng);
    char teste = 'a';
    _ip_mng->notify(&teste);
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

void IP_Manager::routing(unsigned char * ip, unsigned int total_length, unsigned char * data) {

    unsigned char ip_final[4];
    for (int i=0; i < 4; i++) {
        ip_final[i] = ip[i];
    }

    db<ARP_Manager>(WRN) << "\nRetransmissao para: " << static_cast<int>(ip_final[0]) << ".";
    db<ARP_Manager>(WRN) << static_cast<int>(ip_final[1]) << ".";
    db<ARP_Manager>(WRN) << static_cast<int>(ip_final[2]) << ".";
    db<ARP_Manager>(WRN) << static_cast<int>(ip_final[3]) <<  endl;

    Address * mac_next_hop = find_mac(ip_final);

    db<IP_Manager>(TRC) << "total length" << total_length << endl;
    db<IP_Manager>(WRN) << "MAC do próximo gateway: " << *mac_next_hop << endl;
    db<IP_Manager>(WRN) << "Iniciando envio de dados IP" << endl;
    send(data, total_length, ip_final, mac_next_hop);
    db<IP_Manager>(TRC) << "Retransmitiu" << endl;

}


void IP_Manager::send(unsigned char* data, unsigned int data_size, unsigned char * dst_ip, Address * dst_mac) {
    db<IP_Manager>(TRC) << "IP_Manager::IP_send inicio"<< endl;

    // Setando MAC de destino
    Address dst = *dst_mac;
    
    // Irá avançar sobre data para fazer o memcpy
    unsigned char * data_pointer;

    unsigned int header_size = sizeof(IP::Header);
    unsigned int nic_mtu = Ethernet::MTU;
    unsigned int frag_data_size = nic_mtu - header_size;
    unsigned int iter = (data_size/frag_data_size) + 1;
    unsigned int last_size = data_size  % frag_data_size;

    unsigned int id = 0x1230 + id_send;
    id_send++;

    Fragment fragment = Fragment();
    Reg8 version = 4 << 4;
    Reg8 IHL = (header_size/4);
    fragment.Version_IHL = (version | IHL);
    fragment.Type_Service = 0;
    fragment.Total_Length = CPU_Common::htons(nic_mtu);
    fragment.Identification = CPU_Common::htons(id);
    fragment.TTL = 64;
    fragment.Protocol = 253;
    // fragment_Checksum = 0;

    for (int i = 0; i < 4; i++) {
        fragment.SRC_ADDR[i] = ARP_Manager::_arp_mng->IP_ADDR[i];
        fragment.DST_ADDR[i] = dst_ip[i];
    }

    db<IP_Manager>(TRC) << "Identification: " << hex << CPU_Common::htons(fragment.Identification) << endl;
    for (unsigned int i = 0; i < iter; i++) {
        if (i == iter - 1 && !last_size) break; // fragmentação já realizada
        db<IP_Manager>(TRC) << i << "° fragmento " << endl;

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
        fragment.Flags_Offset = CPU_Common::htons(flag_off);

        // Setando o ponteiro para o endereco especifico em data
        data_pointer = data + i*frag_data_size;

        // Copiando os dados para o fragment que sera enviado
        unsigned int size = (i == iter - 1) ? last_size : frag_data_size;
        memcpy(fragment.data, data_pointer, size);

        // Preenchimento do último fragmento e Seta total length
        if (i == iter - 1) {
            db<IP_Manager>(TRC) << "Last_size " << last_size << endl;
            fragment.Total_Length = CPU_Common::htons(last_size + header_size);
            // data_pointer = fragment.data + last_size;   
            // memset(data_pointer, '9', frag_data_size - last_size);
        }

        // if (i == 1) continue;
        SiFiveU_NIC::_device->send(dst, (void*) &fragment, size + header_size, 0x0800);
        Delay (1000000);

        // Print para verificar o header
        db<IP_Manager>(TRC) << "Total_Length original " << CPU_Common::ntohs(fragment.Total_Length)<< endl;
        db<IP_Manager>(TRC) << "Total_Length sem o size: " << (CPU_Common::ntohs(fragment.Total_Length) * 8) - header_size << endl;
        db<IP_Manager>(TRC) << "Identification: " << hex << CPU_Common::htons(fragment.Identification) << endl;
        db<IP_Manager>(TRC) << "Offset: " << (CPU_Common::htons(fragment.Flags_Offset) & GET_OFFSET)*8 << endl;
    }
    
    db<IP_Manager>(TRC) << "---------------------"<< endl;
    db<IP_Manager>(TRC) << "IP_Manager::IP_send fim"<< endl;
}

void IP_Manager::receive(void* data, bool retransmit) {
 
    
    // Cria um novo ponteiro para adicionar na lista de fragmentos que estão chegando
    // char * content = new char[Ethernet::MTU];
    // memcpy(content, data, Ethernet::MTU);

    Fragment * fragment = reinterpret_cast<Fragment*>(data);

    db<IP_Manager>(TRC) << "---------------------"<< endl;
    db<IP_Manager>(TRC) << "IP_Manager::IP_receive\n"<< endl;
    
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
    db<IP_Manager>(TRC) << "identification: " << hex << identification << endl;
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
        Alarm * timer = new Alarm(Microsecond(Second(20)), functor, 1);

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
        complete_dtgs->insert(dt_list->remove(e));
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

    db<IP_Manager>(WRN) << "Timeout handler" <<endl;
    dt_info->sem->p();
    IP_Manager::_ip_mng->clear_dt_info(dt_info);

} 

void IP_Manager::clear_dt_info(INFO * dt_info) {

    db<IP_Manager>(WRN) << "Clear dt_info" <<endl;

    db<IP_Manager>(TRC) << "Deletando fragmentos " << endl;
    unsigned long size = dt_info->fragments->size();
    while (size > 0)
    {
        Simple_List<IP::Fragment>::Element * e =  dt_info->fragments->remove();
        delete e->object();
        delete e;
        size--;

    }

    db<IP_Manager>(TRC) << "Deletando timer " << endl;
    delete dt_info->timer;
    delete dt_info->timeout_handler;

    List::Element * d = dt_list->search(dt_info);
    dt_list->remove(d);
    dt_info->sem->v();
    delete dt_info->sem;
    delete dt_info;
}

void IP_Manager::defragmentation(INFO * dt_info, bool retransmit) {

    db<IP_Manager>(WRN) << "Remontagem" << endl;
        
        // Capturando o 1° fragmento 
        Simple_List<Fragment>::Element * h = dt_info->fragments->head();

        // Aloca um espaço no buffer não contíguo (heap) para o datagrama
        void * base = Network_buffer::net_buffer->dt->alloc(dt_info->total_length);

        // Tamanho do cabeçalho IP
        short unsigned int header_length = (h->object()->Version_IHL & GET_IHL)*4;
        db<IP_Manager>(TRC) << "Header Length: " << header_length << endl;
        
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
            char * next = reinterpret_cast<char*>(base) + offset;
            memcpy(next, f->data,  size);

        }  

        db<IP_Manager>(WRN) << "\nRecebido datagrama:" << endl;
        for (unsigned int i = 0; i < dt_info->total_length; i++) {
            db<IP_Manager>(WRN) << reinterpret_cast<char*>(base)[i];
        }
        db<IP_Manager>(WRN) << endl;


        // TODO: Refazer a retransmissao fora (No handler da thread)
        // if (retransmit) {
        //     routing(h->object()->DST_ADDR, dt_info->total_length, reinterpret_cast<unsigned char*>(base));
        // } else {
        //     db<IP_Manager>(WRN) << "Sou o destino final deste datagrama" << endl;
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
        List::Element * e;

        // Varredura na lista de informações de datagramas
        for (e = complete_dtgs->head(); e; e = e->next()) {
            db<IP_Manager>(TRC) << "Objeto inserido " << hex << e->object()->id << endl;
            // TODO: Retransmissão hardcoded
            IP_Manager::_ip_mng->defragmentation(e->object(), true); 

            // TODO: A desfragmentação deveria retornar um datagrama completo (data + ip header)
            // TODO: IP header não está vindo (Colocar antes da base-> Refatorar)
            // TODO: Decidir se o datagrama montado vai para udp ou retransmitido
            complete_dtgs->remove(e);
            // TODO: limpar memória (clean) do INFO remontado
        }   

    }
    return 0;   
}


__END_SYS
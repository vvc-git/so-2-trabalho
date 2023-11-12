#include <machine/riscv/arp_manager.h>

__BEGIN_SYS

ARP_Manager* ARP_Manager::_arp_mng;

void ARP_Manager::init() {
    db<ARP_Manager>(TRC) << "ARP_Manager::init()"<< endl;

    _arp_mng = new (SYSTEM) ARP_Manager();
}

// ARP_Manager::ARP_Manager() {
//     db<ARP_Manager>(TRC) << "ARP_Manager::ARP_Manager()"<< endl;

// }


// Essa função retorna falso para quando a o IP destino não está na rede ou de algum erro no envio
// Se for true, foi feito um arp request ou a entrada já está na tabela. Então quem a chamou precisa ir na
// tabela
bool ARP_Manager::arp_send_request(unsigned char * dst_ip) {
    
    db<ARP_Manager>(TRC) << "ARP_Manager::send_request() "<< endl;
    
    // ** Fazendo o ARP Request 
    ARP_Packet* packet = new ARP_Packet();
   
    Address src;
    Address dst;

    src = SiFiveU_NIC::_device->address;

    dst[0] = 0x00;
    dst[1] = 0x00;
    dst[2] = 0x00;
    dst[3] = 0x00;
    dst[4] = 0x00;
    dst[5] = 0x00;

    for (int i = 0; i < 4; i++) {
        packet->_sender_prot[i] = IP_ADDR[i];
        packet->_target_prot[i] = dst_ip[i]; 
    }

    db<ARP_Manager>(TRC) << "arp_send_request::packet->target_prot: (" << static_cast<int>(packet->_target_prot[0]) << ".";
    db<ARP_Manager>(TRC) << static_cast<int>(packet->_target_prot[1]) << ".";
    db<ARP_Manager>(TRC) << static_cast<int>(packet->_target_prot[2]) << ".";
    db<ARP_Manager>(TRC) << static_cast<int>(packet->_target_prot[3]) << ")" <<endl;

    
    // Setando os pacotes
    packet->_hw_type = CPU::htons(0x01);
    packet->_prot_type = CPU::htons(0x0800);
    packet->_hw_length = 0x06;
    packet->_prot_length = 0x04;
    packet->_operation = CPU::htons(0x0001);
    packet->_sender_hw = src;
    packet->_target_hw = dst;

    SiFiveU_NIC::_device->send(dst, (void*) packet, 28, 0x0806);
    return true;

}

void ARP_Manager::arp_send_reply(ARP_Packet* requester_packet) {
    db<ARP_Manager>(TRC) << "ARP_Manager::send_reply() " <<  endl;

    ARP_Packet* packet = new ARP_Packet();

    Address src;
    Address dst;

    // Mac origem e destino
    src = SiFiveU_NIC::_device->address;
    dst = requester_packet->_sender_hw;

    // IP origem (proprio)
    packet->_sender_prot[0] = IP_ADDR[0]; // 127.0.0.2      
    packet->_sender_prot[1] = IP_ADDR[1];
    packet->_sender_prot[2] = IP_ADDR[2];
    packet->_sender_prot[3] = IP_ADDR[3];

    // IP destino (hardcoded por enquanto)
    packet->_target_prot[0] = static_cast<int>(requester_packet->_sender_prot[0]); // 127.0.0.1       
    packet->_target_prot[1] = static_cast<int>(requester_packet->_sender_prot[1]); 
    packet->_target_prot[2] = static_cast<int>(requester_packet->_sender_prot[2]); 
    packet->_target_prot[3] = static_cast<int>(requester_packet->_sender_prot[3]); 

    // Setando os pacotes
    packet->_hw_type = CPU::htons(0x01);
    packet->_prot_type = CPU::htons(0x0800);
    packet->_hw_length = 0x06;
    packet->_prot_length = 0x04;
    packet->_operation = CPU::htons(0x0002);
    packet->_sender_hw = src;
    packet->_target_hw = dst;
    

    db<ARP_Manager>(TRC) << "Sending reply to mac: " << hex << dst << endl;
    SiFiveU_NIC::_device->send(dst, (void*) packet, 28, 0x0806);
}

void ARP_Manager::arp_receive(ARP_Packet* packet) {
    db<ARP_Manager>(TRC) << "ARP_Manager::receive()"<< endl;

    unsigned int operation = ntohs(packet->_operation);

    if (operation == 0x0001) { // arp request
        db<ARP_Manager>(TRC) << "Receiving a request: "<< endl;

        if (!is_my_ip(packet->_target_prot)) {
            db<ARP_Manager>(TRC) << "IP destino nao é o meu " << endl;
            return; // Request nao é para o meu IP
        } else {
            db<ARP_Manager>(TRC) << "My IP was requested" << endl;
        }

        // Envia a resposta
        arp_send_reply(packet);

    } else { // recebendo um reply
        db<ARP_Manager>(TRC) << "Receiving a reply: " << endl;
        
        if (!is_my_ip(packet->_target_prot)) {
            db<ARP_Manager>(TRC) << "IP destino nao é o meu " << endl;
            return; // Request nao é para o meu IP
        } 

        db<ARP_Manager>(TRC) << "IP do MAC descoberto: " << static_cast<int>(packet->_sender_prot[0]) << endl;
        db<ARP_Manager>(TRC) << "MAC descoberto: " << packet->_sender_hw << endl;
        
        // Atualizar ARP table com novo mac.
        add_ip(packet->_sender_prot, packet->_sender_hw);

        // Fazer o envio para o MAC desejado
    }
    
}

void ARP_Manager::set_own_IP() {
    db<ARP_Manager>(TRC) << "ARP_Manager::set_own_IP()"<< endl;

    // Setando o proprio endereco IP a partir do MAC definido no makefile 150, 162, 60, 0
    IP_ADDR[0] = 150;      
    IP_ADDR[1] = 162;
    IP_ADDR[2] = 60;
    IP_ADDR[3] = SiFiveU_NIC::_device->address[5];

    // Capturando o mac
    Address mac = SiFiveU_NIC::_device->address;

    // // Criando uma nova entrada na tabela
    unsigned char localhost[4];
    localhost[0] = 127;      
    localhost[1] = 0;
    localhost[2] = 0;
    localhost[3] = 1;

    unsigned char external[4];
    external[0] = 144;
    external[1] = 121;
    external[2] = 100;
    external[3] = 2;
    add_ip(localhost, mac);
    add_ip(IP_ADDR, mac);

    // Para simular uma rede externa conhecida
    Address external_mac;
    external_mac[0] = 0;
    external_mac[1] = 0;
    external_mac[2] = 0;
    external_mac[3] = 0;
    external_mac[4] = 0;
    external_mac[5] = 3;

    add_ip(external, external_mac);

}

ARP_Manager::Address * ARP_Manager::search_ARP_cache(const unsigned char * ip) {
        List::Element * e;
        db<ARP_Manager>(TRC) << "ARP_Manager::search_ARP_cache(IP=" << static_cast<int>(ip[0]) << ".";
        db<ARP_Manager>(TRC) << static_cast<int>(ip[1]) << ".";
        db<ARP_Manager>(TRC) << static_cast<int>(ip[2]) << ".";
        db<ARP_Manager>(TRC) << static_cast<int>(ip[3]) << ")" <<endl;

        db<ARP_Manager>(TRC) << "Procurando endereço IP na tabela ARP" << endl;

        for (e = ARP_Table->head(); e; e = e->next()) {

            db<ARP_Manager>(TRC) << "ARP_Manager::search_ARP_cache() - Tabela: " << static_cast<int>(e->object()->ip[0]) << ".";
            db<ARP_Manager>(TRC) << static_cast<int>(e->object()->ip[1]) << ".";
            db<ARP_Manager>(TRC) << static_cast<int>(e->object()->ip[2]) << ".";
            db<ARP_Manager>(TRC) << static_cast<int>(e->object()->ip[3]) <<  endl;

            bool find = true;
            for (int i = 0; i < 4 && find; i++) {
                if (e->object()->ip[i] != ip[i]) find = false;         
            }

            if (find) {
                db<ARP_Manager>(TRC) <<"ARP_Manager::search_ARP_cache()::Mac descoberto: " << e->object()->mac << endl;
                return &(e->object()->mac);
            }

        }  

        db<ARP_Manager>(TRC) <<"ARP_Manager::search_ARP_cache()::Não achou na tabela" << endl;
        return nullptr;

}

bool ARP_Manager::is_my_ip(unsigned char * ip) {

    for (int i = 0; i < 4; i++) {

        db<ARP_Manager>(TRC) << "is_my_network::IP_ADDR[" << i << "]: " << static_cast<int>(IP_ADDR[i]) << endl;
        db<ARP_Manager>(TRC) << "is_my_network::submask2[" << i << "]: " << static_cast<int>(ip[i]) << endl;

        if (ip[i] != IP_ADDR[i]) {
            return false;
        }
    }

    return true;

}

void ARP_Manager::add_ip(const unsigned char * ip, const Address mac) {
    db<ARP_Manager>(TRC) << "ARP_Manager::add_ip()" << endl;
    
    db<ARP_Manager>(TRC) << "add_ip::ip(IP=" << static_cast<int>(ip[0]) << ".";
    db<ARP_Manager>(TRC) << static_cast<int>(ip[1]) << ".";
    db<ARP_Manager>(TRC) << static_cast<int>(ip[2]) << ".";
    db<ARP_Manager>(TRC) << static_cast<int>(ip[3]) << ")" <<endl;
    
    db<ARP_Manager>(TRC) << "add_ip::ip(MAC=" << mac << ")" << endl;
    // db<ARP_Manager>(TRC) << static_cast<int>(mac[1]) << ".";
    // db<ARP_Manager>(TRC) << static_cast<int>(mac[2]) << ".";
    // db<ARP_Manager>(TRC) << static_cast<int>(mac[3]) << ".";
    // db<ARP_Manager>(TRC) << static_cast<int>(mac[4]) << ".";
    // db<ARP_Manager>(TRC) << static_cast<int>(mac[5]) << ")" <<endl;


    ARPTableEntry * entry = new ARPTableEntry();
    entry->ip[0] = ip[0];
    entry->ip[1] = ip[1];
    entry->ip[2] = ip[2];
    entry->ip[3] = ip[3];

    entry->mac = mac;
    
    // Adicionando na ARP List
    Element * link = new Element(entry);
    ARP_Table->insert(link);

    List::Element * e;
    for (e = ARP_Table->head(); e; e = e->next()) {

        db<ARP_Manager>(TRC) << "add_ip::ip(MAC_Tabela=" << e->object()->mac << ")" << endl;     
        db<ARP_Manager>(TRC) << "add_ip::ip(IP_Tabela= "<< static_cast<int>(e->object()->ip[0]) << ".";
        db<ARP_Manager>(TRC) << static_cast<int>(e->object()->ip[1]) << ".";
        db<ARP_Manager>(TRC) << static_cast<int>(e->object()->ip[2]) << ".";
        db<ARP_Manager>(TRC) << static_cast<int>(e->object()->ip[3]) << ")\n" <<endl;
    }

    db<ARP_Manager>(TRC) << "ARP_Manager::add_ip() fim" << endl;

}

ARP_Manager::Address * ARP_Manager::get_mac(unsigned char * dst_ip) {

    unsigned int tries = 0;
    Address * mac = search_ARP_cache(dst_ip);
    if (mac) return mac;
    while (tries < 3) {  
        db<ARP_Manager>(TRC) <<"ARP request: tentativa " << tries << endl;
        arp_send_request(dst_ip);
        Delay(500000);
        tries++;
        mac = search_ARP_cache(dst_ip);
        if (mac) return mac;
    }

    return nullptr;
}

__END_SYS
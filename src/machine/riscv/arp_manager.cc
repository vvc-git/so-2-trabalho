#include <machine/riscv/arp_manager.h>

__BEGIN_SYS

ARP_Manager* ARP_Manager::_arp_mng;

void ARP_Manager::init() {
    db<ARP_Manager>(TRC) << "ARP_Manager::init()"<< endl;

    _arp_mng = new (SYSTEM) ARP_Manager();
}


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
            db<ARP_Manager>(WRN) << "My IP was requested" << endl;
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
        db<ARP_Manager>(WRN) << "MAC descoberto: " << packet->_sender_hw << endl;
        
        // Atualizar ARP table com novo mac.
        add_ip(packet->_sender_prot, packet->_sender_hw);

        // Fazer o envio para o MAC desejado
    }
    
}

void ARP_Manager::set_own_IP() {
    db<ARP_Manager>(TRC) << "ARP_Manager::set_own_IP()"<< endl;

    // Setando o proprio endereco IP a partir do MAC definido no makefile 150, 162, 60, 0
    IP_ADDR[0] = 150; // 127.0.0.2       
    IP_ADDR[1] = 162;
    IP_ADDR[2] = 60;
    IP_ADDR[3] = SiFiveU_NIC::_device->address[5];

    // Capturando o mac
    Address mac = SiFiveU_NIC::_device->address;

    // // Criando uma nova entrada na tabela
    // ARPTableEntry * entry = new ARPTableEntry{IP_ADDR, mac};
    
    // // Adicionando na ARP List
    // Element * link = new Element(entry);
    // ARP_Table->insert(link);
    add_ip(IP_ADDR, mac);

}

ARP_Manager::Address * ARP_Manager::get_mac_in_table(const unsigned char * ip) {

        List::Element * e;
        db<ARP_Manager>(TRC) << "ARP_Manager::get_mac_in_table(IP=" << static_cast<int>(ip[0]) << ".";
        db<ARP_Manager>(TRC) << static_cast<int>(ip[1]) << ".";
        db<ARP_Manager>(TRC) << static_cast<int>(ip[2]) << ".";
        db<ARP_Manager>(TRC) << static_cast<int>(ip[3]) << ")" <<endl;

        for (e = ARP_Table->head(); e; e = e->next()) {

            db<ARP_Manager>(TRC) << "ARP_Manager::get_mac_in_table() - Tabela: " << static_cast<int>(e->object()->ip[0]) << ".";
            db<ARP_Manager>(TRC) << static_cast<int>(e->object()->ip[1]) << ".";
            db<ARP_Manager>(TRC) << static_cast<int>(e->object()->ip[2]) << ".";
            db<ARP_Manager>(TRC) << static_cast<int>(e->object()->ip[3]) <<  endl;

            bool find = true;
            for (int i = 0; i < 4 && find; i++) {
                if (e->object()->ip[i] != ip[i]) find = false;         
            }

            if (find) {
                db<ARP_Manager>(TRC) <<"ARP_Manager::get_mac_in_table()::Mac descoberto: " << e->object()->mac << endl;
                return &(e->object()->mac);
            }

        }  

        db<ARP_Manager>(TRC) <<"ARP_Manager::get_mac_in_table()::Não achou na tabela" << endl;
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

void ARP_Manager::add_ip(unsigned char * ip, const Address mac) {

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

    // Criando uma nova entrada na tabela
    ARPTableEntry * entry = new ARPTableEntry{ip, mac};
    
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

}

bool ARP_Manager::send(unsigned char * dst_ip) {

    unsigned int tries = 0;
    Address * mac = get_mac_in_table(dst_ip);
    while (tries < 3) {
        db<ARP_Manager>(WRN) <<"ARP_Manager::send()::Tentativa " << tries << endl;
        if (mac) return true;
        arp_send_request(dst_ip);
        tries++;
        Delay(50000000);
        mac = get_mac_in_table(dst_ip);

    }

    return false;
}

__END_SYS
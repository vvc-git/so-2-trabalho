#include <machine/riscv/arp_manager.h>

__BEGIN_SYS

ARP_Manager* ARP_Manager::_arp_mng;

void ARP_Manager::init() {
    db<ARP_Manager>(WRN) << "ARP_Manager::init()"<< endl;

    _arp_mng = new (SYSTEM) ARP_Manager();
}


// Essa função retorna falso para quando a o IP destino não está na rede ou de algum erro no envio
// Se for true, foi feito um arp request ou a entrada já está na tabela. Então quem a chamou precisa ir na
// tabela
bool ARP_Manager::arp_send_request() {

    ARP_Packet* packet = new ARP_Packet();

    db<ARP_Manager>(WRN) << "ARP_Manager::send_request() "<< send << endl;

    Address src;
    Address dst;

    src = SiFiveU_NIC::_device->address;

    dst[0] = 0x00;
    dst[1] = 0x00;
    dst[2] = 0x00;
    dst[3] = 0x00;
    dst[4] = 0x00;
    dst[5] = 0x00;

    // IP origem (proprio)
    packet->_sender_prot[0] = IP_ADDR[0]; // 127.0.0.1      
    packet->_sender_prot[1] = IP_ADDR[1];
    packet->_sender_prot[2] = IP_ADDR[2];
    packet->_sender_prot[3] = IP_ADDR[3];

    // IP destino
    packet->_target_prot[0] = 127; // 127.0.0.2       
    packet->_target_prot[1] = 0;
    packet->_target_prot[2] = 60;
    packet->_target_prot[3] = 2;


        // Verifico se é na minha rede
    is_my_network(packet->_target_prot);
    // Se sim,
        // Verifca se está na minha tabela (Meu ip está incluso)
            // Se sim, retorno false
            // ** Se não, faço o arp request 
    // Se não,
        // return false

    
    // Setando os pacotes
    packet->_hw_type = CPU::htons(0x01);
    packet->_prot_type = CPU::htons(0x0800);
    packet->_hw_length = 0x06;
    packet->_prot_length = 0x04;
    packet->_operation = CPU::htons(0x0001);
    packet->_sender_hw = src;
    packet->_target_hw = dst;

    SiFiveU_NIC::_device->send(dst, (void*) packet, 28, 0x0806);
    send++;
    return true;

}

void ARP_Manager::arp_send_reply(ARP_Packet* requester_packet) {
    db<ARP_Manager>(TRC) << "ARP_Manager::send_reply() "<< reply <<  endl;

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

    reply++;
}

void ARP_Manager::arp_receive(ARP_Packet* packet) {
    db<ARP_Manager>(TRC) << "ARP_Manager::receive()"<< endl;

    unsigned int operation = ntohs(packet->_operation);

    if (operation == 0x0001) { // arp request
        db<ARP_Manager>(WRN) << "Receiving a request: "<< endl;

        // // !! BROADCAST
        // if (is_my_ip(packet->_sender_prot)) {
        //     db<ARP_Manager>(WRN) <<  "IP origem é o meu "<< endl;
        //     return;
        // } 

        if (!is_my_ip(packet->_target_prot)) {
            db<ARP_Manager>(WRN) << "IP destino nao é o meu " << endl;
            return; // Request nao é para o meu IP
        } 

        db<ARP_Manager>(WRN) << "My IP was requested" << endl;
        
        // Envia a resposta
        arp_send_reply(packet);

    } else { // recebendo um reply
        db<ARP_Manager>(WRN) << "Receiving a reply: " << endl;

        // // !! BROADCAST
        // if (is_my_ip(packet->_sender_prot)) {
        //     db<ARP_Manager>(WRN) << "IP origem é o meu " << endl;
        //     return; 
        // } 
        
        if (!is_my_ip(packet->_target_prot)) {
            db<ARP_Manager>(WRN) << "IP destino nao é o meu " << endl;
            return; // Request nao é para o meu IP
        } 

        db<ARP_Manager>(WRN) << "MAC descoberto: " << packet->_sender_hw << endl;
        // Atualizar ARP table com novo mac.


        // Fazer o envio para o MAC desejado
    }
    
}

void ARP_Manager::set_own_IP() {
    db<ARP_Manager>(TRC) << "ARP_Manager::set_own_IP()"<< endl;

    // Setando o proprio endereco IP a partir do MAC definido no makefile
    IP_ADDR[0] = 127; // 127.0.0.2       
    IP_ADDR[1] = 0;
    IP_ADDR[2] = 0;
    IP_ADDR[3] = SiFiveU_NIC::_device->address[5];

    // Capturando o mac
    Address mac = SiFiveU_NIC::_device->address;

    // Criando uma nova entrada na tabela
    ARPTableEntry * entry = new ARPTableEntry{IP_ADDR, mac};
    
    // Adicionando na ARP List
    Element * link = new Element(entry);
    ARP_Table->insert(link);

}

ARP_Manager::Address * ARP_Manager::get_mac_in_table(unsigned char * ip) {

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

            if (find) return &(e->object()->mac);

        }  

        return nullptr;

}

bool ARP_Manager::is_my_ip(unsigned char * ip) {

    if (get_mac_in_table(ip))
        return true;
    return false;

}


bool ARP_Manager::is_my_network(unsigned char * ip) {
     
    db<ARP_Manager>(TRC) << "is_my_network::ip(IP=" << static_cast<int>(ip[0]) << ".";
    db<ARP_Manager>(TRC) << static_cast<int>(ip[1]) << ".";
    db<ARP_Manager>(TRC) << static_cast<int>(ip[2]) << ".";
    db<ARP_Manager>(TRC) << static_cast<int>(ip[3]) << ")" <<endl;

    // Aplicando a máscara de sub-rede no endereço IP
    unsigned char subnetwork1[4];
    unsigned char subnetwork2[4];


    for (int i = 0; i < 4; i++) {
        subnetwork1[i] = ip[i] & submask[i];
        subnetwork2[i] = IP_ADDR[i] & submask[i];

        db<ARP_Manager>(TRC) << "is_my_network::submask1[" << i << "]: " << static_cast<int>(subnetwork1[i]) << endl;
        db<ARP_Manager>(TRC) << "is_my_network::submask2[" << i << "]: " << static_cast<int>(subnetwork2[i]) << endl;

        if (subnetwork1[i] != subnetwork2[i]) {
            return false;
        }

    }

    return true;

}

__END_SYS
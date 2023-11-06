#include <machine/riscv/arp_manager.h>

__BEGIN_SYS

ARP_Manager* ARP_Manager::_arp_mng;

void ARP_Manager::init() {
    db<ARP_Manager>(WRN) << "ARP_Manager::init()"<< endl;

    _arp_mng = new (SYSTEM) ARP_Manager();
}

void ARP_Manager::arp_send_request() {

    ARP_Packet* packet = new ARP_Packet();

    db<ARP_Manager>(WRN) << "ARP_Manager::send_request()"<< endl;

    Address src;
    Address dst;

    src = SiFiveU_NIC::_device->address;

    // dst[0] = 0xFF;
    // dst[1] = 0xFF;
    // dst[2] = 0xFF;
    // dst[3] = 0xFF;
    // dst[4] = 0xFF;
    // dst[5] = 0xFF;

    // src[0] = 0x00;
    // src[1] = 0x00;
    // src[2] = 0x00;
    // src[3] = 0x00;
    // src[4] = 0x00;
    // src[5] = 0x00;

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
    packet->_target_prot[2] = 0;
    packet->_target_prot[3] = 2;
    
    // Setando os pacotes
    packet->_hw_type = CPU::htons(0x01);
    packet->_prot_type = CPU::htons(0x0800);
    packet->_hw_length = 0x06;
    packet->_prot_length = 0x04;
    packet->_operation = CPU::htons(0x0001);
    packet->_sender_hw = src;
    packet->_target_hw = dst;

    SiFiveU_NIC::_device->send(dst, (void*) packet, 28, 0x0806);


}

void ARP_Manager::arp_send_reply(ARP_Packet* requester_packet) {
    db<ARP_Manager>(WRN) << "ARP_Manager::send_reply()"<< endl;

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
    

    db<ARP_Manager>(WRN) << "Sending reply to mac: " << hex << dst << endl;
    SiFiveU_NIC::_device->send(dst, (void*) packet, 28, 0x0806);
}

void ARP_Manager::arp_receive(ARP_Packet* packet) {
    db<ARP_Manager>(TRC) << "ARP_Manager::receive()"<< endl;

    unsigned int operation = ntohs(packet->_operation);

    if (operation == 0x0001) { // arp request
        db<ARP_Manager>(TRC) << "Receiving a request: " << operation << endl;

        bool my_IP = true;
        for (int i = 0; i < 4 && my_IP; i++) {
            if (IP_ADDR[i] != packet->_target_prot[i]) my_IP = false;
        }
        
        if (!my_IP) {
            db<ARP_Manager>(TRC) << "IP destino nao é o meu " << endl;
            return; // Request nao é para o meu IP
        } 

        db<ARP_Manager>(WRN) << "My IP was requested" << endl;
        
        // Envia a resposta
        arp_send_reply(packet);

    } else { // recebendo um reply
        db<ARP_Manager>(TRC) << "Receiving a reply: " << operation << endl;

        // Por enquanto, se confere o IP pois cada pacote enviado é, ao mesmo tempo,
        // recebido. Esse pacotes recebidos por estar em promiscuos mode devem ser descartados
        bool my_IP = true;
        for (int i = 0; i < 4 && my_IP; i++) {
            if (IP_ADDR[i] != packet->_target_prot[i]) my_IP = false;
        }
        
        if (!my_IP) {
            db<ARP_Manager>(TRC) << "IP destino nao é o meu " << endl;
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

}

__END_SYS
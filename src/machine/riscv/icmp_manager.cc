#include <machine/riscv/icmp_manager.h>

__BEGIN_SYS

ICMP_Manager* ICMP_Manager::_icmp_mng;

void ICMP_Manager::init() {
    db<ICMP_Manager>(TRC) << "ICMP_Manager::init()"<< endl;
    _icmp_mng = new (SYSTEM) ICMP_Manager();
}

void ICMP_Manager::send_request(unsigned char * dst_ip, Address dst_mac) {
    db<ICMP_Manager>(TRC) << "ICMP_Manager::IP_send inicio"<< endl;
    
    Echo * echo = new Echo;
    set_header(echo, true);

    // Source and Destination IP
    for (int i = 0; i < 4; i++) {
        echo->SRC_ADDR[i] = IP_Manager::_ip_mng->my_ip[i];
        echo->DST_ADDR[i] = dst_ip[i];
    }

    SiFiveU_NIC::_device->send(dst_mac, (void*)echo, sizeof(IP::Header) + sizeof(IP::Echo), 0x0800);
    chrono->start();

    // Print para verificar o header
    db<ICMP_Manager>(TRC) << "Total_Length original " << CPU_Common::ntohs(echo->Total_Length)<< endl;
    // db<ICMP_Manager>(TRC) << "Total_Length sem o size: " << (CPU_Common::ntohs(echo->Total_Length) * 8) - hsize << endl;
    db<ICMP_Manager>(TRC) << "Identification: " << hex << CPU_Common::htons(echo->Identification) << endl;
    db<ICMP_Manager>(TRC) << "Offset: " << (CPU_Common::htons(echo->Flags_Offset) & IP_Manager::GET_OFFSET)*8 << endl;    
    db<ICMP_Manager>(TRC) << "---------------------"<< endl;
    db<ICMP_Manager>(TRC) << "ICMP_Manager::IP_send fim"<< endl;
}

void ICMP_Manager::send_reply(unsigned char * dst_ip, Address dst_mac) {
    db<ICMP_Manager>(WRN) << "ICMP_Manager::send_reply" << endl;
    // Cria um novo ponteiro para adicionar na lista de fragmentos que estão chegando
    // char * r = new char[1500];
    // memcpy(r, request, 1500);

    // // Ethernet frame
    // Frame * frame = reinterpret_cast<Frame*>(r);
    // Echo * echo = reinterpret_cast<Echo*>(r + 14);
    
    // Capturando os valores do fragmento
    // unsigned int length = CPU_Common::ntohs(echo->Total_Length) - 20;
    // short unsigned int identification = CPU_Common::ntohs(echo->Identification);
    // short unsigned int offset = (CPU_Common::ntohs(echo->Flags_Offset) & GET_OFFSET) * 8;
    // // short unsigned int more_frags = (CPU_Common::ntohs(echo->Flags_Offset) & MORE_FRAGS);
    // short unsigned int flags = ((CPU_Common::ntohs(echo->Flags_Offset) & GET_FLAGS));

    Echo * reply = new Echo;
    set_header(reply, false);
    db<ICMP_Manager>(WRN) << "Fez o set header" << endl;

    // Source and Destination IP
    for (int i = 0; i < 4; i++) {
        reply->SRC_ADDR[i] = IP_Manager::_ip_mng->my_ip[i];
        reply->DST_ADDR[i] = dst_ip[i];
    }
    db<ICMP_Manager>(WRN) << "Preencheu o IP" << sizeof(reply) + sizeof(Echo::Header) << endl;
    db<ICMP_Manager>(WRN) << "dst_mac " << dst_mac << endl;
    SiFiveU_NIC::_device->send(dst_mac, (void*)reply, sizeof(IP::Header) + sizeof(IP::Echo), 0x0800);


    // Verificação se os valores estão certos
    // db<ICMP_Manager>(WRN) << "length: " << hex << length << endl;
    // db<ICMP_Manager>(WRN) << "identification: " << hex << identification << endl;
    // db<ICMP_Manager>(WRN) << "offset: " << offset << endl;
    // db<ICMP_Manager>(WRN) << "flags: " << flags << endl;

}

void ICMP_Manager::receive(void* request) {
    db<ICMP_Manager>(WRN) << "ICMP_Manager::receive" << endl;
    // Cria um novo ponteiro para adicionar na lista de fragmentos que estão chegando
    char * r = new char[1500];
    memcpy(r, request, 1500);

    // Ethernet frame
    Frame * frame = reinterpret_cast<Frame*>(r);
    ICMP * icmp = reinterpret_cast<ICMP*>(r + 14);
    TEM * tem = reinterpret_cast<TEM*>(r + 14);

    switch (icmp->Type)
    {
    case 0:
        db<ICMP_Manager>(WRN) << "Recebendo um Echo reply" << endl;
        chrono->stop();
        db<ICMP_Manager>(WRN) << "Ping: " << chrono->read() / 1000000 << ",";
        db<ICMP_Manager>(WRN) << chrono->read() % 1000000 << "s" << endl;
        break;
    case 8:
        db<ICMP_Manager>(WRN) << "Recebendo um Echo request" << endl;
        send_reply(icmp->SRC_ADDR, frame->src());
        break;

    case 11:
        db<ICMP_Manager>(WRN) << "Recebendo um Time Message Exceeded" << endl;
        db<ICMP_Manager>(WRN) << "Datagrama com o identificador [";
        db<ICMP_Manager>(WRN) << hex << ntohs(tem->IP_header.Identification) << "] = data: ";
        for (int i=0; i < 8; i++ ) {
            db<ICMP_Manager>(WRN) << tem->data[i];
        }
        db<ICMP_Manager>(WRN) <<" foi perdido" << endl;
        break;
    default:
        db<ICMP_Manager>(WRN) << "Recebendo uma mensagem ICMP não definida" << endl;
        break;
    }


}

void ICMP_Manager::send_tem(Address dst_mac, IP::Header* header, unsigned char* data) {
    db<ICMP_Manager>(WRN) << "ICMP_Manager::send_tem" << endl;
    db<ICMP_Manager>(WRN) << "address: " << dst_mac << endl;
    db<ICMP_Manager>(WRN) << "header id: " << ntohs(header->Identification) << endl;

    TEM *tem = new TEM;

    // Novo header do ICMP
    tem->header(header);

    db<ICMP_Manager>(WRN) << "ICMP_Manager::Primeira copia funcionando" << endl;

    // Header que do datagrama com erro
    // memcpy(tem->IP_header, header, sizeof(IP::Header));
    tem->IP_header.header(header);


    // Source and Destination IP
    for (int i = 0; i < 4; i++) {
        tem->SRC_ADDR[i] = IP_Manager::_ip_mng->my_ip[i];
        tem->DST_ADDR[i] = header->SRC_ADDR[i];
    }

    // Setando header IP
    tem->Total_Length = ntohs(sizeof(IP::TEM));
    tem->Identification = 0x742;
    tem->Flags_Offset = 0; // offset 0 e flags = 000
    tem->TTL = 64;
    tem->Protocol = 1; // ICMP
    tem->Header_Checksum = 0; 

    // Setando header ICMP
    tem->Type = 11;
    tem->Code = 1;
    tem->Checksum = 0;

    memcpy(tem->data, data, 8);

    // db<ICMP_Manager>(WRN) << "total length: " << ntohs(tem->Total_Length) << endl;
    // for (int i=0; i<8; i++) {
    //     tem->data[i] = 'A';
    // }

    // for (int i=0; i<8; i++) {
    //     db<ICMP_Manager>(WRN) << data[i] ;
    // }
    // db<ICMP_Manager>(WRN) << endl;

    SiFiveU_NIC::_device->send(dst_mac, (void*)tem, sizeof(IP::TEM), 0x0800);


}

void ICMP_Manager::set_header(Echo * echo, bool request) {

    unsigned int hsize = 20;
    unsigned int id = 0x1230 + id_send;
    id_send++;

    Reg8 version = 4 << 4;
    Reg8 IHL = (hsize / 4);
    echo->Version_IHL = (version | IHL);
    echo->Type_Service = 0;
    echo->TTL = 64;
    echo->Total_Length = CPU_Common::htons(sizeof(IP::Echo));

    // ICMP 
    echo->Protocol = 1;

    // Fragmentation especific fields (Identification, Flags, Fragment Offset)
    echo->Identification = CPU_Common::htons(id);
    echo->Flags_Offset = 0;


    // ICMP especific fields
    echo->Type = (request ? 8 : 0);
    echo->Code = 0;
    echo->Checksum = 0; // TODO: Refactor
    echo->Identifer = 0;
    echo->Sequence_Number = 0;

}

__END_SYS
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

}

void ICMP_Manager::send_reply(unsigned char * dst_ip, Address dst_mac) {

    db<ICMP_Manager>(WRN) << "ICMP_Manager::send_reply(dst_ip=";
    for (int i=0; i < 3; i++) {db<ICMP_Manager>(WRN) << dst_ip[i] << ".";}
    db<ICMP_Manager>(WRN) << dst_ip[4] << ", dst_mac=" << dst_mac << ")";

    // Setando o header default
    Echo * reply = new Echo;
    set_header(reply, false);

    // Source and Destination IP
    for (int i = 0; i < 4; i++) {
        reply->SRC_ADDR[i] = IP_Manager::_ip_mng->my_ip[i];
        reply->DST_ADDR[i] = dst_ip[i];
    }
    SiFiveU_NIC::_device->send(dst_mac, (void*)reply, sizeof(IP::Header) + sizeof(IP::Echo), 0x0800);

}

void ICMP_Manager::receive(void* icmp_msg, unsigned int size) {
    db<ICMP_Manager>(TRC) << "ICMP_Manager::receive" << endl;
    unsigned char * r = reinterpret_cast<unsigned char *>(icmp_msg);

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


    db<ICMP_Manager>(WRN) << "Enviando Time Exceeded Message(dst_ip=";
    for (int i=0; i < 3; i++) {db<ICMP_Manager>(WRN) << header->SRC_ADDR[i] << ".";}
    db<ICMP_Manager>(WRN) << header->SRC_ADDR[4] << ", dst_mac=" << dst_mac << ")";
 
    TEM *tem = new TEM;

    // Novo header do ICMP
    tem->header(header);

    // Header que do datagrama com erro
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
    echo->Checksum = 0;
    echo->Identifer = 0;
    echo->Sequence_Number = 0;

}

__END_SYS
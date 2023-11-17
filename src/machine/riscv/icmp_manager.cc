#include <machine/riscv/icmp_manager.h>

__BEGIN_SYS

ICMP_Manager* ICMP_Manager::_icmp_mng;

void ICMP_Manager::init() {
    db<ICMP_Manager>(TRC) << "ICMP_Manager::init()"<< endl;
    _icmp_mng = new (SYSTEM) ICMP_Manager();
}

void ICMP_Manager::send(unsigned char* data, unsigned int data_size, unsigned char * dst_ip, Address * dst_mac) {
    db<ICMP_Manager>(TRC) << "ICMP_Manager::IP_send inicio"<< endl;

    unsigned int mtu = 1500;

    Echo * echo = new Echo;
    set_header(echo, true);

    memcpy(echo->data, data, data_size); 

    // Source and Destination IP
    for (int i = 0; i < 4; i++) {
        echo->SRC_ADDR[i] = ARP_Manager::_arp_mng->IP_ADDR[i];
        echo->DST_ADDR[i] = dst_ip[i];
    }

    SiFiveU_NIC::_device->send(*dst_mac, (void*)echo, mtu, 0x0800);


    // Print para verificar o header
    db<ICMP_Manager>(TRC) << "Total_Length original " << CPU_Common::ntohs(echo->Total_Length)<< endl;
    // db<ICMP_Manager>(TRC) << "Total_Length sem o size: " << (CPU_Common::ntohs(echo->Total_Length) * 8) - hsize << endl;
    db<ICMP_Manager>(TRC) << "Identification: " << hex << CPU_Common::htons(echo->Identification) << endl;
    db<ICMP_Manager>(TRC) << "Offset: " << (CPU_Common::htons(echo->Flags_Offset) & IP_Manager::GET_OFFSET)*8 << endl;    
    db<ICMP_Manager>(TRC) << "---------------------"<< endl;
    db<ICMP_Manager>(TRC) << "ICMP_Manager::IP_send fim"<< endl;
}

void ICMP_Manager::receive(void* request) {
 
    
    // Cria um novo ponteiro para adicionar na lista de fragmentos que estão chegando
    char * r = new char[1500];
    memcpy(r, request, 1500);

    // Ethernet frame
    Frame * frame = reinterpret_cast<Frame*>(r);
    Echo * echo = reinterpret_cast<Echo*>(r + 14);
    
    // Capturando os valores do fragmento
    unsigned int length = CPU_Common::ntohs(echo->Total_Length) - 20;
    short unsigned int identification = CPU_Common::ntohs(echo->Identification);
    short unsigned int offset = (CPU_Common::ntohs(echo->Flags_Offset) & GET_OFFSET) * 8;
    // short unsigned int more_frags = (CPU_Common::ntohs(echo->Flags_Offset) & MORE_FRAGS);
    short unsigned int flags = ((CPU_Common::ntohs(echo->Flags_Offset) & GET_FLAGS));

    Echo * reply = new Echo;
    set_header(reply, false);
    db<ICMP_Manager>(WRN) << "Fez o set header" << endl;

    unsigned int mtu = 1500;

    // Source and Destination IP
    for (int i = 0; i < 4; i++) {
        reply->SRC_ADDR[i] = echo->DST_ADDR[i];
        reply->DST_ADDR[i] = echo->SRC_ADDR[i];
    }
    db<ICMP_Manager>(WRN) << "Preencheu o IP" << sizeof(reply) + sizeof(Echo::Header) << endl;
    SiFiveU_NIC::_device->send(frame->dst(), (void*)reply, mtu, 0x0800);


    // Verificação se os valores estão certos
    db<ICMP_Manager>(WRN) << "length: " << hex << length << endl;
    db<ICMP_Manager>(WRN) << "identification: " << hex << identification << endl;
    db<ICMP_Manager>(WRN) << "offset: " << offset << endl;
    db<ICMP_Manager>(WRN) << "flags: " << flags << endl;

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
    echo->Total_Length = 1500;

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
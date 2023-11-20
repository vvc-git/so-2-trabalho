#include <utility/ostream.h>
#include <utility/observer.h>
#include <utility/ct_buffer.h>
#include <machine/riscv/riscv_nic.h>
#include <network/ethernet.h>
#include <time.h>
#include <process.h>


// Para o delay e alarm
#include <time.h>
#include <utility/handler.h>
#include <system/types.h>


// Gerenciadores de rede
#include <machine/riscv/icmp_manager.h>
#include <machine/riscv/arp_manager.h>

using namespace EPOS;

OStream cout;

void test_ip_complete() {

     unsigned int frag_data_size = 1480;

     // IP destino na minha rede e não é o meu 150, 162, 60, 0
     unsigned char ip[4];
     ip[0] = 150;        
     ip[1] = 162;
     ip[2] = 60;
     ip[3] = 2;

     cout << "\nTeste Funcionando" << endl;

     NIC_Common::Address<6> * mac =  IP_Manager::_ip_mng->find_mac(ip);
     if (!mac) {
          cout << "MAC não encontrado: " << endl;
          return;
     } 

     cout << "Iniciando envio de dados IP" << endl;

     unsigned int data_size = 200;
     unsigned char data_second[data_size];
     for(unsigned int i = 0; i < data_size; i++) {
          if (i < frag_data_size) data_second[i] = '3';
          else if (i < frag_data_size*2) data_second[i] = 'D';
          else data_second[i] = 'U';
     }

     IP::Header * header = new IP::Header;
     IP_Manager::default_header(header);

     memcpy(header->SRC_ADDR, IP_Manager::_ip_mng->my_ip, 4);
     memcpy(header->DST_ADDR, ip, 4);

     IP_Manager::_ip_mng->send(header, data_second, data_size, *mac);

}

void test_ip_incomplete() {

     unsigned int frag_data_size = 1480;

     // IP destino na minha rede e não é o meu 150, 162, 60, 0
     unsigned char ip[4];
     ip[0] = 150;        
     ip[1] = 162;
     ip[2] = 60;
     ip[3] = 2;

     cout << "\nTeste pacote IP perdido por Timeout" << endl;

     NIC_Common::Address<6> * mac = IP_Manager::_ip_mng->find_mac(ip);
     if (!mac) {
          cout << "MAC não encontrado: " << endl;
          return;
     }    

     cout << "Iniciando envio de dados IP" << endl;

     // Criação do dado enviado
     unsigned int data_size = 2000;
     unsigned char data[data_size];
     for(unsigned int i = 0; i < data_size; i++) {
          if (i < frag_data_size) data[i] = '3';
          else if (i < frag_data_size*2) data[i] = 'D';
          else data[i] = 'U';
     }

     // Construção do header com parametros default
     IP::Header * header = new IP::Header;
     IP_Manager::default_header(header);

     // Seta o IP origem e destino
     memcpy(header->SRC_ADDR, IP_Manager::_ip_mng->my_ip, 4);
     memcpy(header->DST_ADDR, ip, 4);

     // Cria o datagrama para ser enviado
     unsigned char datagram[sizeof(IP::Header) + data_size];
     memcpy(datagram, header, sizeof(IP::Header));
     memcpy(datagram + sizeof(IP::Header), data, data_size);

     // Realiza a fragmentação
     Simple_List<IP::Fragment> * fragments = IP_Manager::_ip_mng->fragmentation(datagram, data_size);

     // Teste forçando a perda de um fragmento
     Simple_List<IP::Fragment>::Element * e = fragments->head();
     for (; e; e = e->next() ) {
          unsigned int offset = ntohs(e->object()->Flags_Offset) & IP_Manager::GET_OFFSET;
          if (offset == 185) continue;
          SiFiveU_NIC::_device->send(*mac, (void*)e->object(),  ntohs(e->object()->Total_Length), 0x800);
          Delay(1000000);
     }

}


void test_ping() {

     // IP destino fora da minha rede
     unsigned char ip[4];
     ip[0] = 150;        
     ip[1] = 162;
     ip[2] = 60;
     ip[3] = 2;

     cout << "Ping para ";
     cout << static_cast<int>(ip[0]) << ".";
     cout << static_cast<int>(ip[1]) << ".";
     cout << static_cast<int>(ip[2]) << ".";
     cout << static_cast<int>(ip[3]) << endl;


     NIC_Common::Address<6> * mac = IP_Manager::_ip_mng->find_mac(ip);
     if (!mac) {
          cout << "MAC não encontrado: " << endl;
          return;
     }    
     ICMP_Manager::_icmp_mng->send_request(ip, *mac);

}

int main()
{
     SiFiveU_NIC * sifiveu_nic = SiFiveU_NIC::_device;          
     cout << "  MAC: " << sifiveu_nic->address << "\n" << endl;

     // Sender
     if(sifiveu_nic->address[5] % 2 ) {
          cout << "Sender" << endl;

          test_ip_incomplete();
          Delay(1000000);

          test_ip_complete();
          Delay(5000000);

     // Receiver | Router
     } else {
          cout << "Receiver" << endl;
          Delay (100000000000000);
     }

     return 0;
}

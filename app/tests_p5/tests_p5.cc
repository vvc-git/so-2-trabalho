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

     cout << "\nTeste rede local" << endl;

     // Envio para host na mesma rede local
     cout << "Quem tem o mac do IP  " << static_cast<int>(ip[0]) << ".";
     cout << static_cast<int>(ip[1]) << ".";
     cout << static_cast<int>(ip[2]) << ".";
     cout << static_cast<int>(ip[3]) << "?" <<endl;

     NIC_Common::Address<6> * mac =  IP_Manager::_ip_mng->find_mac(ip);
     if (mac) cout << "MAC encontrado: " << *mac << endl;

     cout << "Iniciando envio de dados IP" << endl;

     unsigned int data_size = 2000;
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

     cout << "IP DST " << static_cast<int>(header->DST_ADDR[0]) << ".";
     cout << static_cast<int>(header->DST_ADDR[1]) << ".";
     cout << static_cast<int>(header->DST_ADDR[2]) << ".";
     cout << static_cast<int>(header->DST_ADDR[3]) << "?" << endl;

     cout << "IP SRC " << static_cast<int>(header->SRC_ADDR[0]) << ".";
     cout << static_cast<int>(header->SRC_ADDR[1]) << ".";
     cout << static_cast<int>(header->SRC_ADDR[2]) << ".";
     cout << static_cast<int>(header->SRC_ADDR[3]) << "?" << endl;

     db<Network_buffer>(WRN) << "Datagrama enviado: " << data_second << endl;
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

     cout << "\nTeste rede local" << endl;

     // Envio para host na mesma rede local
     cout << "Quem tem o mac do IP  " << static_cast<int>(ip[0]) << ".";
     cout << static_cast<int>(ip[1]) << ".";
     cout << static_cast<int>(ip[2]) << ".";
     cout << static_cast<int>(ip[3]) << "?" <<endl;

     NIC_Common::Address<6> * mac =  IP_Manager::_ip_mng->find_mac(ip);
     if (mac) cout << "MAC encontrado: " << *mac << endl;

     cout << "Iniciando envio de dados IP" << endl;

     unsigned int data_size = 2000;
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

     cout << "IP DST " << static_cast<int>(header->DST_ADDR[0]) << ".";
     cout << static_cast<int>(header->DST_ADDR[1]) << ".";
     cout << static_cast<int>(header->DST_ADDR[2]) << ".";
     cout << static_cast<int>(header->DST_ADDR[3]) << "?" << endl;

     cout << "IP SRC " << static_cast<int>(header->SRC_ADDR[0]) << ".";
     cout << static_cast<int>(header->SRC_ADDR[1]) << ".";
     cout << static_cast<int>(header->SRC_ADDR[2]) << ".";
     cout << static_cast<int>(header->SRC_ADDR[3]) << "?" << endl;

     db<Network_buffer>(WRN) << "Datagrama enviado: " << data_second << endl;
     IP_Manager::_ip_mng->send(header, data_second, data_size, *mac);

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

          test_ping();
          Delay(5000000);

     // Receiver | Router
     } else {
          cout << "Receiver" << endl;
          Delay (100000000000000);
     }

     return 0;
}

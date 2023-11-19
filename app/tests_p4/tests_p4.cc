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

// void test_localhost() {

//      unsigned char ip[4];
//      ip[0] = 127; // 127.0.0.1     
//      ip[1] = 0;
//      ip[2] = 0;
//      ip[3] = 1;

//      cout << "\nTeste Localhost" << endl;

//      // Envio para localhost
//      cout << "Quem tem o mac do IP  " << static_cast<int>(ip[0]) << ".";
//      cout << static_cast<int>(ip[1]) << ".";
//      cout << static_cast<int>(ip[2]) << ".";
//      cout << static_cast<int>(ip[3]) << "?" <<endl;

//      NIC_Common::Address<6> * mac =  IP_Manager::_ip_mng->find_mac(ip);
//      if (mac)
//           cout << "MAC encontrado: " << *mac << endl;

//      cout << "Iniciando envio de dados IP" << endl;

//      unsigned int data_size = 1480;
//      unsigned char data_first[data_size];
//      for(unsigned int i = 0; i < data_size; i++) {
//           data_first[i] = '3';
//      }
// // 
//      // IP_Manager::_ip_mng->send(data_first, data_size, *mac);

// }

void test_same_network() {

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

void test_external_network() {

     // IP destino fora da minha rede
     unsigned char ip[4];
     ip[0] = 99;      
     ip[1] = 0;
     ip[2] = 0;
     ip[3] = 1;

     cout << "\nTeste rede externa" << endl;

     // Envio para host na rede externa (E não temos na tabela de roteamento -> vai para default)
     cout << "Quem tem o mac do IP  " << static_cast<int>(ip[0]) << ".";
     cout << static_cast<int>(ip[1]) << ".";
     cout << static_cast<int>(ip[2]) << ".";
     cout << static_cast<int>(ip[3]) << "?" << endl;


     NIC_Common::Address<6> * mac = IP_Manager::_ip_mng->find_mac(ip);
     if (mac)
          cout << "MAC do gateway default: " << *mac << endl;

     cout << "Iniciando envio de dados IP" << endl;
     
     unsigned int frag_data_size = 1480;
     unsigned int data_size = 2000;
     unsigned char data_third[data_size];
     for(unsigned int i = 0; i < data_size; i++) {
          if (i < frag_data_size) data_third[i] = 'A';
          else if (i < frag_data_size * 2) data_third[i] = 'B';
          else data_third[i] = 'C';
     }

     IP::Header * header = new IP::Header;
     IP_Manager::default_header(header);

     memcpy(header->SRC_ADDR, IP_Manager::_ip_mng->my_ip, 4);
     memcpy(header->DST_ADDR, ip, 4);

     cout << "IP DST " << static_cast<int>(header->DST_ADDR[0]) << ".";
     cout << static_cast<int>(header->DST_ADDR[1]) << ".";
     cout << static_cast<int>(header->DST_ADDR[2]) << ".";
     cout << static_cast<int>(header->DST_ADDR[3]) << "" << endl;

     cout << "IP SRC " << static_cast<int>(header->SRC_ADDR[0]) << ".";
     cout << static_cast<int>(header->SRC_ADDR[1]) << ".";
     cout << static_cast<int>(header->SRC_ADDR[2]) << ".";
     cout << static_cast<int>(header->SRC_ADDR[3]) << "" << endl;

     db<Network_buffer>(WRN) << "Datagrama enviado: " << data_third << endl;
     IP_Manager::_ip_mng->send(header, data_third, data_size, *mac);
     
}


void test_icmp() {

     // IP destino fora da minha rede
     unsigned char ip[4];
     ip[0] = 150;        
     ip[1] = 162;
     ip[2] = 60;
     ip[3] = 2;

     cout << "\nTeste rede externa" << endl;

     // Envio para host na rede externa (E não temos na tabela de roteamento -> vai para default)
     cout << "Quem tem o mac do IP  " << static_cast<int>(ip[0]) << ".";
     cout << static_cast<int>(ip[1]) << ".";
     cout << static_cast<int>(ip[2]) << ".";
     cout << static_cast<int>(ip[3]) << "?" << endl;


     NIC_Common::Address<6> * mac = IP_Manager::_ip_mng->find_mac(ip);
     if (mac)
          cout << "MAC do gateway default: " << *mac << endl;

     cout << "Iniciando envio de dados IP - ICMP" << endl;
     
     // db<Network_buffer>(WRN) << "Datagrama enviado: " << data_second << endl;
     ICMP_Manager::_icmp_mng->send_request(ip, *mac);

}

void test_default_header() {

     IP::Header * header = new IP::Header;
     IP_Manager::default_header(header);


     // cout << "Version e header length" << header->Version_IHL << endl;
     // cout << "Type Service" << header->Type_Service << endl;
     // cout << "TTL" << header->TTL << endl;
     // cout << "Protocol" << header->Protocol << endl;


}
struct Teste {
     char a;
     char b;
};

int main()
{
     SiFiveU_NIC * sifiveu_nic = SiFiveU_NIC::_device;          
     cout << "  MAC: " << sifiveu_nic->address << "\n" << endl;

     TSC_Chronometer *chrono = new TSC_Chronometer;
     chrono->start();
     Delay(1000000);
     chrono->stop();

     cout << chrono->read() / 1000000 << ",";
     cout << chrono->read() % 1000000 << endl;


     // Sender
     if(sifiveu_nic->address[5] % 2 ) {
          cout << "Sender" << endl;
          // Alarm *alarm1 = new Alarm(time, &functor, 1); // Dá pra colocar um número diretamente em time
          // Delay(2000000);
          // cout << "Passaram dois segundos" << endl;
          // alarm1->reset();
          // Delay(2000000);
          // cout << "Passaram dois segundos" << endl;
          // alarm1->reset();
          // Delay(2000000);
          // cout << "Passaram dois segundos" << endl;
          // delete alarm1;

          // test_localhost();
          // Delay(5000000);

          // test_same_network();     
          // test_icmp();
          // Delay(5000000);

          // test_same_network();     
          // test_icmp();

          // // test_default_header();
          // test_same_network();
          // Delay(5000000);

          test_same_network();
          Delay(5000000);
     

          // test_external_network();
          // Delay(10000000000);

     // Receiver | Router
     } else {
          cout << "Receiver" << endl;

          Delay (100000000000000);
     }

     return 0;
}

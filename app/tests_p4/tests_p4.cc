#include <utility/ostream.h>
#include <utility/observer.h>
#include <utility/ct_buffer.h>
#include <machine/riscv/riscv_nic.h>
#include <network/ethernet.h>
#include <time.h>
#include <process.h>
#include <machine/riscv/arp_manager.h>

// Para o delay
#include <time.h>


using namespace EPOS;

OStream cout;

int main()
{
     SiFiveU_NIC * sifiveu_nic = SiFiveU_NIC::_device;

     NIC<Ethernet>::Address dst;

     dst[0] = 0x00;
     dst[1] = 0x00;
     dst[2] = 0x00;
     dst[3] = 0x00;
     dst[4] = 0x00;
     dst[5] = 0x02;

     unsigned char ip1[4];
     // unsigned char ip4[4];
     unsigned char ip2[4];
     unsigned char ip3[4];

     // IP destino fora da minha rede ()
     ip1[0] = 99; // 127.0.60.2       
     ip1[1] = 0;
     ip1[2] = 0;
     ip1[3] = 1;

     // IP destino fora da minha rede ()
     // ip4[0] = 210;
     // ip4[1] = 154;
     // ip4[2] = 80;
     // ip4[3] = 0;

     // IP destino na minha rede e não é o meu 150, 162, 60, 0
     ip2[0] = 150;        
     ip2[1] = 162;
     ip2[2] = 60;
     ip2[3] = 2;

     // IP destino na minha rede e sou eu mesmo 
     ip3[0] = 127; // 127.0.0.1     
     ip3[1] = 0;
     ip3[2] = 0;
     ip3[3] = 1;

          
     cout << "  MAC: " << sifiveu_nic->address << "\n" << endl;
     NIC_Common::Address<6> * mac;
     unsigned int frag_data_size = 1480;

     // unsigned int frag_data_size = 1480;
     // unsigned int data_size = 1480;
     if(sifiveu_nic->address[5] % 2 ) {
          
          // Envio para localhost
          cout << "Quem tem o mac do IP  " << static_cast<int>(ip3[0]) << ".";
          cout << static_cast<int>(ip3[1]) << ".";
          cout << static_cast<int>(ip3[2]) << ".";
          cout << static_cast<int>(ip3[3]) << "?" <<endl;

          mac =  Network_buffer::net_buffer->IP_find_mac(ip3);
          if (mac)
               cout << "MAC encontrado: " << *mac << "\n" << endl;

          cout << "Iniciando envio de dados IP\n" << endl;

          unsigned int data_size = 1480;
          char data_first[data_size];
          for(unsigned int i = 0; i < data_size; i++) {
               data_first[i] = '3';
          }

          Network_buffer::net_buffer->IP_send(data_first, data_size, ip3, mac);
          Delay(5000000);

          // Envio para host na mesma rede local
          cout << "Quem tem o mac do IP  " << static_cast<int>(ip2[0]) << ".";
          cout << static_cast<int>(ip2[1]) << ".";
          cout << static_cast<int>(ip2[2]) << ".";
          cout << static_cast<int>(ip2[3]) << "?" <<endl;

          mac =  Network_buffer::net_buffer->IP_find_mac(ip2);
          if (mac) cout << "MAC encontrado: " << *mac << "\n" << endl;

          data_size = 4020;
          char data_second[data_size];
          for(unsigned int i = 0; i < data_size; i++) {
              if (i < frag_data_size) data_second[i] = '3';
              else if (i < frag_data_size*2) data_second[i] = 'D';
              else data_second[i] = 'U';
          }

          // db<Network_buffer>(WRN) << "Datagrama enviado: " << data_second << endl;
          Network_buffer::net_buffer->IP_send(data_second, data_size, ip2, mac);
          Delay(5000000);

          // Envio para host na rede externa (E não temos na tabela de roteamento -> vai para default)
          cout << "Quem tem o mac do IP  " << static_cast<int>(ip1[0]) << ".";
          cout << static_cast<int>(ip1[1]) << ".";
          cout << static_cast<int>(ip1[2]) << ".";
          cout << static_cast<int>(ip1[3]) << "?" << endl;

          mac = Network_buffer::net_buffer->IP_find_mac(ip1);
          if (mac)
               cout << "MAC encontrado: " << *mac << "\n" << endl;

          data_size = 2000;
          char data_third[data_size];
          for(unsigned int i = 0; i < data_size; i++) {
              if (i < frag_data_size) data_third[i] = 'A';
              else if (i < frag_data_size*2) data_third[i] = 'B';
              else data_third[i] = 'C';
          }

          // db<Network_buffer>(WRN) << "Datagrama enviado: " << data_second << endl;
          Network_buffer::net_buffer->IP_send(data_third, data_size, ip1, mac);
          // Delay(5000000);

          Delay(10000000000);

     } else {
          
          Delay (100000000000000);
     }

     return 0;
}

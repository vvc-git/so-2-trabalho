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
     // unsigned char ip2[4];
     // unsigned char ip3[4];

     // // IP destino fora da minha rede
     ip1[0] = 127; // 127.0.60.2       
     ip1[1] = 0;
     ip1[2] = 60;
     ip1[3] = 2;

     // IP destino na minha rede e não é o meu 150, 162, 60, 0
     // ip2[0] = 150; // 127.0.0.2       
     // ip2[1] = 162;
     // ip2[2] = 60;
     // ip2[3] = 2;

     // IP destino na minha rede e sou eu mesmo 
     // ip3[0] = 127; // 127.0.0.1     
     // ip3[1] = 0;
     // ip3[2] = 0;
     // ip3[3] = 1;

          
     cout << "  MAC: " << sifiveu_nic->address << "\n" << endl;

     // unsigned int frag_data_size = 1480;
     // unsigned int data_size = 1480;
     if(sifiveu_nic->address[5] % 2 ) {

          Network_buffer::net_buffer->IP_routing(ip1);

          // ARP_Manager * arp_mng = ARP_Manager::_arp_mng;
          // cout << "Caso ignorado " << endl;
          // arp_mng->arp_send_request(ip2);
          
          // // cout << "Caso 1: IP destino fora da minha rede (não faz nada ainda) "<< endl;
          // // arp_mng->arp_send_request(ip1);

          // cout << "Caso 2:  IP destino na minha rede e não é o meu (e NÃO tem na tabela) "<< endl;
          // arp_mng->arp_send_request(ip2);

          // Delay(5000000);

          // cout << "Caso 2.1:  IP destino na minha rede e não é o meu ( tem na tabela) "<< endl;
          // arp_mng->arp_send_request(ip2);


          // cout << "  Caso 3: IP destino na minha rede e sou eu mesmo (e tem na tabela) " << "\n" << endl;
          // arp_mng->arp_send_request(ip3);
          
          Delay(10000000000);

          
     //      char data_first[data_size];
     //      for(unsigned int i = 0; i < data_size; i++) {
     //           data_first[i] = '3';
     //      }
     //      Network_buffer::net_buffer->IP_send(data_first, data_size);

     //      data_size = 1480;
     //      char data_second[data_size];
     //      for(unsigned int i = 0; i < data_size; i++) {
     //           if (i < frag_data_size) data_second[i] = '3';
     //           else if (i < frag_data_size*2) data_second[i] = 'D';
     //           else data_second[i] = 'U';
     //      }
     //      Network_buffer::net_buffer->IP_send(data_second, data_size);

     //      data_size = 4020;
     //      char data_third[data_size];
     //      for(unsigned int i = 0; i < data_size; i++) {
     //           if (i < frag_data_size) data_third[i] = '3';
     //           else if (i < frag_data_size*2) data_third[i] = 'D';
     //           else data_third[i] = 'U';
     //      }
     //      Network_buffer::net_buffer->IP_send(data_third, data_size);
     } else {
          
          Delay (100000000000000);
     }

     return 0;
}

#include <utility/ostream.h>
#include <utility/observer.h>
#include <utility/ct_buffer.h>
#include <machine/riscv/riscv_nic.h>
#include <network/ethernet.h>
#include <time.h>
#include <process.h>

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
     

     cout << "IP sending" << endl;
     cout << "  MAC: " << sifiveu_nic->address << endl;

     unsigned int frag_size = 1452;
     unsigned int data_size = 3000;
     //unsigned int iter = data_size/frag_size;
     //unsigned int last = data_size%frag_size;
     if(sifiveu_nic->address[5] % 2 ) {

          char data[data_size];
          for(unsigned int i = 0; i < data_size; i++) {
               if (i < frag_size) data[i] = '0';
               else if (i < frag_size*2) data[i] = '1';
               else data[i] = '2';
          }

          Network_buffer::net_buffer->IP_send(data, data_size);
     } else {
          Delay (100000000000000);
     }



     // print pra conferir o data
     // for (unsigned int i=0; i<data_size; i++) {
     //      cout << data[i];
     // }
     // cout << endl;

     // if((sifiveu_nic->address[5] % 2 )) { // sender
     //      cout << "Sender" << endl;
     //      for(int i = 0; i < 10; i++) {
     //           cout << "Sender " << i << endl;
     //           memset(payload, '0' + i, MTU);
     //           sifiveu_nic->send(dst, payload, MTU);
     //           Delay (50000);
     //      }
     // } else {
     //      cout << "Receiver" << endl;
     //      Delay (10000000000000000);
     //      cout << "Receiver" << endl;
     // }

     

     return 0;
}

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

     cout << "  MAC: " << sifiveu_nic->address << endl;

     unsigned int frag_data_size = 1480;
     unsigned int data_size = 3000;

     char data1[1480];
     char data2[data_size];

     if(sifiveu_nic->address[5] % 2 ) {
          
          // Datagrama 1
          for(unsigned int i = 0; i < 1480; i++) {
               data1[i] = 'A';
          }

          // Datagrama 2
          for(unsigned int i = 0; i < data_size; i++) {
               if (i < frag_data_size) data2[i] = 'B';
               else if (i < frag_data_size*2) data2[i] = 'C';
               else data2[i] = 'D';
          }

          Network_buffer::net_buffer->IP_send(data1, 1480);
          Network_buffer::net_buffer->IP_send(data2, data_size);

     } else {
          
          Delay (100000000000000);
     }

     return 0;
}

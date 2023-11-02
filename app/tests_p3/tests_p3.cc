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

     cout << "  MAC: " << sifiveu_nic->address << "\n" << endl;

     unsigned int frag_data_size = 1480;
     unsigned int data_size = 1480;
     if(sifiveu_nic->address[5] % 2 ) {
          
          char data_first[data_size];
          for(unsigned int i = 0; i < data_size; i++) {
               data_first[i] = '3';
          }
          Network_buffer::net_buffer->IP_send(data_first, data_size);

          data_size = 1480;
          char data_second[data_size];
          for(unsigned int i = 0; i < data_size; i++) {
               if (i < frag_data_size) data_second[i] = '3';
               else if (i < frag_data_size*2) data_second[i] = 'D';
               else data_second[i] = 'U';
          }
          Network_buffer::net_buffer->IP_send(data_second, data_size);

          data_size = 4020;
          char data_third[data_size];
          for(unsigned int i = 0; i < data_size; i++) {
               if (i < frag_data_size) data_third[i] = '3';
               else if (i < frag_data_size*2) data_third[i] = 'D';
               else data_third[i] = 'U';
          }
          Network_buffer::net_buffer->IP_send(data_third, data_size);
     } else {
          
          Delay (100000000000000);
     }

     return 0;
}

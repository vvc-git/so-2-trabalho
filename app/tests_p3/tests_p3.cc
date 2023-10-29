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

     // Primeiro frame estragado
     char data_nic[1500];
     for(unsigned int i = 0; i < 1500; i++) {
          data_nic[i] = '0';
     }
     SiFiveU_NIC::_device->send(dst, data_nic, 1500);
     Delay (1000000);
     cout << "  MAC: " << sifiveu_nic->address << endl;

     unsigned int frag_data_size = 1480;
     unsigned int data_size = 3000;
     //unsigned int iter = data_size/frag_data_size;
     //unsigned int last = data_size%frag_data_size;
     if(sifiveu_nic->address[5] % 2 ) {
          
          char data[data_size];
          for(unsigned int i = 0; i < data_size; i++) {
               if (i < frag_data_size) data[i] = '3';
               else if (i < frag_data_size*2) data[i] = 'D';
               else data[i] = 'U';
          }

          Network_buffer::net_buffer->IP_send(data, data_size);
          Network_buffer::net_buffer->IP_send(data, data_size);
     } else {
          
          Delay (100000000000000);
     }

     return 0;
}

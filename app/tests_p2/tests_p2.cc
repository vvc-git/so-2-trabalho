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
     

     unsigned int MTU = 1500;
     char payload[MTU];
     cout << "  MAC: " << sifiveu_nic->address << endl;

     if((sifiveu_nic->address[5] % 2 )) { // sender
          cout << "Sender" << endl;
          for(int i = 0; i < 10; i++) {
               cout << "Sender " << i << endl;
               memset(payload, '0' + i, MTU);
               sifiveu_nic->send(dst, payload, MTU);
               Delay (50000);
          }
     } else {
          cout << "Receiver" << endl;
          Delay (10000000000000000);
          cout << "Receiver" << endl;
     }

     

     return 0;
}

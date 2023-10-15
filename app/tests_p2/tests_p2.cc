#include <utility/ostream.h>
#include <utility/observer.h>
#include <utility/ct_buffer.h>
#include <machine/riscv/riscv_nic.h>
#include <network/ethernet.h>
#include <time.h>

// Para o delay
#include <time.h>


using namespace EPOS;

OStream cout;

// class NIC_Receiver : public Data_Observer<CT_Buffer, void>
// {
// public:
//      NIC_Receiver(){};
//      ~NIC_Receiver(){};
//      void update(Data_Observed<CT_Buffer, void> *obs, CT_Buffer *buffer)
//      {
//           char data[FRAME_SIZE];
//           buffer->get_dma_data(data);

//           cout << "NIC_Receiver update: " << endl;
//           for (int i = 0; i < 1500; i++) {
//                cout << data[i];
//           }
//           cout << endl;
//      }
// };


int main()
{
     SiFiveU_NIC * sifiveu_nic = SiFiveU_NIC::_device;
     // NIC_Receiver nic_receiver = NIC_Receiver();

     // sifiveu_nic->attach(&nic_receiver);

     NIC<Ethernet>::Address src, dst;
     
     src[0] = 0x01;
     src[1] = 0x00;
     src[2] = 0x00;
     src[3] = 0x00;
     src[4] = 0x00;
     src[5] = 0x00;

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
               sifiveu_nic->send(src, dst, payload, MTU);
               Delay (50000);
          }
     } else {
          Delay (10000000000000000);
          cout << "Receiver" << endl;
     }

     return 0;
}

#include <utility/ostream.h>
#include <utility/observer.h>
#include <utility/ct_buffer.h>
#include <machine/riscv/riscv_nic.h>
#include <network/ethernet.h>


using namespace EPOS;

OStream cout;

class NIC_Receiver : public Data_Observer<CT_Buffer, void>
{
public:
     NIC_Receiver(){};
     ~NIC_Receiver(){};
     void update(Data_Observed<CT_Buffer, void> *obs, CT_Buffer *buffer)
     {
          char data[FRAME_SIZE];
          buffer->get_dma_data(data);

          cout << "NIC_Receiver update: " << endl;
          cout << data[0] << " " << data[1] << " " << data[2] << " " << data[3] << endl;

     }
};


int main()
{

     SiFiveU_NIC sifiveu_nic = SiFiveU_NIC();
     NIC_Receiver nic_receiver = NIC_Receiver();

     sifiveu_nic.attach(&nic_receiver);

     NIC<Ethernet>::Address src, dst;

     // src[5] = 0x00;
     // src[4] = 0x00;
     // src[3] = 0x00;
     // src[2] = 0x00;
     // src[1] = 0x00;
     // src[0] = 0x01;

     dst[0] = 0xff;
     dst[1] = 0xff;
     dst[2] = 0xff;
     dst[3] = 0xff;
     dst[4] = 0xff;
     dst[5] = 0xff;


     char payload[100];
     cout << "  MAC: " << sifiveu_nic.address << endl;

     if(!(sifiveu_nic.address[5] % 2 )) { // sender
          cout << "Sender" << endl;
          for(int i = 0; i < 1; i++) {
               memset(payload, '0' + i, 100);
               payload[100 - 1] = '\n';
               cout << "  dest: " << dst << endl;
               sifiveu_nic.send(src, dst, payload, 100);
          }
     } else {
          cout << "Receiver" << endl;
          sifiveu_nic.receive(src, payload, 100);
     }


//     } else { // receiver
//         for(int i = 0; i < 10; i++) {
//            nic->receive(&src, &prot, data, nic->mtu());
//            cout << "  Data: " << data;
//         }
//     }



     return 0;
}

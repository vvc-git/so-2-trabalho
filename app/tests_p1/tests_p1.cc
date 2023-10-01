#include <utility/ostream.h>
#include <utility/observer.h>
#include <utility/ct_buffer.h>
#include <machine/riscv/riscv_nic.h>
#include <machine/riscv/riscv_gem.h>
#include <network/ethernet.h>


using namespace EPOS;

// OStream cout;

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
     cout << "\n\n ********************  P1 *********************\n\n"
          << endl;


     SiFiveU_NIC sifiveu_nic = SiFiveU_NIC();
     NIC_Receiver nic_receiver = NIC_Receiver();

     sifiveu_nic.attach(&nic_receiver);

     NIC<Ethernet>::Address src, dst;

     src[5] = 0x01;
     src[4] = 0x00;
     src[3] = 0x00;
     src[2] = 0x00;
     src[1] = 0x00;
     src[0] = 0x00;

     dst[5] = 0x02;
     dst[4] = 0x00;
     dst[3] = 0x00;
     dst[2] = 0x00;
     dst[1] = 0x00;
     dst[0] = 0x00;


     char payload[100];
     for(int i = 0; i < 20; i++) {
          memset(payload, '0' + i, 100);
          payload[100 - 1] = '\n';
          sifiveu_nic.send(src, dst, payload, 100);
     }

     cout << "\n\n ************************* P1 *************************\n\n"
          << endl;

     return 0;
}

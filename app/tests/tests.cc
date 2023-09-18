#include <utility/ostream.h>
#include <utility/observer.h>
#include <utility/ct_buffer.h>
#include <utility/ct_buffer.h>
#include <machine/riscv/riscv_nic.h>

using namespace EPOS;

// OStream cout;

class NIC_Receiver : public Data_Observer<CT_Buffer, void>
{
public:
     NIC_Receiver(){};
     ~NIC_Receiver(){};
     void update(Data_Observed<CT_Buffer, void> *obs, CT_Buffer *buffer)
     {
          char data[1500];
          buffer->get_dma_data(data);

          cout << "NIC_Receiver update: " << endl;
          cout << data[0] << " " << data[1] << " " << data[2] << " " << data[3] << endl;

          //  ! verificar aqui depois
          //  buffer->nic()->free(buffer); // to return to the buffer pool;
     }
};

int main()
{
     cout << "\n\n ******************** TESTANDO NIC *********************\n\n"
          << endl;

     SiFiveU_NIC sifiveu_nic = SiFiveU_NIC();
     NIC_Receiver nic_receiver = NIC_Receiver();

     sifiveu_nic.attach(&nic_receiver);

     sifiveu_nic.int_handler();

     cout << "\n\n ************************* FIM *************************\n\n"
          << endl;

     return 0;
}

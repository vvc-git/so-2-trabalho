#include <utility/ostream.h>
#include <utility/observer.h>
#include <utility/ct_buffer.h>
#include <machine/riscv/riscv_nic.h>
#include <machine/riscv/riscv_gem.h>

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

void copy_rx_tx(SiFiveU_NIC *nic)
{

     memcpy(nic->rx_data_phy, nic->tx_data_phy, FRAME_SIZE);
}

int main()
{
     cout << "\n\n ********************  NIC *********************\n\n"
          << endl;


     SiFiveU_NIC sifiveu_nic = SiFiveU_NIC();
     NIC_Receiver nic_receiver = NIC_Receiver();

     sifiveu_nic.attach(&nic_receiver);

     // TESTE
     char data[FRAME_SIZE];
     data[0] = 'a';
     data[1] = 'b';
     data[2] = 'c';
     data[3] = 'd';

     sifiveu_nic.send(data, FRAME_SIZE);

     copy_rx_tx(&sifiveu_nic);

     sifiveu_nic.int_handler();

     cout << "\n\n ************************* NIC *************************\n\n"
          << endl;

     return 0;
}
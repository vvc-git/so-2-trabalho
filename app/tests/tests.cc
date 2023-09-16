#include <utility/ostream.h>
#include <machine/riscv/riscv_nic.h>

using namespace EPOS;

OStream cout;

int main()
{
    cout << "\n\n ************** TESTANDO NIC **************\n\n" << endl;

    Cadence_NIC net = Cadence_NIC();
    
    cout << "\nEndereço físico do buffer descritor: " << net.phy_init_tx_desc << "\n";
    cout << "\nEndereço físico do buffer de dados: " << net.phy_init_tx_data << "\n";

    cout << "\n\n ******************* FIM *******************\n\n" << endl;

    return 0;
}

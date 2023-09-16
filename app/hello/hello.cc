#include <utility/ostream.h>

#include <machine/riscv/riscv_cadence.h>
#include <utility/string.h>
#include <machine/riscv/sifive_u/sifive_u_memory_map.h>

using namespace EPOS;

// OStream cout;

int main()
{
    // cout << "Hello world!" << endl;
    cout << "\n\n ************ TESTANDO CADENCE ************\n\n" << endl;

    Cadence net = Cadence();
    int * addr = reinterpret_cast<int *>(Memory_Map::ETH_BASE + 0x18);

    cout << &net << endl;
    cout << &addr << endl;

    cout << "\n\n ******************* FIM *******************\n\n" << endl;

    return 0;
}

#include <utility/ostream.h>

#include <machine/riscv/riscv_cadence.h>

using namespace EPOS;

// OStream cout;

int main()
{
    // cout << "Hello world!" << endl;
    cout << "\n\n ************ TESTANDO CADENCE ************\n\n" << endl;

    Cadence net = Cadence();
    cout << "Ponteiro para o objeto Cadence: " << &net << "\n";

    cout << "\n\n ******************* FIM *******************\n\n" << endl;

    return 0;
}

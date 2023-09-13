#include <utility/ostream.h>

using namespace EPOS;

OStream cout;

int main()
{
    cout << "\n\n ************ TESTANDO CADENCE ************\n\n" << endl;

    Cadence net = Cadence();
    cout << &net << "\n";

    cout << "\n\n ******************* FIM *******************\n\n" << endl;

    return 0;
}

#include <utility/ostream.h>

#include <system/memory_map.h>
#include <architecture/cpu.h>

using namespace EPOS;

OStream cout;

inline void print_register(const long & base, const long & offset) {
    CPU::Reg32 *v = reinterpret_cast<CPU::Reg32 *>(base + offset);
    cout << v << " => " << "value=" << *v << endl;
}

int main()
{
    cout << "Hello world!" << endl;

    unsigned long base = 0x16040000;
    
    for (int i = 0; i < 12; i++) {
        print_register(base, i * 4);
    }

    return 0;
}

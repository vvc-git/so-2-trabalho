#include <utility/ostream.h>

#include <system/memory_map.h>
#include <architecture/cpu.h>
#include <machine/nic.h>

using namespace EPOS;

OStream cout;

inline void print_register(const long & base, const long & offset) {
    CPU::Reg32 *v = reinterpret_cast<CPU::Reg32 *>(base + offset);
    cout << v << " => " << "value=" << *v << endl;
}

int main()
{
    cout << "Hello world!" << endl;

    print_register(Memory_Map::ETH_BASE, 0x004);

    return 0;
}

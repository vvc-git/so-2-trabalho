#include <utility/ostream.h>

#include <system/memory_map.h>
#include <architecture/cpu.h>
#include <machine/nic.h>

using namespace EPOS;

OStream cout;

int main()
{
    cout << "Hello world!" << endl;
    //Ideia here is to create a pointer to any register mapped into the sifive network interface card, for that we must use the register offset
    CPU::Reg32 *register_value = (CPU::Reg32 *) Memory_Map::NIC_BASE + GEM::NWCFG;
    CPU::Reg32 *register_value2 = (CPU::Reg32 *) Memory_Map::NIC_BASE + GEM::DMACR;

    cout << "Network config: " << *register_value << "Network DMA config" << *register_value2 << endl;

    return 0;
}

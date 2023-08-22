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


    print_register(Memory_Map::MAC_DMA_BASE, 0x1000);
    print_register(Memory_Map::MAC_DMA_BASE, 0x1004);
    print_register(Memory_Map::MAC_DMA_BASE, 0x1008);
    print_register(Memory_Map::MAC_DMA_BASE, 0x100c);
    print_register(Memory_Map::MAC_DMA_BASE, 0x1010);
    print_register(Memory_Map::MAC_DMA_BASE, 0x1014);
    print_register(Memory_Map::MAC_DMA_BASE, 0x1018);
    print_register(Memory_Map::MAC_DMA_BASE, 0x101c);
    print_register(Memory_Map::MAC_DMA_BASE, 0x1020);
    print_register(Memory_Map::MAC_DMA_BASE, 0x1048);
    print_register(Memory_Map::MAC_DMA_BASE, 0x104c);
    print_register(Memory_Map::MAC_DMA_BASE, 0x1050);
    print_register(Memory_Map::MAC_DMA_BASE, 0x1054);

    print_register(Memory_Map::GMAC0_BASE, 0x0000);
    print_register(Memory_Map::GMAC0_BASE, 0x0004);
    print_register(Memory_Map::GMAC0_BASE, 0x0008);
    print_register(Memory_Map::GMAC0_BASE, 0x000c);
    print_register(Memory_Map::GMAC0_BASE, 0x0010);
    print_register(Memory_Map::GMAC0_BASE, 0x0014);
    print_register(Memory_Map::GMAC0_BASE, 0x0018);
    print_register(Memory_Map::GMAC0_BASE, 0x001c);
    print_register(Memory_Map::GMAC0_BASE, 0x0020);
    print_register(Memory_Map::GMAC0_BASE, 0x0028);
    print_register(Memory_Map::GMAC0_BASE, 0x002c);
    print_register(Memory_Map::GMAC0_BASE, 0x0038);
    print_register(Memory_Map::GMAC0_BASE, 0x003C);
    print_register(Memory_Map::GMAC0_BASE, 0x0040);
    print_register(Memory_Map::GMAC0_BASE, 0x0044);

    return 0;
}

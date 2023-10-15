// EPOS Buffer Declarations

#ifndef __CT_Buffer_h
#define __CT_Buffer_h
#define FRAME_SIZE 1600

#include <utility/list.h>
#include <system.h>
#include <utility/heap.h>
#include <utility/string.h>
#include <architecture/mmu.h>
#include <synchronizer.h>
#include <architecture/cpu.h>
#include <network/ethernet.h>

__BEGIN_UTIL

// OStream cout;


// Buffer designed to control acess of frames  

class CT_Buffer
{
public:
    typedef CPU::Phy_Addr Phy_Addr;
    typedef CPU::Log_Addr Log_Addr;
    typedef Ethernet::Frame Frame;
    typedef No_MMU::DMA_Buffer DMA;

    CT_Buffer(long unsigned bytes): size(bytes / FRAME_SIZE) { dma = new DMA(bytes); base = dma->log_address();};
    ~CT_Buffer(){};
    void get_data_frame(char *data);
    void save_data_frame(char *data);
    Phy_Addr phy_address() { return dma->phy_address(); };
    Log_Addr log_address() { return dma->log_address(); };

public:

    // Structures availables continous allocation
    DMA* dma;

    // First address 
    char *base;

    // index of next empty space
    int next = 0;

    // number of slots 
    int size;
};

__END_UTIL

#endif

// EPOS Buffer Declarations

#ifndef __CT_Buffer_h
#define __CT_Buffer_h
#define FRAME_SIZE 1536

#include <utility/list.h>
#include <system.h>
#include <utility/heap.h>
#include <utility/string.h>
#include <architecture/mmu.h>
#include <synchronizer.h>
#include <architecture/cpu.h>

__BEGIN_UTIL

// OStream cout;

class CT_Buffer
{
public:
    typedef CPU::Phy_Addr Phy_Addr;
    typedef CPU::Log_Addr Log_Addr;

    CT_Buffer(unsigned long bytes) : dma(bytes), buffer_size(bytes) { dma_ptr = dma.log_address(); };
    ~CT_Buffer(){};
    void alloc_frame(char frame[]);
    void get_dma_data(char *dma_data);
    void set_dma_data(char *dma_data, int amnt_frames);
    unsigned int allocated();
    Phy_Addr phy_address() { return dma.phy_address(); };
    Log_Addr log_address() { return dma.log_address(); };

public:
    No_MMU::DMA_Buffer dma;
    char *dma_ptr;
    unsigned int count_frames = 0;
    unsigned int buffer_size;
};

__END_UTIL

#endif

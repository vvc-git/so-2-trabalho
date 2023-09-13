// EPOS Buffer Declarations

#ifndef __DT_Buffer_h
#define __DT_Buffer_h

#include <utility/list.h>
#include <system.h>
#include <utility/heap.h>
#include <architecture/mmu.h>



__BEGIN_UTIL


//OStream cout;

class DT_Buffer
{


public:
    
    DT_Buffer(void * addr, unsigned long bytes): app(addr, bytes){}
    ~DT_Buffer() {};
    void * alloc(unsigned long int bytes);
    void free(void * ptr, unsigned long bytes) {app.free(ptr, bytes);};

public:
    Heap app;
};

void * DT_Buffer::alloc(unsigned long int bytes) {
    void * addr = app.alloc(bytes);

    return addr;

};


__END_UTIL

#endif

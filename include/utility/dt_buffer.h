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
    
    DT_Buffer(unsigned long bytes);
    ~DT_Buffer() {};
    void * alloc(unsigned long int bytes);
    void free(void * ptr, unsigned long bytes) {buffer->free(ptr, bytes);};

public:
    Heap * buffer;
    char * data;
};

DT_Buffer::DT_Buffer(unsigned long bytes) {

    char data[bytes];
    buffer = new Heap(data, bytes);

};

void * DT_Buffer::alloc(unsigned long int bytes) {
    void * addr = buffer->alloc(bytes);
    return addr;

};



__END_UTIL

#endif

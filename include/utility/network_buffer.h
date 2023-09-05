// EPOS Buffer Declarations

#ifndef __network_buffer_h
#define __network_buffer_h

#include <utility/list.h>
#include <system.h>
#include <utility/heap.h>
#include <utility/string.h>
#include <architecture/mmu.h>



__BEGIN_UTIL


OStream cout;

class Network_buffer
{

private:
    // typedef Grouping_List<char> List;
    // typedef List_Elements::Doubly_Linked_Grouping<char> Element;
    // typedef Simple_List<char> Queue;
    // typedef List_Elements::Singly_Linked<char> Queue_Element;



public:
    
    Network_buffer(void * addr, unsigned long bytes): _app(addr, bytes), _dma(1500*3){}; 
    ~Network_buffer() {};
    void * alloc(unsigned long int bytes);
    void remove();

    // // test purposes
    // char * buffer() {return _buffer;};
    // List list() {return _grouping_mng;};

private:
    // // Lista que gerencia os espaços livres do buffer
    //List _grouping_mng;

    // char * _buffer;

    // Queue _occupied;

    Heap _app;
    No_MMU::DMA_Buffer _dma;
    No_MMU _no_mmu;

};

void * Network_buffer::alloc(unsigned long int bytes) {

    // 
    void * addr = _app.alloc(bytes);

    return addr;

};

// void Network_buffer::remove() {

//     char * addr = _occupied.remove()->object();
//     cout << "remover -> " << reinterpret_cast<long *>(addr) << endl;
//     cout << _occupied.empty() << endl;

//     char * data = (char*)malloc(sizeof(char)*1024*64); //

//     memcpy((void *) data, (void *) addr, 64*1024);

//     cout <<"data[0] = " << data[0] << endl;
//     cout << "data[20] = " << data[20] << endl;

// };

__END_UTIL

#endif

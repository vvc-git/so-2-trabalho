// EPOS Buffer Declarations

#ifndef __network_buffer_h
#define __network_buffer_h

#include <utility/list.h>
#include <system.h>
#include <utility/heap.h>
#include <utility/string.h>



__BEGIN_UTIL

OStream cout;

class Network_buffer
{

private:
    typedef Grouping_List<char> List;
    typedef List_Elements::Doubly_Linked_Grouping<char> Element;
    typedef Simple_List<char> Queue;
    typedef List_Elements::Singly_Linked<char> Queue_Element;


public:
    
    Network_buffer(unsigned int s); 
    ~Network_buffer() { delete _buffer; };
    int insert(char * ptr, unsigned long int size);
    void remove();

    // test purposes
    char * buffer() {return _buffer;};
    List list() {return _grouping_mng;};

private:
    // Lista que gerencia os espaços livres do buffer
    List _grouping_mng;

    char * _buffer;

    Queue _occupied;
};

inline Network_buffer::Network_buffer(unsigned int s) {
    // se multiheap, malloc() irá alocar na heap da aplicação
    // temporario
    _buffer = (char *) malloc(s); // impressão de que a memoria não tá sendo alocada

    // cout << " ,&_buffer: " << &_buffer 
    //      << " ,*_buffer: " << *_buffer 
    //      << " ,&_buffer[0]: " << _buffer[0] 
    //      << " ,_buffer: " << _buffer<< "\n";

    // _buffer[0] = 'a';

    // cout << " _buffer[0] = 'a' \n ";

    // cout << " ,&_buffer: " << &_buffer 
    //      << " ,*_buffer: " << *_buffer 
    //      << " ,&_buffer[0]: " << _buffer[0] 
    //      << " ,_buffer: " << _buffer<< "\n";

    // Criando um elemento de linkagem compatível com a grouping_list e que contenha o buffer como objeto
    Element elem = List_Elements::Doubly_Linked_Grouping<char>(_buffer, s);

    // inserindo este elemento na lista
    _grouping_mng.insert_tail(&elem);

    // cout << "_grouping_mng.head()->object(): " << _grouping_mng.head()->object() << "\n";

};

int Network_buffer::insert(char* data, unsigned long int size) {
    Element * e = _grouping_mng.search_decrementing(size);
    //cout << "e->size(): " << e->size() << endl;
    //cout << "e->object(): " << reinterpret_cast<long *>(e->object()) << endl;
    char * addr = reinterpret_cast<char *>(e->object() + e->size());
    // Element * e = new (ptr) Element(reinterpret_cast<char *>(ptr), bytes);
    // cout << "addr: " << addr << endl;
    //char* pointer = addr;
    //char * char_addr = reinterpret_cast<char *>(addr);
    // unsigned long int len = size;
    memcpy((void *) addr, (void *) data, size);

    // *addr = *data;
    // cout << "addr[0]: " << addr[0] << endl;
    // cout << "addr[20]: " << addr[20] << endl;
    // cout << "addr: " << *addr << endl;

    Queue_Element el = List_Elements::Singly_Linked<char>(&(*addr));

    //cout << "addr: " << el << endl;

    _occupied.insert(&el);

    // char * chr = _occupied.head()->object();

    cout << "occupied.head()->object() " << reinterpret_cast<long *>(_occupied.head()->object()) << endl;

    return 1;
};

void Network_buffer::remove() {
};

__END_UTIL

#endif

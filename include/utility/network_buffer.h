// EPOS Buffer Declarations

#ifndef __network_buffer_h
#define __network_buffer_h

#include <utility/list.h>
#include <system.h>

__BEGIN_UTIL

// OStream cout;

class Network_buffer
{

private:
    typedef Grouping_List<char> List;
    typedef List_Elements::Doubly_Linked_Grouping<char> Element;


public:
    
    Network_buffer(unsigned int s); 
    ~Network_buffer() {delete _buffer;};
    bool insert();
    bool remove();

    // test purposes
    char * buffer() {return _buffer;};
    List list() {return _grouping_mng;};

private:
    // Lista que gerencia os espaços livres do buffer
    List _grouping_mng;

    char * _buffer;
};

inline Network_buffer::Network_buffer(unsigned int s) {
    // se multiheap, malloc() irá alocar na heap da aplicação
    // temporario
    _buffer = (char *) malloc(s); // impressão de que a memoria não tá sendo alocada

    // Criando um elemento de linkagem compatível com a grouping_list e que contenha o buffer como objeto
    Element elem = List_Elements::Doubly_Linked_Grouping<char>(_buffer, s);

    // inserindo este elemento na lista
    _grouping_mng.insert_tail(&elem);

};

__END_UTIL

#endif

// EPOS Buffer Declarations

#ifndef __network_buffer_h
#define __network_buffer_h

// #include <architecture.h>

__BEGIN_SYS

class Network_buffer
{

private:
    typedef Grouping_List<char> List;


public:
    Network_buffer(unsigned int s) {buffer = alloc(s);};
    bool insert();
    bool remove();

private:
    // Lista que gerencia os espaços livres do buffer
    List _free;

    // 
    void * _buffer;


};
#ifndef __udp_manager_h
#define __udp_manager_h

#include <system.h>
#include <network/ip.h>
#include <machine/riscv/riscv_nic.h>
#include <utility/string.h>
#include <utility/observer.h>
#include <synchronizer.h>

__BEGIN_SYS

class UDP_Manager: public Data_Observer<char, void> {
public:
    static void init();
    void hello();
    void update(Data_Observed<char, void> * o, char * d);

public:
    static UDP_Manager* udp_mng;    
};

__END_SYS

#endif
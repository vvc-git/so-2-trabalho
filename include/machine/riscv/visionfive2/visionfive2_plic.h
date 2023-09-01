#ifndef __riscv_sifive_u_plic_h
#define __riscv_sifive_u_plic_h

#include <machine/riscv/riscv_plic.h>

__BEGIN_SYS

class PLIC : private PLIC_Common
{
public:
    static const unsigned int EIRQS = 1024; // Find out how many external interrupts there are

    static Reg32 next_pending() {
        return PLIC_Common::next_pending();
    }

    static void complete(Ex_Interrupt_Id id) {
        PLIC_Common::complete(id);
    }

    static void enable(Ex_Interrupt_Id id) {
        PLIC_Common::enable(id);
    }

    static void disable(Ex_Interrupt_Id id) {
        PLIC_Common::disable(id);
    }

    static void permit_all() {
        PLIC_Common::permit_all();
    }

    static void mask_all() {
        PLIC_Common::mask_all();
    }

    static void set_priority(Ex_Interrupt_Id id, unsigned int priority) {
        PLIC_Common::set_priority(id, priority);
    }

    static void set_threshold(unsigned int tsh) {
        PLIC_Common::set_threshold(tsh);
    }
};

__END_SYS

#endif

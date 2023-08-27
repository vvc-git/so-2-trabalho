// EPOS RISC-V IC Mediator Declarations

#ifndef __riscv_ic_h
#define __riscv_ic_h

#include <architecture/cpu.h>
#include <machine/ic.h>
#include <system/memory_map.h>

__BEGIN_SYS

// Core Local Interrupter (CLINT)
class CLINT
{
private:
    typedef CPU::Reg Reg;
    typedef CPU::Reg32 Reg32;
    typedef CPU::Reg64 Reg64;
    typedef CPU::Phy_Addr Phy_Addr;
    typedef CPU::Log_Addr Log_Addr;

public:
    static const unsigned int IRQS = 16;

    // Interrupts ([M|S]CAUSE with interrupt = 1)
    enum : unsigned long {
        IRQ_USR_SOFT            = 0,
        IRQ_SUP_SOFT            = 1,
        IRQ_MAC_SOFT            = 3,
        IRQ_USR_TIMER           = 4,
        IRQ_SUP_TIMER           = 5,
        IRQ_MAC_TIMER           = 7,
        IRQ_USR_EXT             = 8,
        IRQ_SUP_EXT             = 9,
        IRQ_MAC_EXT             = 11,
        INTERRUPT               = 1UL << (Traits<CPU>::WORD_SIZE - 1),
        INT_MASK                = ~INTERRUPT

    };

    // Registers offsets from CLINT_BASE
    enum {                                // Description
        MSIP                    = 0x0000, // Per-HART 32-bit software interrupts (IPI) trigger; each HART is offset by 4 bytes from MSIP
        MTIMECMP                = 0x4000, // Per-HART 64-bit timer compare register; lower 32 bits stored first; each HART is offset by 8 bytes from MTIMECMP
        MTIME                   = 0xbff8, // Timer counter shared by all HARTs lower 32 bits stored first
    };

    // MTVEC modes
    enum Mode {
        DIRECT  = 0,
        INDEXED = 1
    };

public:
    static void mtvec(Mode mode, Phy_Addr base) {
    	Reg tmp = (base & -4UL) | (Reg(mode) & 0x3);
        ASM("csrw mtvec, %0" : : "r"(tmp) : "cc");
    }

    static Reg mtvec() {
        Reg value;
        ASM("csrr %0, mtvec" : "=r"(value) : : );
        return value;
    }

    static Reg64 mtime() { return *reinterpret_cast<Reg64 *>(Memory_Map::CLINT_BASE + MTIME); }
    static void  mtimecmp(Reg64 v) { *reinterpret_cast<Reg64 *>(Memory_Map::CLINT_BASE + MTIMECMP) = v; }
};

class PLIC
{
private:
    typedef CPU::Reg Reg;
    typedef CPU::Reg32 Reg32;
    typedef CPU::Reg64 Reg64;
    typedef CPU::Phy_Addr Phy_Addr;
    typedef CPU::Log_Addr Log_Addr;

public:
    // Registers offsets from PLIC_CPU_BASE
    enum {                                  // Description
        PLIC_PENDING_1      = 0x001000,     // PLIC Interrupt Pending Register 1 (pending1)
        PLIC_PENDING_2      = 0x001004,     // PLIC Interrupt Pending Register 2 (pending2)
        PLIC_INT_ENABLE_1   = 0x002000,     // PLIC Interrupt Enable Register 1 (enable1) for Hart 0 M-Mode
        PLIC_INT_ENABLE_2   = 0x002404,     // PLIC Interrupt Enable Register 2 (enable2) for Hart 4 S-Mode
        PLIC_THRESHOLD      = 0x200000,     // PLIC Interrupt Priority Threshold Register (threshold)
        PLIC_CLAIM          = 0x200004,     // PLIC Claim/Complete Register (claim)
    };

    enum Mode {
        MACHINE = 0,
        SUPERVISOR = 1,
    };

private:
    static volatile Reg32 & getPriorityReg(unsigned int id) {
        return reinterpret_cast<volatile Reg32 *>(Memory_Map::PLIC_CPU_BASE)[id * 4];
    }

    static volatile Reg32 & getThresholdReg() {
        Reg hartId = CPU::mhartid();
        Reg mode = CPU::mstatus() & 0x1800;

        Mode hartMode = (mode == 0x1800) ? SUPERVISOR : MACHINE;

        if (hartId == 0) {
            return reinterpret_cast<volatile Reg32 *>(Memory_Map::PLIC_CPU_BASE)[PLIC_THRESHOLD/sizeof(Reg32)];
        }
        else {
            return reinterpret_cast<volatile Reg32 *>(Memory_Map::PLIC_CPU_BASE)[(PLIC_THRESHOLD + 0x1000 + 0x2000 * (hartId - 1) + 0x1000*hartMode)/sizeof(Reg32)];
        }
    }

    static volatile Reg32 & getClaimReg() {
        Reg hartId = CPU::mhartid();
        Reg mode = CPU::mstatus() & 0x1800;

        Mode hartMode = (mode == 0x1800) ? SUPERVISOR : MACHINE;

        if (hartId == 0) {
            return reinterpret_cast<volatile Reg32 *>(Memory_Map::PLIC_CPU_BASE)[PLIC_CLAIM/sizeof(Reg32)];
        }
        else {
            return reinterpret_cast<volatile Reg32 *>(Memory_Map::PLIC_CPU_BASE)[((PLIC_CLAIM + 0x1000) + 0x2000 * (hartId - 1) + (0x1000*hartMode))/sizeof(Reg32)];
        }
    }

    static bool isPending(unsigned int id) {
        Reg32 pendingReg;

        if (id < 32) {
            pendingReg = *reinterpret_cast<Reg32 *>(Memory_Map::PLIC_CPU_BASE + PLIC_PENDING_1);
        }
        else {
            pendingReg = *reinterpret_cast<Reg32 *>(Memory_Map::PLIC_CPU_BASE + PLIC_PENDING_2);
        }
        int actualId = 1 << id;

        return ((pendingReg & actualId) != 0);
    }

    static volatile Reg32 & getEnableReg(unsigned int id) {
        if (id < 32) {
            return reinterpret_cast<volatile Reg32 *>(Memory_Map::PLIC_CPU_BASE)[PLIC_INT_ENABLE_1/sizeof(Reg32)];
        }
        else {
            return reinterpret_cast<volatile Reg32 *>(Memory_Map::PLIC_CPU_BASE)[PLIC_INT_ENABLE_2/sizeof(Reg32)];
        }
    }
};

class IC: private IC_Common, private CLINT
{
    friend class Setup;
    friend class Machine;

private:
    typedef CPU::Reg Reg;

public:
    static const unsigned int EXCS = CPU::EXCEPTIONS;
    static const unsigned int IRQS = CLINT::IRQS;
    static const unsigned int INTS = EXCS + IRQS;

    using IC_Common::Interrupt_Id;
    using IC_Common::Interrupt_Handler;

    enum {
        INT_SYS_TIMER = EXCS + IRQ_MAC_TIMER
    };

public:
    IC() {}

    static Interrupt_Handler int_vector(Interrupt_Id i) {
        assert(i < INTS);
        return _int_vector[i];
    }

    static void int_vector(Interrupt_Id i, const Interrupt_Handler & h) {
        db<IC>(TRC) << "IC::int_vector(int=" << i << ",h=" << reinterpret_cast<void *>(h) <<")" << endl;
        assert(i < INTS);
        _int_vector[i] = h;
    }

    static void enable() {
        db<IC>(TRC) << "IC::enable()" << endl;
        CPU::mie(CPU::MSI | CPU::MTI); // TODO: external interrupts should be treated by PLIC (not implemented yet), so not enabled for now
    }

    static void enable(Interrupt_Id i) {
        db<IC>(TRC) << "IC::enable(int=" << i << ")" << endl;
        assert(i < INTS);
        enable();
        // TODO: this should handle individual INTs and also be done at PLIC
    }

    static void disable() {
        db<IC>(TRC) << "IC::disable()" << endl;
        CPU::miec(CPU::MSI | CPU::MTI | CPU::MEI);
    }

    static void disable(Interrupt_Id i) {
        db<IC>(TRC) << "IC::disable(int=" << i << ")" << endl;
        assert(i < INTS);
        disable();
        // TODO: this should handle individual INTs and also be done at PLIC
    }

    static Interrupt_Id int_id() {
        // Id is retrieved from [m|s]cause even if mip has the equivalent bit up, because only [m|s]cause can tell if it is an interrupt or an exception
        Reg id = CPU::mcause();
        if(id & INTERRUPT)
            return irq2int(id & INT_MASK);
        else
            return (id & INT_MASK);
    }

    static int irq2int(int i) { return i + EXCS; }
    static int int2irq(int i) { return i - EXCS; }

private:
    static void dispatch();

    // Logical handlers
    static void int_not(Interrupt_Id i);
    static void exception(Interrupt_Id i);

    // Physical handler
    static void entry() __attribute((naked, aligned(4)));

    static void init();

private:
    static Interrupt_Handler _int_vector[INTS];
};

__END_SYS

#endif

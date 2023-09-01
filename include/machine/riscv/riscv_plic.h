#ifndef __plic_h
#define __plic_h

#include <architecture/cpu.h>
#include <system/memory_map.h>
#include <system/config.h>

__BEGIN_SYS

// Platform Level Interrupt Controller (PLIC)
class PLIC_Common // The current PLIC driver only operates in M-Mode, for any hart. S-Mode is not supported yet.
{
public:
    typedef CPU::Reg Reg;
    typedef CPU::Reg32 Reg32;
    typedef CPU::Reg64 Reg64;
    typedef CPU::Phy_Addr Phy_Addr;
    typedef CPU::Log_Addr Log_Addr;

    typedef unsigned int Ex_Interrupt_Id;

    // PUTS THIS IN TRAITS LATER: EACH PLIC IMPLEMENTATION HAS A DIFFERENT NUMBER OF EINTS. 1024 IS THE MAXIMUM SUPPORTED
    static const unsigned int EIRQS = 1024; // All possible External interrupts. 1024 = EINT0 (no interrupt) to EINT1023.

    enum {
        MIN_GLOBAL_THRESHOLD = 0,
        MAX_GLOBAL_THRESHOLD = 7  // masks all interrupts.
    };

    // Registers offsets from PLIC_CPU_BASE
    enum {                                  // Description
        PLIC_PENDING      = 0x001000,     // PLIC Interrupt Pending base offset.
        PLIC_INT_ENABLE   = 0x002000,     // PLIC Interrupt Enable base offset.
        PLIC_THRESHOLD      = 0x200000,     // PLIC Interrupt Priority Threshold Register (threshold)
        PLIC_CLAIM          = 0x200004,     // PLIC Claim/Complete Register (claim)
    };

    enum Mode {
        MACHINE = 0,
        SUPERVISOR = 1, // Not supported yet
    };

public:
    // Get the next available interrupt. This is the "claim" process.
    // The PLIC will automatically sort by priority and hand us the
    // ID of the interrupt.
    static Reg32 next_pending() {
        // The claim register is filled with the highest-priority, enabled interrupt.
        unsigned int claim_no = get_claim_reg();
        db<PLIC_Common>(TRC)<<"PLIC::next(): claim_no=" << claim_no << endl;
        return claim_no;
    }

    // Complete a pending interrupt by id. The id should come
    // from the next() function above.
    static void complete(Ex_Interrupt_Id id) {
        get_claim_reg() = id;
    }

    // Set the global threshold. The threshold can be a value [0..7].
    // The PLIC will mask any interrupts at or below the given threshold.
    // This means that a threshold of 7 will mask ALL interrupts and
    // a threshold of 0 will allow ALL interrupts.
    static void set_threshold(unsigned int tsh) {
        Ex_Interrupt_Id const actual_tsh = tsh & 7;
        get_threshold_reg() = actual_tsh;
    }

    // permits all interrupts with non-zero priority.
    static void permit_all() {
        set_threshold(MIN_GLOBAL_THRESHOLD);
    }

    // masks all interrupts, no matter the priority.
    static void mask_all() {
        set_threshold(MAX_GLOBAL_THRESHOLD);
    }

    // Enable a given interrupt ID.
    static void enable(Ex_Interrupt_Id id) {
        Reg32 enables = get_enable_reg(id);
        int actualId = 1 << (id % 32);

        // The register is a 32-bit register, so that gives us enables
        // for interrupt 31 through 1 (0 is hardwired to 0).
        get_enable_reg(id) = enables | actualId;
    }

    // Disable a given interrupt ID.
    static void disable(Ex_Interrupt_Id id) {
        Reg32 enables = get_enable_reg(id);
        int actualId = 1 << (id % 32);

        // Clear the bit corresponding to the interrupt id
        get_enable_reg(id) = (enables & ~actualId);
    }

    // Set a given interrupt priority to the given priority.
    static void set_priority(Ex_Interrupt_Id id, unsigned int priority) {
        unsigned int actualPriority = priority & 7; // 7 is the max priority
        get_priority_reg(id) = actualPriority; // Set the priority
    }

private:
    static volatile Reg32 & get_priority_reg(unsigned int id) {

        return reinterpret_cast<volatile Reg32 *>(Memory_Map::PLIC_CPU_BASE)[id];
    }

    static volatile Reg32 & get_threshold_reg() {
        Reg hartId = CPU::mhartid();
        db<PLIC_Common>(TRC)<<"PLIC::get_threshold_reg: hartId=" << hartId << endl;
        if (hartId == 0) {
            return reinterpret_cast<volatile Reg32 *>(Memory_Map::PLIC_CPU_BASE)[PLIC_THRESHOLD/sizeof(Reg32)];
        }
        else {
            // + 0x1000 gets to Hart-1 threshold + 0x2000 * (n - 1) gets to Hart-n threshold (skipping S-Mode)
            return reinterpret_cast<volatile Reg32 *>(Memory_Map::PLIC_CPU_BASE)[(PLIC_THRESHOLD + 0x1000 + 0x2000 * (hartId - 1))/sizeof(Reg32)];
        }
    }

    static volatile Reg32 & get_claim_reg() {
        Reg hartId = CPU::mhartid();
        db<PLIC_Common>(TRC)<<"PLIC::get_claim_reg: hartId=" << hartId << endl;
        if (hartId == 0) {
            return reinterpret_cast<volatile Reg32 *>(Memory_Map::PLIC_CPU_BASE)[PLIC_CLAIM/sizeof(Reg32)];
        }
        else {
            // + 0x1000 gets to Hart-1 claim + 0x2000 * (n - 1) gets to Hart-n claim (skipping S-Mode)
            return reinterpret_cast<volatile Reg32 *>(Memory_Map::PLIC_CPU_BASE)[(PLIC_CLAIM + 0x1000) + 0x2000 * (hartId - 1)/sizeof(Reg32)];
        }
    }

    static bool is_pending(Ex_Interrupt_Id id) {
        // 32 registers with 32 bits for enable each. 0th bit is hardwired to 0.
        Reg32 pendingReg = *reinterpret_cast<Reg32 *>(Memory_Map::PLIC_CPU_BASE + (PLIC_PENDING/sizeof(Reg32)) + (id / 32));

        int actualId = 1 << (id % 32);

        return ((pendingReg & actualId) != 0);
    }

    static volatile Reg32 & get_enable_reg(Ex_Interrupt_Id id) {
        unsigned int hartId = CPU::mhartid();
        db<PLIC_Common>(TRC)<<"PLIC::get_enable_reg: hartId=" << hartId << endl;
        // + 32*(hartId) gets to the correct hart's enable base address
        // + Math::floor(id/32) gets to the correct register within the hart's enable base address
        return reinterpret_cast<volatile Reg32 *>(Memory_Map::PLIC_CPU_BASE)[(PLIC_INT_ENABLE/sizeof(Reg32)) + 32*(hartId) + (id / 32)];
    }
};

__END_SYS

#endif

#if defined(__PLIC_H) && !defined(__plic_common_only__)
#include __PLIC_H
#endif

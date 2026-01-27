//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lir/machine/MachFunction.hpp"
#include "lir/machine/MachInst.hpp"
#include "lir/machine/MachLabel.hpp"
#include "lir/machine/Register.hpp"
#include "lir/machine/RegisterAllocator.hpp"
#include "lir/machine/RegisterAnalysis.hpp"
#include "lir/machine/Segment.hpp"

#ifdef LIR_MACHINE_DEBUGGING
#include <iostream>
#endif // LIR_MACHINE_DEBUGGING

using namespace lir;

using Ranges = std::vector<LiveRange>;

/// A pass that linearly scans over the machine intermediate representation
/// (MIR) to identify the positions/ranges in which virtual registers are live.
class LinearScan final {
    const MachFunction &m_func;
    Ranges &m_ranges;
    uint32_t m_pos = 0;

    /// Update the live range for the given |reg| to now continue to |pos|. 
    /// If a live range does not exist for the register, then create a new one 
    /// with the given |cls|, starting at |pos|.
    LiveRange &update_range(Register reg, RegisterClass cls, uint32_t pos) {
        // Attempt to find an existing range for |reg|.
        // @Todo: map ranges by the register they involve for faster lookup.
        for (auto &range : m_ranges) {
            // The range has been killed i.e. is no longer live, so we never 
            // update it.
            if (range.killed)
                continue;

            if (range.reg == reg) {
                // A live range still exists for |reg|, so update its 
                // endpoint and stop here.
                range.end = pos;
                return range;
            }
        }

        // No live range for |reg| could be found, so we begin a new one.
        LiveRange range = {};
        range.reg = reg;
        range.start = pos;
        range.end = pos;
        range.cls = cls;
        range.killed = false;
        range.alloc = Register::NoRegister;

        if (reg.is_physical())
            range.alloc = reg;

        m_ranges.push_back(range);
        return m_ranges.back();
    }

    /// Process the given |inst|, determining live ranges for each of its
    /// operands that involve registers.
    void process_inst(const MachInst &inst) {
        for (const MachOperand &operand : inst.get_operands()) {
            // Only process register/memory operands.
            if (!operand.is_reg() && !operand.is_mem())
                continue;

            Register reg;
            RegisterClass cls;

            // Determine the register used in this operand.
            if (operand.is_reg()) {
                reg = operand.get_reg();
            } else if (operand.is_mem()) {
                reg = operand.get_mem_base();
            }

            if (reg.is_physical()) {
                cls = get_register_class(static_cast<X64_Register>(reg.id()));
            } else {
                // The register is virtual, so we need to determine the 
                // register class it needs to be allocated to. This is stored
                // in the function register table created during instruction
                // selection.
                const auto &rtable = m_func.get_register_table();
                assert(rtable.count(reg.id()) != 0 && 
                    "virtual register does not exist in register table!");
                
                cls = rtable.at(reg.id()).cls;
            }

            LiveRange &range = update_range(reg, cls, m_pos);

            if (operand.is_reg() && (operand.is_kill() || operand.is_dead())) {
                // If the register is no longer live at the point of this 
                // instruction, then it should be killed off.
                range.end = m_pos;
                range.killed = true;
            }
        }
    }

public:
    LinearScan(const MachFunction &func, Ranges &ranges)
      : m_func(func), m_ranges(ranges) {}

    void run() {
        const MachLabel *curr = m_func.get_head();
        while (curr) {
            for (const MachInst &inst : curr->insts()) {
                process_inst(inst);
                m_pos++;
            }

            curr = curr->get_next();
        }
    }
};

/// An analysis pass that identifies the need for caller-saved register spills
/// around call instructions per ABI conventions.
class CallsiteAnalysis final {
    MachFunction& m_func;
    const Ranges &m_ranges;
    uint32_t m_pos = 0;

    void process_label(MachLabel &label) {
        const Machine &mach = m_func.get_machine();

        std::vector<MachInst> insts = {};
        insts.reserve(label.size());

        // For each call instruction under this label, spill any registers 
        // whose liveliness overlap to the stack.
        for (uint32_t i = 0, e = label.size(); i < e; ++m_pos, ++i) {
            MachInst& inst = label.insts().at(i);

            if (inst.op() != X64_Mnemonic::CALL) {
                // Non-calls simply get placed in their usual spots.
                insts.push_back(inst);
                continue;
            }

            std::vector<Register> to_spill = {};

            // For each live register at this point, mark it to be spilled if 
            // it is a caller-saved register.
            for (const LiveRange &range : m_ranges) {
                if (!range.overlaps(m_pos))
                    continue;

                auto alloc = static_cast<X64_Register>(range.alloc.id());
                if (mach.is_caller_saved(alloc))
                    to_spill.push_back(alloc);
            }

            // For each register that needs to be spilled, push it to the stack
            // just before the call.
            for (const Register &reg : to_spill) {
                insts.push_back(MachInst(X64_Mnemonic::PUSH, X64_Size::Quad)
                    .add_reg(reg, 8, false)
                );
            }

            // Add the call instruction now.
            insts.push_back(inst);

            // For each register that was spilled, pop it back off the stack
            // now that the call has completed.
            for (const Register &reg : to_spill) {
                insts.push_back(MachInst(X64_Mnemonic::POP, X64_Size::Quad)
                    .add_reg(reg, 8, true)
                );
            }
        }

        // Replace the instructions under the label with the new list.
        label.insts() = insts;
    }

public:
    CallsiteAnalysis(MachFunction &func, const Ranges &ranges)
      : m_func(func), m_ranges(ranges) {}
    
    void run() {
        MachLabel *curr = m_func.get_head();
        while (curr) {
            process_label(*curr);
            curr = curr->get_next();
        }
    }
};

void RegisterAnalysis::run() {
    for (const auto &[name, func] : m_seg.get_functions()) {
        std::vector<LiveRange> ranges = {};
        
        // Run linear scan over the function, producing live ranges for each
        // register used.
        LinearScan linscan { *func, ranges };
        linscan.run();

#ifdef LIR_MACHINE_DEBUGGING
        std::cerr << "Function '" << name << "' ranges:\n";
        for (const auto& range : ranges) {
            if (range.reg.is_virtual()) {
                std::cerr << 'v' << range.reg.id() - Register::VirtualBarrier;
            } else {
                std::cerr << '%' << range.reg.id();
            }

            std::cerr << " [" << range.start << ", " << range.end << "]\n";
        }
#endif // LIR_MACHINE_DEBUGGING

        // Allocate registers over the live ranges produced by the linear scan.
        // The allocator assigns the physical registers to the ranges 
        // themselves.
        RegisterAllocator allocator { *func, ranges };
        allocator.run();

        MachFunction::RegisterTable &rtable = func->get_register_table();
        
        // For each range, propogate the allocated register to the function
        // register table.
        for (const LiveRange &range : ranges) {
            const Register reg = range.reg;
            
            // Skip physical registers, since their allocations are already known.
            if (reg.is_physical()) 
                continue;

            rtable[reg.id()].alloc = range.alloc;
        }

        CallsiteAnalysis canal { *func, ranges };
        canal.run();
    }
}

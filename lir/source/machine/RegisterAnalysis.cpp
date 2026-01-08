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

/// Implementation of a linear scan over the intermediate representation to
/// identify live ranges of named producers.
class LinearScan final {
    const MachFunction& m_function;
    std::vector<LiveRange>& m_ranges;
    uint32_t m_position = 0;

    LiveRange& update_range(Register reg, RegisterClass cls, uint32_t pos) {
        // Attempt to find an existing range for |reg|.
        for (auto& range : m_ranges) {
            // The range has been killed, so we never update it.
            if (range.killed)
                continue;

            if (range.reg == reg) {
                range.end = pos;
                return range;
            }
        }

        // No existing range could be found, so we begin a new one.
        LiveRange range = {};
        range.reg = reg;
        range.start = range.end = pos;
        range.cls = cls;
        range.killed = false;
        range.alloc = Register::NoRegister;

        if (reg.is_physical())
            range.alloc = reg;

        m_ranges.push_back(range);
        return m_ranges.back();
    }

    void process_inst(const MachInst& inst) {
        for (const MachOperand& op : inst.get_operands()) {
            if (!op.is_reg() && !op.is_mem())
                continue;

            Register reg;
            RegisterClass cls;

            if (op.is_reg()) {
                reg = op.get_reg();
            } else if (op.is_mem()) {
                reg = op.get_mem_base();
            }

            if (reg.is_physical()) {
                cls = get_register_class(static_cast<X64_Register>(reg.id()));
            } else {
                // reg refers to a virtual register, whose
                // information is stored in the parent function.
                const MachFunction::RegisterTable& regs = 
                    m_function.get_register_table();
                assert(regs.count(reg.id()) != 0);
                cls = regs.at(reg.id()).cls;
            }

            LiveRange& range = update_range(reg, cls, m_position);
            if (op.is_reg() && op.is_kill()) {
                range.end = m_position;
                range.killed = true;
            }
        }
    }

public:
    LinearScan(const MachFunction& function, std::vector<LiveRange>& ranges)
      : m_function(function), m_ranges(ranges) {}

    void run() {
        m_position = 0;
        const MachLabel* curr = m_function.get_head();
        while (curr) {
            for (const MachInst& inst : curr->insts()) {
                process_inst(inst);
                ++m_position;
            }

            curr = curr->get_next();
        }
    }
};

/// Implementation of a per-function pass that identifies the need for spills
/// around call instructions due to ABI conventions.
class CallsiteAnalysis final {
    MachFunction& m_function;
    const std::vector<LiveRange>& m_ranges;
    uint32_t m_position = 0;

    void process_label(MachLabel& label) {
        std::vector<MachInst> insts = {};
        insts.reserve(label.size());

        for (uint32_t i = 0, e = label.size(); i < e; ++m_position, ++i) {
            MachInst& inst = label.insts().at(i);

            X64_Mnemonic op = inst.op();
            if (op == X64_Mnemonic::CALL) {
                std::vector<Register> spill = {};

                for (const LiveRange& range : m_ranges) {
                    if (!range.overlaps(m_position))
                        continue;

                    X64_Register alloc = static_cast<X64_Register>(range.alloc.id());
                    if ( m_function.get_machine().is_caller_saved(alloc))
                        spill.push_back(alloc);
                }

                for (const Register& reg : spill) {
                    MachOperand op = MachOperand::create_reg(reg, 8, false);
                    insts.push_back(MachInst(
                        X64_Mnemonic::PUSH, X64_Size::Quad, { op }));
                }

                insts.push_back(inst);

                for (const Register& reg : spill) {
                    MachOperand op = MachOperand::create_reg(reg, 8, true);
                    insts.push_back(MachInst(X64_Mnemonic::POP, X64_Size::Quad, { op }));
                }
            } else {
                insts.push_back(inst);
            }
        }

        label.insts() = insts;
    }

public:
    CallsiteAnalysis(MachFunction& function, 
                     const std::vector<LiveRange>& ranges)
      : m_function(function), m_ranges(ranges) {}
    
    void run() {
        m_position = 0;
        MachLabel* curr = m_function.get_head();
        while (curr) {
            process_label(*curr);
            curr = curr->get_next();
        }
    }
};

void RegisterAnalysis::run() {
    for (const auto& [name, function] : m_seg.get_functions()) {
        std::vector<LiveRange> ranges;
        
        LinearScan linscan { *function, ranges };
        linscan.run();

#ifdef LIR_MACHINE_DEBUGGING
        std::cerr << "Function '" << name << "' ranges:\n";
        for (const auto& range : ranges) {
            if (range.reg.is_virtual()) {
                std::cerr << 'v' << range.reg.id() - MachRegister::VirtualBarrier;
            } else {
                std::cerr << '%' << x64::to_string(static_cast<x64::Register>(
                    range.reg.id()), 8);
            }

            std::cerr << " [" << range.start << ", " << range.end << "]\n";
        }
#endif // LIR_MACHINE_DEBUGGING

        RegisterAllocator allocator { *function, ranges };
        allocator.run();

        MachFunction::RegisterTable& regs = function->get_register_table();
        for (const auto& range : ranges) {
            Register reg = range.reg;
            if (reg.is_physical())
                continue;

            regs[reg.id()].alloc = range.alloc;
        }

        CallsiteAnalysis CAN { *function, ranges };
        CAN.run();
    }
}

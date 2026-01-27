//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lir/graph/Value.hpp"
#include "lir/machine/MachLabel.hpp"
#include "lir/machine/MachInst.hpp"
#include "lir/machine/MachOperand.hpp"
#include "lir/machine/MachFunction.hpp"
#include "lir/machine/Register.hpp"
#include "lir/machine/Printer.hpp"

#include <format>

using namespace lir;

static const MachFunction::RegisterTable *rtable = nullptr;

static void print_operand(std::ostream &os, const MachFunction &func, 
                          const MachOperand &operand) {
    switch (operand.kind()) {
        case MachOperand::Reg: {
            if (operand.is_def()) {
                if (operand.is_implicit())
                    os << "implicit-def ";

                if (operand.is_dead())
                    os << "dead ";
            } else if (operand.is_use()) {
                if (operand.is_implicit())
                    os << "implicit-use ";

                if (operand.is_kill())
                    os << "killed ";
            }

            Register reg = operand.get_reg();
            if (reg.is_virtual()) {
                // If the register operand is virtual, then replace it with
                // it's allocated physical register, if it exists.
                Register alloc = rtable->at(reg.id()).alloc;
                if (alloc != Register::NoRegister)
                    reg = alloc;
            }

            if (reg.is_virtual()) {
                uint32_t vreg = reg.id() - Register::VirtualBarrier;
                os << std::format("v{}:{}", vreg, operand.get_subreg());
            } else {
                X64_Register mreg = static_cast<X64_Register>(reg.id());
                os << std::format("%{}", to_string(mreg, operand.get_subreg()));
            }

            break;
        }

        case MachOperand::Memory: {
            os << '[';

            Register reg = operand.get_mem_base();
            if (reg.is_virtual()) {
                uint32_t vreg = reg.id() - Register::VirtualBarrier;
                os << std::format("v{}", vreg);
            } else {
                X64_Register mreg = static_cast<X64_Register>(reg.id());
                os << std::format("%{}", to_string(mreg, 8));
            }

            // Print memory displacement if it is non-zero. If it is zero, then
            // the access appears like [rax].
            if (operand.get_mem_disp() != 0) {
                if (operand.get_mem_disp() > 0)
                    os << '+'; // Use '+' to signify positive displacement.

                os << operand.get_mem_disp();
            }

            os << ']';
            break;
        }

        case MachOperand::Stack:
            os << "stack." << operand.get_stack();
            break;

        case MachOperand::Immediate:
            os << '$' << operand.get_imm();
            break;

        case MachOperand::Label:
            os << std::format("bb{}", operand.get_label()->position());
            break;

        case MachOperand::Constant:
            os << "const." << operand.get_constant();
            break;
        
        case MachOperand::Symbol: 
            os << operand.get_symbol();
            break;
    }
}

static void print_inst(std::ostream &os, const MachFunction &func,
                       const MachInst &inst) {
    if (inst.has_comment())
        os << std::format("\t; {}\n", inst.get_comment());

    os << '\t';

    if (inst.num_explicit_defs() > 0) {
        // If there is an explicit definition, then we want to print it in a
        // format of 'rxy = ...', so we print the first explicit-def first.

        for (const MachOperand& operand : inst.get_operands()) {
            if (operand.is_reg() && operand.is_explicit_def()) {
                print_operand(os, func, operand);
                break;
            }
        }

        os << " = ";
    }

    os << std::format("{}{} ", to_string(inst.op()), to_string(inst.size()));

    // Print each operand of this instruction.
    for (uint32_t i = 0, e = inst.num_operands(); i < e; ++i) {
        const MachOperand& operand = inst.get_operand(i);

        // Skip printing the explicit def since we already did it before the
        // mnemonic.
        if (operand.is_reg() && operand.is_explicit_def())
            continue;

        print_operand(os, func, operand);

        // If this isn't the last operand, append a comma.
        if (i + 1 != e) {
            const MachOperand& next = inst.get_operand(i + 1);

            // Don't append a comma if the next operand is an explicit def
            // cause it was already printed.
            //
            // @Todo: reassess this control flow. it works, but is nonsensical.
            if (!(next.is_reg() && next.is_explicit_def()))
                os << ", ";
        }
    }

    os << '\n';
}

static void print_label(std::ostream &os, const MachFunction &func, 
                        const MachLabel &label) {
    os << std::format("bb{}:\n", label.position());

    for (const MachInst& inst : label.insts())
        print_inst(os, func, inst);
}

static void print_function(std::ostream &os, const MachFunction &func) {
    rtable = &func.get_register_table();

    os << std::format("{}:\n", func.get_name());

    // Print each of the entries in this function's stack frame.
    const StackFrame &frame = func.get_stack_frame();
    for (uint32_t i = 0, e = frame.num_entries(); i < e; ++i) {
        const StackEntry &entry = frame.entries[i];
        os << std::format("\tstack #{}, offset: {}, size: {}, align: {}\n", 
            i, entry.offset, entry.size, entry.align); 
    }

    if (frame.num_entries() > 0)
        os << '\n';

    // Print each of the constants in this function's constant pool.
    const ConstantPool &pool = func.get_constant_pool();
    for (uint32_t i = 0, e = pool.num_entries(); i < e; ++i) {
        const ConstantPoolEntry &entry = pool.entries[i];
        os << std::format("\tconst #{}, ", 
            i, entry.constant->get_type()->to_string());
        entry.constant->print(os, PrintPolicy::Def);
        os << '\n';
    }

    // If there were stack/pool entries, then emit a new line before the 
    // instructions for cleanliness.
    if (pool.num_entries() > 0 )
        os << '\n';

    // Print each label in this function.
    const MachLabel* curr = func.get_head();
    while (curr) {
        print_label(os, func, *curr);
        curr = curr->get_next();
    }

    os << '\n';
}

void Printer::run(std::ostream &os) const {
    rtable = nullptr;

    for (const auto& [name, function] : m_seg.get_functions())
        print_function(os, *function);
}

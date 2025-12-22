//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "spbe/machine/MachBasicBlock.hpp"
#include "spbe/machine/MachInstruction.hpp"
#include "spbe/machine/MachOperand.hpp"
#include "spbe/machine/MachOperand.hpp"
#include "spbe/machine/MachFunction.hpp"
#include "spbe/X64/X64.hpp"
#include "spbe/X64/X64Printer.hpp"

using namespace spbe;

static const FunctionRegisterInfo* g_register_info = nullptr;

static void print_operand(std::ostream& os, const MachFunction& MF, 
                          const MachOperand& MO) {
    switch (MO.kind()) {
    case MachOperand::MO_Register: {
        if (MO.is_def()) {
            if (MO.is_implicit())
                os << "implicit-def ";

            if (MO.is_dead())
                os << "dead ";
        } else if (MO.is_use()) {
            if (MO.is_implicit())
                os << "implicit ";

            if (MO.is_kill())
                os << "killed ";
        }

        const FunctionRegisterInfo& regi = MF.get_register_info();
        MachRegister reg = MO.get_reg();
        if (reg.is_virtual()) {
            MachRegister alloc = regi.vregs.at(reg.id()).alloc;
            if (alloc != MachRegister::NoRegister)
                reg = alloc;
        }

        if (reg.is_virtual()) {
            os << 'v' << (reg.id() - MachRegister::VirtualBarrier) << 
                ':' << MO.get_subreg();
        } else {
            os << "%" << x64::to_string(
                static_cast<x64::Register>(reg.id()), MO.get_subreg());
        }

        break;
    }

    case MachOperand::MO_Memory: {
        os << '[';

        MachRegister reg = MO.get_mem_base();
        if (reg.is_virtual()) {
            os << 'v' << (reg.id() - MachRegister::VirtualBarrier);
        } else {
            os << '%' << x64::to_string(
                static_cast<x64::Register>(reg.id()), 64);
        }

        if (MO.get_mem_disp() != 0) {
            if (MO.get_mem_disp() > 0)
                os << '+';

            os << MO.get_mem_disp();
        }

        os << ']';
        break;
    }

    case MachOperand::MO_StackIdx:
        os << "stack." << MO.get_stack_index();
        break;

    case MachOperand::MO_Immediate:
        os << '$' << MO.get_imm();
        break;

    case MachOperand::MO_BasicBlock:
        os << "bb" << MO.get_mmb()->position();
        break;

    case MachOperand::MO_ConstantIdx:
        os << "const." << MO.get_constant_index();
        break;
    
    case MachOperand::MO_Symbol: 
        os << MO.get_symbol();
        break;
    }
}

static void print_inst(std::ostream& os, const MachFunction& MF,
                       const MachInstruction& MI) {
    os << "    ";

    if (MI.num_explicit_defs() == 1) {
        for (auto& MO : MI.operands()) {
            if (MO.is_reg() && MO.is_explicit_def()) {
                print_operand(os, MF, MO);
                break;
            }
        }

        os << " = ";
    }

    os << x64::to_string(static_cast<x64::Opcode>(MI.opcode())) << " ";

    for (uint32_t idx = 0, e = MI.num_operands(); idx != e; ++idx) {
        const MachOperand& mo = MI.get_operand(idx);
        if (MI.num_explicit_defs() == 1) {
            if (mo.is_reg() && mo.is_explicit_def())
                continue;
        }

        print_operand(os, MF, mo);
        if (idx + 1 != e) {
            const MachOperand& next = MI.get_operand(idx + 1);
            if (!next.is_reg() || !next.is_explicit_def())
                os << ", ";
        }
    }
}

static void print_block(std::ostream& os, const MachFunction& MF,
                        const MachBasicBlock& MBB) {
    os << "bb" << MBB.position() << ":\n";

    for (auto inst : MBB.insts()) {
        print_inst(os, MF, inst);
        os << '\n';
    }
}

static void print_function(std::ostream& os, const MachFunction& MF) {
    g_register_info = &MF.get_register_info();

    os << MF.get_name() << ":\n";

    const FunctionStackInfo& stack = MF.get_stack_info();
    for (uint32_t idx = 0, e = stack.num_entries(); idx != e; ++idx) {
        const FunctionStackEntry& entry = stack.entries[idx];
        os << "    stack." << idx << " offset: " << entry.offset << ", size: " << 
            entry.size << ", align: " << entry.align << '\n';
    }

    const FunctionConstantPool& pool = MF.get_constant_pool();
    for (uint32_t idx = 0, e = pool.num_entries(); idx != e; ++idx) {
        const FunctionConstantPoolEntry& entry = pool.entries[idx];
        os << "    const." << idx << ' ' << 
            entry.constant->get_type()->to_string() << ' ';
        entry.constant->print(os);
        os << '\n';
    }

    if (stack.num_entries() > 0 || pool.num_entries() > 0 )
        os << '\n';

    for (auto curr = MF.front(); curr; curr = curr->next())
        print_block(os, MF, *curr);
}

void x64::X64Printer::run(std::ostream& os) const {
    g_register_info = nullptr;

    for (const auto& [name, function] : m_obj.functions()) {
        print_function(os, *function);
        os << '\n';
    }
}

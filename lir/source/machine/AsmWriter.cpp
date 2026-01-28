//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lir/graph/BasicBlock.hpp"
#include "lir/graph/CFG.hpp"
#include "lir/graph/Constant.hpp"
#include "lir/graph/Function.hpp"
#include "lir/graph/Type.hpp"
#include "lir/machine/AsmWriter.hpp"
#include "lir/machine/MachInst.hpp"
#include "lir/machine/MachOperand.hpp"
#include "lir/machine/MachFunction.hpp"
#include "lir/machine/Register.hpp"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <format>

using namespace lir;

X64_Register AsmWriter::map_register(Register reg, const MachFunction& func) {
    if (reg.is_virtual())
        reg = func.get_register_table().at(reg.id()).alloc;

    return static_cast<X64_Register>(reg.id());
}

bool AsmWriter::is_redundant_move(const MachFunction& func, const MachInst& inst) {
    if (inst.op() != X64_Mnemonic::MOV)
        return false;

    if (inst.num_operands() != 2)
        return false;

    const MachOperand& left = inst.get_operand(0);
    const MachOperand& right = inst.get_operand(1);

    if (!left.is_reg() || !right.is_reg())
        return false;

    X64_Register regl = map_register(left.get_reg(), func);
    X64_Register regr = map_register(right.get_reg(), func);
    return regl == regr && left.get_subreg() == right.get_subreg();
}

void AsmWriter::emit_operand(
        std::ostream& os, const MachFunction& func, const MachOperand& op) {
    switch (op.kind()) {
        case MachOperand::Reg:
            os << std::format("%{}", 
                to_string(map_register(op.get_reg(), func), op.get_subreg()));
            break;

        case MachOperand::Memory:
            if (op.get_mem_disp() != 0) 
                os << op.get_mem_disp();

            os << std::format("(%{})", 
                to_string(map_register(op.get_mem_base(), func)));
            break;

        case MachOperand::Stack: {
            const StackFrame& frame = func.get_stack_frame();
            const StackEntry& entry = frame.entries.at(op.get_stack());

            // Stack is accessed on negative offsets, so negate the offset.
            os << std::format("{}(%rbp)", 
                (-entry.offset - static_cast<int32_t>(entry.size)));
            break;
        }

        case MachOperand::Immediate:
            os << '$' << op.get_imm();
            break;

        case MachOperand::Label:
            os << std::format(".L{}_{}", m_funcs.at(&func), op.get_label()->position());
            break;
        
        case MachOperand::Constant:
            os << std::format(".LCPI{}_{}(%rip)", m_funcs.at(&func), op.get_constant());
            break;
        
        case MachOperand::Symbol:
            os << op.get_symbol();
            break;
        
        default:
            assert(false && "unrecognized machine operand kind!");
    }
}

void AsmWriter::emit_inst(
        std::ostream& os, const MachFunction& func, const MachInst& inst) {
    if (inst.has_comment())
        os << std::format("#\t{}\n", inst.get_comment());
    
    if (is_redundant_move(func, inst)) {
        os << "#RM"; // Little marking for removed instructions.
    }

    // If this is a return instruction, inject necessary epilogue parts.
    // @Todo: make this optional along with prologue injection.
    if (inst.op() == X64_Mnemonic::RET) {
        os << std::format(
            "\taddq\t${}, %rsp\n\tpopq\t%rbp\n\tret\n", 
            func.get_stack_frame().alignment());
        return;
    }

    os << std::format("\t{}{}\t", to_string(inst.op()), to_string(inst.size()));

    // Emit all (explicit) instruction operands.
    for (uint32_t i = 0, e = inst.num_explicit_operands(); i < e; ) {
        emit_operand(os, func, inst.get_operand(i));
        if (++i != e) 
            os << ", ";
    }

    os << '\n';
}

void AsmWriter::emit_label(
        std::ostream& os, const MachFunction& func, const MachLabel& label) {
    os << std::format(".L{}_{}:\n", m_funcs.at(&func), label.position());

    for (const auto& inst : label.insts()) 
        emit_inst(os, func, inst);
}

void AsmWriter::emit_function(std::ostream& os, const MachFunction& func) {
    m_funcs.emplace(&func, m_funcs.size());
    const std::string& name = func.get_name();
    const ConstantPool& pool = func.get_constant_pool();
    int32_t last_size = -1;

    for (uint32_t i = 0, e = pool.num_entries(); i < e; ++i) {
        const ConstantPoolEntry& entry = pool.entries.at(i);
        const Constant* constant = entry.constant;

        uint32_t size = func.get_machine().get_size(constant->get_type());
        if (size != last_size) {
            os << std::format(
                "\t.section\t.rodata.cst{},\"aM\", @progbits, 8\n\t.p2align\t{},0x0\n", 
                size, 
                std::log2(size));
            last_size = size;
        }

        os << std::format(".LCPI{}_{}:\n", m_funcs.at(&func), i);
        emit_constant(os, *constant);
    }

    os << "\t.text\n";

    if (func.get_function()->has_linkage(Function::LinkageType::Public))
        os << std::format("\t.global\t{}\n", name);

    os << std::format(
        "\t.type\t{}, @function\n{}:\n\tpushq\t%rbp\n\tmovq\t%rsp, %rbp\n\tsubq\t${}, %rsp\n", 
        name, 
        name, 
        func.get_stack_frame().alignment());

    const MachLabel* curr = func.get_head();
    while (curr) {
        emit_label(os, func, *curr);
        curr = curr->get_next();
    }

    os << std::format(
        ".LFE{}:\n\t.size\t{}, .-{}\n\n", m_funcs.at(&func), name, name);
}

void AsmWriter::emit_constant(std::ostream& os, const Constant& constant) {
    const Machine& mach = m_seg.get_machine();
    const uint32_t size = mach.get_size(constant.get_type());

    os << "\t.";

    if (const Integer* integer = dynamic_cast<const Integer*>(&constant)) {
        switch (size) {
            case 1:
                os << "byte";
                break;
        
            case 2:
                os << "word";
                break;
            case 4:
                os << "long";
                break;
            case 8:
                os << "quad";
                break;
        }

        os << '\t' << integer->get_value();
    } else if (const Float* fp = dynamic_cast<const Float*>(&constant)) {
        switch (size) {
            case 4: {
                os << "long 0x";

                uint32_t bits = 0;
                float value = fp->get_value();
                std::memcpy(&bits, &value, sizeof(bits));
                os << std::hex << bits << std::dec;
                break;
            }
            case 8: {
                os << "quad 0x";

                uint64_t bits = 0;
                double value = fp->get_value();
                std::memcpy(&bits, &value, sizeof(bits));
                os << std::hex << bits << std::dec;
                break;
            }
            default:
                assert(false && "unsupported SSE floating point size!");
        }
    } else if (const Null* null = dynamic_cast<const Null*>(&constant)) {
        os << "quad 0x0";
    } else if (const String* string = dynamic_cast<const String*>(&constant)) {
        os << "string \"";

        for (uint32_t i = 0, e = string->get_value().size(); i < e; ++i) {
            switch (string->get_value()[i]) {
                case '\\':
                    os << "\\\\";
                    break;
                case '\'':
                    os << "\\'";
                    break;
                case '\"':
                    os << "\\\"";
                    break;
                case '\n':
                    os << "\\n";
                    break;
                case '\t':
                    os << "\\t";
                    break;
                case '\r':
                    os << "\\r";
                    break;
                case '\b':
                    os << "\\b";
                    break;
                case '\0':
                    os << "\\0";
                    break;
                default:
                    os << string->get_value()[i];
                    break;
            }
        }

        os << '"';
    }

    os << '\n';
}

void AsmWriter::emit_global(std::ostream& os, const Global& global) {
    if (global.is_read_only()) {
        os << "\t.section\t.rodata\n";
    } else {
        os << "\t.data\n";
    }

    if (global.get_linkage() == Global::External)
        os << std::format("\t.global{}\n", global.get_name());

    const Machine& mach = m_seg.get_machine();
    Type* type = static_cast<PointerType*>(global.get_type())->get_pointee();

    const Constant* init = global.get_initializer();
    const uint32_t size = mach.get_size(type);
    const uint32_t align = mach.get_align(type);

    os << "\t.align\t" << align << '\n'
       << "\t.type\t" << global.get_name() << ", @object\n"
       << "\t.size\t" << global.get_name() << ", " << size << '\n'
       << global.get_name() << ":\n";

    if (init) {
        emit_constant(os, *init);
    } else {
        os << std::format("\t.zero\t{}\n", size);
    }
}

void AsmWriter::run(std::ostream& os) {
    os << "\t.file\t\"" << m_seg.get_graph().get_filename() << "\"\n";

    for (const auto& global : m_seg.get_graph().get_globals())
        emit_global(os, *global);

    for (const auto& pair : m_seg.get_functions()) {
        emit_function(os, *pair.second);
    }

    //os << "\t.ident\t\t\"lovelace: 0.1.0\"\n" 
    //   << "\t.section\t.note.GNU-stack,\"\",@progbits\n";
}

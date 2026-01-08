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

static const char* to_string(X64_Size size) {
    switch (size) {
        case X64_Size::None:
            return "";
        case X64_Size::Byte:
            return "b";
        case X64_Size::Word:
            return "w";
        case X64_Size::Long:
            return "l";
        case X64_Size::Quad:
            return "q";
        case X64_Size::Single:
            return "s";
        case X64_Size::Double:
            return "d";
    }
}

static const char* to_string(X64_Mnemonic op) {
    switch (op) {
        case X64_Mnemonic::NOP:
            return "nop";
        case X64_Mnemonic::JMP:
            return "jmp";
        case X64_Mnemonic::UD2:
            return "ud2";
        case X64_Mnemonic::CQO:
            return "cqo";
        case X64_Mnemonic::SYSCALL:
            return "syscall";
        case X64_Mnemonic::CALL:
            return "call";
        case X64_Mnemonic::RET:
            return "ret";
        case X64_Mnemonic::LEA:
            return "lea";
        case X64_Mnemonic::PUSH:
            return "push";
        case X64_Mnemonic::POP:
            return "pop";
        case X64_Mnemonic::MOV:
            return "mov";
        case X64_Mnemonic::ADD:
            return "add";
        case X64_Mnemonic::SUB:
            return "sub";
        case X64_Mnemonic::MUL:
            return "mul";
        case X64_Mnemonic::IMUL:
            return "imul";
        case X64_Mnemonic::DIV:
            return "div";
        case X64_Mnemonic::IDIV:
            return "idiv";
        case X64_Mnemonic::AND:
            return "and";
        case X64_Mnemonic::OR:
            return "or";
        case X64_Mnemonic::XOR:
            return "xor";
        case X64_Mnemonic::SHL:
            return "shl";
        case X64_Mnemonic::SHR:
            return "shr";
        case X64_Mnemonic::SAR:
            return "sar";
        case X64_Mnemonic::CMP:
            return "cmp";
        case X64_Mnemonic::NOT:
            return "not";
        case X64_Mnemonic::NEG:
            return "neg";
        case X64_Mnemonic::MOVABS:
            return "movabs";
        case X64_Mnemonic::MOVSX:
            return "movsx";
        case X64_Mnemonic::MOVSXD:
            return "movsxd";
        case X64_Mnemonic::MOVZX:
            return "movzx";
        case X64_Mnemonic::JE:
            return "je";
        case X64_Mnemonic::JNE:
            return "jne";
        case X64_Mnemonic::JZ:
            return "jz";
        case X64_Mnemonic::JNZ:
            return "jnz";
        case X64_Mnemonic::JL:
            return "jl";
        case X64_Mnemonic::JLE:
            return "jle";
        case X64_Mnemonic::JG:
            return "jg";
        case X64_Mnemonic::JGE:
            return "jge";
        case X64_Mnemonic::JA:
            return "ja";
        case X64_Mnemonic::JAE:
            return "jae";
        case X64_Mnemonic::JB:
            return "jb";
        case X64_Mnemonic::JBE:
            return "jbe";
        case X64_Mnemonic::SETE:
            return "sete";
        case X64_Mnemonic::SETNE:
            return "setne";
        case X64_Mnemonic::SETZ:
            return "setz";
        case X64_Mnemonic::SETNZ:
            return "setnz";
        case X64_Mnemonic::SETL:
            return "setl";
        case X64_Mnemonic::SETLE:
            return "setle";
        case X64_Mnemonic::SETG:
            return "setg";
        case X64_Mnemonic::SETGE:
            return "setge";
        case X64_Mnemonic::SETA:
            return "seta";
        case X64_Mnemonic::SETAE:
            return "setae";
        case X64_Mnemonic::SETB:
            return "setb";
        case X64_Mnemonic::SETBE:
            return "setbe";
        case X64_Mnemonic::MOVS:
            return "movs";
        case X64_Mnemonic::MOVAP:
            return "movap";
        case X64_Mnemonic::UCOMIS:
            return "ucomis";
        case X64_Mnemonic::ADDS:
            return "adds";
        case X64_Mnemonic::SUBS:
            return "subs";
        case X64_Mnemonic::MULS:
            return "muls";
        case X64_Mnemonic::DIVS:
            return "divs";
        case X64_Mnemonic::ANDP:
            return "andp";
        case X64_Mnemonic::ORP:
            return "orp";
        case X64_Mnemonic::XORP:
            return "xorp";
        case X64_Mnemonic::CVTSS2SD:
            return "cvtss2sd";
        case X64_Mnemonic::CVTSD2SS:
            return "cvtsd2ss";
        case X64_Mnemonic::CVTSI2SS:
            return "cvtsi2ss";
        case X64_Mnemonic::CVTSI2SD:
            return "cvtsi2sd";
        case X64_Mnemonic::CVTTSS2SI:
            return "cvttss2si";
        case X64_Mnemonic::CVTTSD2SI:
            return "cvttsd2si";
        default:
            assert(false && "invalid mnemonic!");
    }
}

static const char* to_string(X64_Register reg, uint16_t subreg = 0) {
    switch (reg) {
        case RAX:
            switch (subreg) {
                case 8: return "rax";
                case 4: return "eax";
                case 2: return "ax";
                case 1: return "al";
                default: return "ah";
            }
        case RBX:
            switch (subreg) {
                case 8: return "rbx";
                case 4: return "ebx";
                case 2: return "bx";
                case 1: return "bl";
                default: return "bh";
            }
        case RCX:
            switch (subreg) {
                case 8: return "rcx";
                case 4: return "ecx";
                case 2: return "cx";
                case 1: return "cl";
                default: return "ch";
            }
        case RDX:
            switch (subreg) {
                case 8: return "rdx";
                case 4: return "edx";
                case 2: return "dx";
                case 1: return "dl";
                default: return "dh";
            }
        case RDI:
            switch (subreg) {
                case 8: return "rdi";
                case 4: return "edi";
                case 2: return "di";
                case 1: return "dil";
                default: return "";
            }
        case RSI:
            switch (subreg) {
                case 8: return "rsi";
                case 4: return "esi";
                case 2: return "si";
                case 1: return "sil";
                default: return "";
            }
        case RBP:
            switch (subreg) {
                case 8: return "rbp";
                case 4: return "ebp";
                case 2: return "bp";
                case 1: return "bpl";
                default: return "";
            }
        case RSP:
            switch (subreg) {
                case 8: return "rsp";
                case 4: return "esp";
                case 2: return "sp";
                case 1: return "spl";
                default: return "";
            }
        case R8:
            switch (subreg) {
                case 8: return "r8";
                case 4: return "r8d";
                case 2: return "r8w";
                case 1: return "r8b";
                default: return "";
            }
        case R9:
            switch (subreg) {
                case 8: return "r9";
                case 4: return "r9d";
                case 2: return "r9w";
                case 1: return "r9b";
                default: return "";
            }
        case R10:
            switch (subreg) {
                case 8: return "r10";
                case 4: return "r10d";
                case 2: return "r10w";
                case 1: return "r10b";
                default: return "";
            }
        case R11:
            switch (subreg) {
                case 8: return "r11";
                case 4: return "r11d";
                case 2: return "r11w";
                case 1: return "r11b";
                default: return "";
            }
        case R12:
            switch (subreg) {
                case 8: return "r12";
                case 4: return "r12d";
                case 2: return "r12w";
                case 1: return "r12b";
                default: return "";
            }
        case R13:
            switch (subreg) {
                case 8: return "r13";
                case 4: return "r13d";
                case 2: return "r13w";
                case 1: return "r13b";
                default: return "";
            }
        case R14:
            switch (subreg) {
                case 8: return "r14";
                case 4: return "r14d";
                case 2: return "r14w";
                case 1: return "r14b";
                default: return "";
            }
        case R15:
            switch (subreg) {
                case 8: return "r15";
                case 4: return "r15d";
                case 2: return "r15w";
                case 1: return "r15b";
                default: return "";
            }
        case RIP:
            return "rip";
        case XMM0:
            return "xmm0";
        case XMM1:
            return "xmm1";
        case XMM2:
            return "xmm2";
        case XMM3: 
            return "xmm3";
        case XMM4: 
            return "xmm4";
        case XMM5: 
            return "xmm5";
        case XMM6: 
            return "xmm6";
        case XMM7: 
            return "xmm7";
        case XMM8: 
            return "xmm8";
        case XMM9: 
            return "xmm9";
        case XMM10:
            return "xmm10";
        case XMM11:
            return "xmm11";
        case XMM12:
            return "xmm12";
        case XMM13:
            return "xmm13";
        case XMM14:
            return "xmm14";
        case XMM15:
            return "xmm15";
        default:
            assert(false && "invalid register!");
    }
}

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
    if (is_redundant_move(func, inst))
        return;

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

    if (func.get_function()->get_linkage() == Function::External)
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

        os << ' ' << integer->get_value();
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

    if (init)
        emit_constant(os, *init);
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

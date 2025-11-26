//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "spbe/graph/BasicBlock.hpp"
#include "spbe/graph/CFG.hpp"
#include "spbe/graph/Constant.hpp"
#include "spbe/graph/Function.hpp"
#include "spbe/machine/MachOperand.hpp"
#include "spbe/machine/MachFunction.hpp"
#include "spbe/machine/MachRegister.hpp"
#include "spbe/X64/X64.hpp"
#include "spbe/X64/X64AsmWriter.hpp"

#include <cmath>
#include <cstdint>
#include <cstring>

using namespace spbe;
using namespace spbe::x64;

const char* X64AsmWriter::opcode_to_string(x64::Opcode op) const {
    switch (op) {
    case NOP:         return "nop";
    case JMP:         return "jmp";
    case UD2:         return "ud2";
    case CQO:         return "cqo";
    case MOV:         return "mov ";
    case CALL64:      return "callq";
    case RET64:       return "retq";
    case LEA32:       return "leal";
    case LEA64:       return "leaq";
    case PUSH64:      return "pushq";
    case POP64:       return "popq";
    case MOV8:        return "movb";
    case MOV16:       return "movw";
    case MOV32:       return "movl";
    case MOV64:       return "movq";
    case ADD8:        return "addb";
    case ADD16:       return "addw";
    case ADD32:       return "addl";
    case ADD64:       return "addq";
    case SUB8:        return "subb";
    case SUB16:       return "subw";
    case SUB32:       return "subl";
    case SUB64:       return "subq";
    case MUL8:        return "mulb";
    case MUL16:       return "mulw";
    case MUL32:       return "mull";
    case MUL64:       return "mulq";
    case IMUL8:       return "imulb";
    case IMUL16:      return "imulw";
    case IMUL32:      return "imull";
    case IMUL64:      return "imulq";
    case DIV8:        return "divb";
    case DIV16:       return "divw";
    case DIV32:       return "divl";
    case DIV64:       return "divq";
    case IDIV8:       return "idivb";
    case IDIV16:      return "idivw";
    case IDIV32:      return "idivl";
    case IDIV64:      return "idivq";
    case AND8:        return "andb";
    case AND16:       return "andw";
    case AND32:       return "andl";
    case AND64:       return "andq";
    case OR8:         return "orb";
    case OR16:        return "orw";
    case OR32:        return "orl";
    case OR64:        return "orq";
    case XOR8:        return "xorb";
    case XOR16:       return "xorw";
    case XOR32:       return "xorl";
    case XOR64:       return "xorq";
    case SHL8:        return "shlb";
    case SHL16:       return "shlw";
    case SHL32:       return "shll";
    case SHL64:       return "shlq";
    case SHR8:        return "shrb";
    case SHR16:       return "shrw";
    case SHR32:       return "shrl";
    case SHR64:       return "shrq";
    case SAR8:        return "sarb";
    case SAR16:       return "sarw";
    case SAR32:       return "sarl";
    case SAR64:       return "sarq";
    case CMP8:        return "cmpb";
    case CMP16:       return "cmpw";
    case CMP32:       return "cmpl";
    case CMP64:       return "cmpq";
    case NOT8:        return "notb";
    case NOT16:       return "notw";
    case NOT32:       return "notl";
    case NOT64:       return "notq";
    case NEG8:        return "negb";
    case NEG16:       return "negw";
    case NEG32:       return "negl";
    case NEG64:       return "negq";
    case MOVABS:      return "movabs";
    case MOVSX:       return "movsx";
    case MOVSXD:      return "movsxd";
    case MOVZX:       return "movzx";
    case JE:          return "je";
    case JNE:         return "jne";
    case JZ:          return "jz";
    case JNZ:         return "jnz";
    case JL:          return "jl";
    case JLE:         return "jle";
    case JG:          return "jg";
    case JGE:         return "jge";
    case JA:          return "ja";
    case JAE:         return "jae";
    case JB:          return "jb";
    case JBE:         return "jbe";
    case SETE:        return "sete";
    case SETNE:       return "setne";
    case SETZ:        return "setz";
    case SETNZ:       return "setnz";
    case SETL:        return "setl";
    case SETLE:       return "setle";
    case SETG:        return "setg";
    case SETGE:       return "setge";
    case SETA:        return "seta";
    case SETAE:       return "setae";
    case SETB:        return "setb";
    case SETBE:       return "setbe";
    case MOVSS:       return "movss";
    case MOVSD:       return "movsd";
    case MOVAPS:      return "movaps";
    case MOVAPD:      return "movapd";
    case UCOMISS:     return "ucomiss";
    case UCOMISD:     return "ucomisd";
    case ADDSS:       return "addss";
    case ADDSD:       return "addsd";
    case SUBSS:       return "subss";
    case SUBSD:       return "subsd";
    case MULSS:       return "mulss";
    case MULSD:       return "mulsd";
    case DIVSS:       return "divss";
    case DIVSD:       return "divsd";
    case ANDPS:       return "andps";
    case ANDPD:       return "andpd";
    case ORPS:        return "orps";
    case ORPD:        return "orpd";
    case XORPS:       return "xorps";
    case XORPD:       return "xorpd";
    case CVTSS2SD:    return "cvtss2sd";
    case CVTSD2SS:    return "cvtsd2ss";
    case CVTSI2SS:    return "cvtsi2ss";
    case CVTSI2SD:    return "cvtsi2sd";
    case CVTTSS2SI8:  return "cvttss2sib";
    case CVTTSS2SI16: return "cvtss2siw";
    case CVTTSS2SI32: return "cvtss2sil";
    case CVTTSS2SI64: return "cvtss2siq";
    case CVTTSD2SI8:  return "cvtsd2sib";
    case CVTTSD2SI16: return "cvtsd2siw";
    case CVTTSD2SI32: return "cvtsd2sil";
    case CVTTSD2SI64: return "cvtsd2siq";
    default:
        assert(false && "unrecognized x64 opcode!");
    }
}

x64::Register X64AsmWriter::map_register(
        MachRegister reg, const MachFunction& MF) {
    if (reg.is_virtual())
        reg = MF.get_register_info().vregs.at(reg.id()).alloc;

    return static_cast<x64::Register>(reg.id());
}

bool X64AsmWriter::is_redundant_move(
        const MachFunction &MF, const MachInstruction& MI) {
    if (!is_move_opcode(static_cast<x64::Opcode>(MI.opcode())))
        return false;

    if (MI.num_operands() != 2)
        return false;

    const MachOperand& MOL = MI.get_operand(0);
    const MachOperand& MOR = MI.get_operand(1);

    if (!MOL.is_reg() || !MOR.is_reg())
        return false;

    x64::Register regl = map_register(MOL.get_reg(), MF);
    x64::Register regr = map_register(MOR.get_reg(), MF);
    return regl == regr && MOL.get_subreg() == MOR.get_subreg();
}

void X64AsmWriter::write_operand(
        std::ostream& os, const MachFunction& MF, const MachOperand& MO) {
    switch (MO.kind()) {
    case MachOperand::MO_Register: {
        os << '%' << to_string(map_register(MO.get_reg(), MF), MO.get_subreg());
        break;
    }

    case MachOperand::MO_Memory: {
        if (MO.get_mem_disp() != 0) 
            os << MO.get_mem_disp();

        os << "(%" << to_string(map_register(MO.get_mem_base(), MF), 8) << ')';
        break;
    }

    case MachOperand::MO_StackIdx: {
        const auto& stack = MF.get_stack_info();
        const auto& slot = stack.entries.at(MO.get_stack_index());

        os << (-slot.offset - static_cast<int32_t>(slot.size)) << "(%rbp)";
        break;
    }

    case MachOperand::MO_Immediate: {
        os << '$' << MO.get_imm();
        break;
    }

    case MachOperand::MO_BasicBlock: {
        os << ".LBB" << m_function << '_' << MO.get_mmb()->position();
        break;
    }
    
    case MachOperand::MO_ConstantIdx: {
        const auto& cpool = MF.get_constant_pool();
        const auto& constant = cpool.entries.at(MO.get_constant_index());

        os << ".LCPI" << m_function << '_' << MO.get_constant_index() 
           << "(%rip)";
        break;
    }
    
    case MachOperand::MO_Symbol: {
        os << MO.get_symbol();
        break;
    }
      
    default:
        assert(false && "unrecognized machine operand kind!");
    }
}

void X64AsmWriter::write_instruction(
        std::ostream& os, const MachFunction& MF, const MachInstruction& MI) {
    if (is_redundant_move(MF, MI))
        return;

    // If this is a return instruction, inject necessary epilogue parts.
    // TODO: Make this optional along with prologue injection.
    if (is_ret_opcode(static_cast<x64::Opcode>(MI.opcode()))) {
        os << "\taddq\t$" << MF.get_stack_info().alignment() << ",%rsp\n"
           << "\tpopq\t%rbp\n"
           << "\t.cfi_def_cfa %rsp,8\n"
           << "\tretq\n";
        return;
    }

    os << '\t' << opcode_to_string(static_cast<x64::Opcode>(MI.opcode())) 
       << '\t';

    // Write out all instruction operands.
    for (uint32_t idx = 0, e = MI.num_explicit_operands(); idx != e;) {
        write_operand(os, MF, MI.get_operand(idx));
        if (++idx != e) os << ',';
    }

    // Allow opt-out based on target.
    if (x64::is_call_opcode(static_cast<x64::Opcode>(MI.opcode())))
        os << "@PLT";

    os << '\n';
}

void X64AsmWriter::write_block(
        std::ostream& os, const MachFunction& MF, const MachBasicBlock& MBB) {
    if (!MBB.get_basic_block()->has_preds()) {
        // Basic blocks without predecessors (typically only the entry block)
        // can be emitted as just a comment.
        os << "#bb" << MBB.position();
    } else {
        os << ".LBB" << m_function << '_' << MBB.position();
    }

    os << ":\n";

    for (const auto& MI : MBB.insts()) write_instruction(os, MF, MI);
}

void X64AsmWriter::write_function(std::ostream& os, const MachFunction& MF) {
    const std::string& name = MF.get_name();

    os << "# begin function " << name << '\n';

    const auto& cpool = MF.get_constant_pool();
    int32_t last_size = -1;
    for (uint32_t idx = 0, e = cpool.num_entries(); idx != e; ++idx) {
        const auto& entry = cpool.entries.at(idx);
        const auto& constant = entry.constant;

        uint32_t size = MF.get_target().get_type_size(constant->get_type());
        if (size != last_size) {
            os << "\t.section\t.rodata.cst" << size << ",\"aM\",@progbits,8\n"
               << "\t.p2align\t" << std::log2(size) << ",0x0\n";

            last_size = size;
        }

        os << ".LCPI" << m_function << '_' << idx << ":\n";
        write_constant(os, *entry.constant);
    }

    os << "\t.text\n";

    if (MF.get_function()->get_linkage() == Function::LinkageType::External)
        os << "\t.global\t" << name << '\n';

    os << "\t.p2align 4\n"
       << "\t.type\t" << name << ",@function\n"
       << name << ":\n"
       << "\t.cfi_startproc\n"
       << "\tpushq\t%rbp\n"
       << "\t.cfi_def_cfa_offset 16\n"
       << "\t.cfi_offset %rbp,-16\n"
       << "\tmovq\t%rsp,%rbp\n"
       << "\t.cfi_def_cfa_register %rbp\n"
       << "\tsubq\t$" << MF.get_stack_info().alignment() << ",%rsp\n";

    for (const auto* MBB = MF.front(); MBB != nullptr; MBB = MBB->next())
        write_block(os, MF, *MBB);

    os << ".LFE" << m_function << ":\n"
       << "\t.size\t" << name << ", .LFE" << m_function << '-' << name << '\n'
       << "\t.cfi_endproc\n"
       << "# end function " << name << "\n\n";
}

void X64AsmWriter::write_constant(std::ostream& os, const Constant& constant) {
    const auto& target = m_object.get_target();
    const uint32_t size = target->get_type_size(constant.get_type());

    os << "\t.";

    if (auto integer = dynamic_cast<const ConstantInt*>(&constant)) {
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
    } else if (auto fp = dynamic_cast<const ConstantFP*>(&constant)) {
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

    } else if (auto null = dynamic_cast<const ConstantNull*>(&constant)) {
        os << "quad 0x0";
    } else if (auto string = dynamic_cast<const ConstantString*>(&constant)) {
        os << "string \"";

        for (uint32_t idx = 0, e = string->get_value().size(); idx != e; ++idx) {
            switch (string->get_value()[idx]) {
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
                os << string->get_value()[idx];
                break;
            }
        }

        os << '"';
    }

    os << '\n';
}

void X64AsmWriter::write_global(std::ostream& os, const Global& global) {
    // TODO: If global is uninitialized, use bss section.
    
    if (global.is_read_only()) {
        os << "\t.section\t.rodata\n";
    } else {
        os << "\t.data\n";
    }

    if (global.get_linkage() == Global::LinkageType::External)
        os << "\t.global" << global.get_name() << '\n';

    const auto& target = m_object.get_target();
    const auto& initializer = global.get_initializer();
    const uint32_t align = target->get_type_align(initializer->get_type()),
                   size = target->get_type_size(initializer->get_type());

    os << "\t.align\t" << align << '\n'
       << "\t.type\t" << global.get_name() << ",@object\n"
       << "\t.size\t" << global.get_name() << ',' << size << '\n'
       << global.get_name() << ":\n";

    write_constant(os, *initializer);
}

void X64AsmWriter::run(std::ostream& os) {
    os << "\t.file\t\"" << m_object.get_graph()->get_file() << "\"\n";

    for (const auto& global : m_object.get_graph()->globals())
        write_global(os, *global);

    for (const auto& [name, function] : m_object.functions()) {
        write_function(os, *function);
        ++m_function;
    }

    os << "\t.ident\t\t\"spbe: 0.1.0, nwm\"\n" 
       << "\t.section\t.note.GNU-stack,\"\",@progbits\n";
}

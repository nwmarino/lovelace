//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lir/machine/MachInst.hpp"
#include "lir/machine/MachLabel.hpp"
#include "lir/machine/MachOperand.hpp"

using namespace lir;

const char* lir::to_string(X64_Mnemonic op) {
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

const char* lir::to_string(X64_Size size) {
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

MachInst::MachInst(X64_Mnemonic op, X64_Size size, const Operands& ops, 
                   MachLabel* parent, const std::string& comment)
  : m_op(op), m_size(size), m_parent(parent), m_ops(ops), m_comment(comment) {
    if (m_parent)
        m_parent->append(*this);
}

const MachFunction* MachInst::get_function() const {
    return m_parent ? m_parent->get_parent() : nullptr;
}

uint32_t MachInst::num_defs() const {
    uint32_t num = 0;
    for (const auto& op : m_ops) {
        if (op.is_reg() && op.is_def()) 
            ++num;
    }

    return num;
}

uint32_t MachInst::num_implicit_operands() const {
    uint32_t num = 0;
    for (const auto& op : m_ops) {
        if (op.is_reg() && op.is_implicit()) 
            ++num;
    }

    return num;
}

uint32_t MachInst::num_explicit_operands() const {
    uint32_t num = 0;
    for (const auto& op : m_ops) {
        if (!op.is_reg() || !op.is_implicit()) 
            ++num;
    }

    return num;
}

uint32_t MachInst::num_implicit_defs() const {
    uint32_t num = 0;
    for (const auto& op : m_ops) {
        if (op.is_reg() && op.is_def() && op.is_implicit()) 
            ++num;
    }

    return num;
}

uint32_t MachInst::num_explicit_defs() const {
    uint32_t num = 0;
    for (const auto& op : m_ops) {
        if (op.is_reg() && op.is_def() && !op.is_implicit()) 
            ++num;
    }

    return num;
}

bool MachInst::has_implicit_def() const {
    for (const auto& op : m_ops) {
        if (op.is_reg() && op.is_implicit() && op.is_def())
            return true;
    }

    return false;
}

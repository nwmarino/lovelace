//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "spbe/machine/MachRegister.hpp"
#include "spbe/X64/X64.hpp"

#include <cassert>
#include <cstdint>
#include <string>
#include <unordered_map>

using namespace spbe;
using namespace spbe::x64;

bool x64::is_call_opcode(x64::Opcode opc) {
    return opc == x64::CALL64;
}

bool x64::is_ret_opcode(x64::Opcode opc) {
    return opc == x64::RET64;
}

bool x64::is_move_opcode(x64::Opcode opc) {
    switch (opc) {
    case MOV:
    case MOV8:
    case MOV16:
    case MOV32:
    case MOV64:
    case MOVSS:
    case MOVSD:
    case MOVAPS:
    case MOVAPD:
        return true;
    default:
        return false;
    }
}

bool x64::is_terminating_opcode(x64::Opcode opc) {
    switch (opc) {
    case JMP:
    case RET64:
    case JE:
    case JNE:
    case JZ:
    case JNZ:
    case JL:
    case JLE:
    case JG:
    case JGE:
    case JA:
    case JAE:
    case JB:
    case JBE:
        return true;
    default:
        return false;
    }
}

RegisterClass x64::get_class(Register reg) {
    switch (reg) {
    case RAX:
    case RBX:
    case RCX:
    case RDX:
    case RDI:
    case RSI:
    case R8:
    case R9:
    case R10:
    case R11:
    case R12:
    case R13:
    case R14:
    case R15:
    case RSP:
    case RBP:
    case RIP:
        return GeneralPurpose;
    case XMM0:
    case XMM1:
    case XMM2:
    case XMM3:
    case XMM4:
    case XMM5:
    case XMM6:
    case XMM7:
    case XMM8:
    case XMM9:
    case XMM10:
    case XMM11:
    case XMM12:
    case XMM13:
    case XMM14:
    case XMM15:
        return FloatingPoint;
    default:
        assert(false && "unrecognied x64 physical register!");
    }
}

bool x64::is_callee_saved(Register reg) {
    switch (reg) {
    case RBX:
    case R12:
    case R13:
    case R14:
    case R15:
    case RSP:
    case RBP:
        return true;
    default:
        return false;
    }
}

bool x64::is_caller_saved(Register reg) {
    switch (reg) {
    case RAX:
    case RCX:
    case RDX:
    case RDI:
    case RSI:
    case R8:
    case R9:
    case R10:
    case R11:
    case R12:
    case R13:
    case R14:
    case R15:
    case XMM0:
    case XMM1:
    case XMM2:
    case XMM3:
    case XMM4:
    case XMM5:
    case XMM6:
    case XMM7:
    case XMM8:
    case XMM9:
    case XMM10:
    case XMM11:
    case XMM12:
    case XMM13:
    case XMM14:
    case XMM15:
        return true;
    default:
        return false;
    }
}

TargetRegisters x64::get_registers() {
    RegisterSet gpr;
    gpr.cls = GeneralPurpose;
    gpr.regs = {
        RAX, RCX, RDX, RSI, RDI, R8, R9, 
        R10, R11, R12, R13, R14, R15
    };

    RegisterSet fpr;
    fpr.cls = FloatingPoint;
    fpr.regs = {
        XMM0, XMM1, XMM2, XMM3,
        XMM4, XMM5, XMM6, XMM7,
        XMM8, XMM9, XMM10, XMM11,
        XMM12, XMM13, XMM14, XMM15
    };

    TargetRegisters tregs;
    tregs.regs[GeneralPurpose] = gpr;
    tregs.regs[FloatingPoint] = fpr;
    return tregs;
}

x64::Opcode x64::parse_opcode(const std::string& str) {
    std::unordered_map<std::string, x64::Opcode> ops = {
        { "nop", x64::NOP },
        { "j", x64::JMP },
        { "ud2", x64::UD2 },
        { "cqo", x64::CQO },
        { "syscall", x64::SYSCALL },
        { "mov", x64::MOV },
        { "callq", x64::CALL64 },
        { "retq", x64::RET64 },
        { "leal", x64::LEA32 },
        { "leaq", x64::LEA64 },
        { "pushq", x64::PUSH64 },
        { "popq", x64::POP64 },
        { "movb", x64::MOV8 },
        { "movw", x64::MOV16 },
        { "movl", x64::MOV32 },
        { "movq", x64::MOV64 },
        { "addb", x64::ADD8 },
        { "addw", x64::ADD16 },
        { "addl", x64::ADD32 },
        { "addq", x64::ADD64 },
        { "subb", x64::SUB8 },
        { "subw", x64::SUB16 },
        { "subl", x64::SUB32 },
        { "subq", x64::SUB64 },
        { "mulb", x64::MUL8 },
        { "mulw", x64::MUL16 },
        { "mull", x64::MUL32 },
        { "mulq", x64::MUL64 },
        { "imulb", x64::IMUL8 },
        { "imulw", x64::IMUL16 },
        { "imull", x64::IMUL32 },
        { "imulq", x64::IMUL64 },
        { "divb", x64::DIV8 },
        { "divw", x64::DIV16 },
        { "divl", x64::DIV32 },
        { "divq", x64::DIV64 },
        { "idivb", x64::IDIV8 },
        { "idivw", x64::IDIV16 },
        { "idivl", x64::IDIV32 },
        { "idivq", x64::IDIV64 },
        { "andb", x64::AND8 },
        { "andw", x64::AND16 },
        { "andl", x64::AND32 },
        { "andq", x64::AND64 },
        { "orb", x64::OR8 },
        { "orw", x64::OR16 },
        { "orl", x64::OR32 },
        { "orq", x64::OR64 },
        { "xorb", x64::XOR8 },
        { "xorw", x64::XOR16 },
        { "xorl", x64::XOR32 },
        { "xorq", x64::XOR64 },
        { "shlb", x64::SHL8 },
        { "shlw", x64::SHL16 },
        { "shll", x64::SHL32 },
        { "shlq", x64::SHL64 },
        { "shrb", x64::SHR8 },
        { "shrw", x64::SHR16 },
        { "shrl", x64::SHR32 },
        { "shrq", x64::SHR64 },
        { "sarb", x64::SAR8 },
        { "sarw", x64::SAR16 },
        { "sarl", x64::SAR32 },
        { "sarq", x64::SAR64 },
        { "cmpb", x64::CMP8 },
        { "cmpw", x64::CMP16 },
        { "cmpl", x64::CMP32 },
        { "cmpq", x64::CMP64 },
        { "notb", x64::NOT8 },
        { "notw", x64::NOT16 },
        { "notl", x64::NOT32 },
        { "notq", x64::NOT64 },
        { "negb", x64::NEG8 },
        { "negw", x64::NEG16 },
        { "negl", x64::NEG32 },
        { "negq", x64::NEG64 },
        { "movabs", x64::MOVABS },
        { "movsx", x64::MOVSX },
        { "movsxd", x64::MOVSXD },
        { "movzx", x64::MOVZX },
        { "je", x64::JE },
        { "jne", x64::JNE },
        { "jz", x64::JZ },
        { "jnz", x64::JNZ },
        { "jl", x64::JL },
        { "jle", x64::JLE },
        { "jg", x64::JG },
        { "jge", x64::JGE },
        { "ja", x64::JA },
        { "jae", x64::JAE },
        { "jb", x64::JB },
        { "jbe", x64::JBE },
        { "sete", x64::SETE },
        { "setne", x64::SETNE },
        { "setz", x64::SETZ },
        { "setnz", x64::SETNZ },
        { "setl", x64::SETL },
        { "setle", x64::SETLE },
        { "setg", x64::SETG },
        { "setge", x64::SETGE },
        { "seta", x64::SETA },
        { "setae", x64::SETAE },
        { "setb", x64::SETB },
        { "setbe", x64::SETBE },
        { "movss", x64::MOVSS },
        { "movsd", x64::MOVSD },
        { "movaps", x64::MOVAPS },
        { "movapd", x64::MOVAPD },
        { "ucomiss", x64::UCOMISS },
        { "ucomisd", x64::UCOMISD },
        { "addss", x64::ADDSS },
        { "addsd", x64::ADDSD },
        { "subss", x64::SUBSS },
        { "subsd", x64::SUBSD },
        { "mulss", x64::MULSS },
        { "mulsd", x64::MULSD },
        { "divss", x64::DIVSS },
        { "divsd", x64::DIVSD },
        { "xorps", x64::XORPS },
        { "xorpd", x64::XORPD },
        { "cvtss2sd", x64::CVTSS2SD },
        { "cvtsd2ss", x64::CVTSD2SS },
        { "cvtsi2ss", x64::CVTSI2SS },
        { "cvtsi2sd", x64::CVTSI2SD },
        { "cvttss2sib", x64::CVTTSS2SI8 },
        { "cvttss2siw", x64::CVTTSS2SI16 },
        { "cvttss2sil", x64::CVTTSS2SI32 },
        { "cvttss2siq", x64::CVTTSS2SI64 },
        { "cvttsd2sib", x64::CVTTSD2SI8 },
        { "cvttsd2siw", x64::CVTTSD2SI16 },
        { "cvttsd2sil", x64::CVTTSD2SI32 },
        { "cvttsd2siq", x64::CVTTSD2SI64 },
    };

    auto it = ops.find(str);
    if (it != ops.end())
        return it->second;

    return x64::NO_OPC;
}

std::pair<x64::Register, uint16_t> x64::parse_register(const std::string& str) {
    std::unordered_map<std::string, std::pair<x64::Register, uint16_t>> regs = {
        { "rax",   { x64::RAX, 8 } },
        { "eax",   { x64::RAX, 4 } },
        { "ax",    { x64::RAX, 2 } },
        { "al",    { x64::RAX, 1 } },
        { "rbx",   { x64::RBX, 8 } },
        { "ebx",   { x64::RBX, 4 } },
        { "bx",    { x64::RBX, 2 } },
        { "bl",    { x64::RBX, 1 } },
        { "rcx",   { x64::RCX, 8 } },
        { "ecx",   { x64::RCX, 4 } },
        { "cx",    { x64::RCX, 2 } },
        { "cl",    { x64::RCX, 1 } },
        { "rdx",   { x64::RDX, 8 } },
        { "edx",   { x64::RDX, 4 } },
        { "dx",    { x64::RDX, 2 } },
        { "dl",    { x64::RDX, 1 } },
        { "rdi",   { x64::RDI, 8 } },
        { "edi",   { x64::RDI, 4 } },
        { "di",    { x64::RDI, 2 } },
        { "dil",   { x64::RDI, 1 } },
        { "rsi",   { x64::RSI, 8 } },
        { "esi",   { x64::RSI, 4 } },
        { "si",    { x64::RSI, 2 } },
        { "sil",   { x64::RSI, 1 } },
        { "rbp",   { x64::RBP, 8 } },
        { "ebp",   { x64::RBP, 4 } },
        { "bp",    { x64::RBP, 2 } },
        { "bpl",   { x64::RBP, 1 } },
        { "rsp",   { x64::RSP, 8 } },
        { "esp",   { x64::RSP, 4 } },
        { "sp",    { x64::RSP, 2 } },
        { "spl",   { x64::RSP, 1 } },
        { "r8",    { x64::R8, 8 } },
        { "r8d",   { x64::R8, 4 } },
        { "r8w",   { x64::R8, 2 } },
        { "r8b",   { x64::R8, 1 } },
        { "r9",    { x64::R9, 8 } },
        { "r9d",   { x64::R9, 4 } },
        { "r9w",   { x64::R9, 2 } },
        { "r9b",   { x64::R9, 1 } },
        { "r10",   { x64::R10, 8 } },
        { "r10d",  { x64::R10, 4 } },
        { "r10w",  { x64::R10, 2 } },
        { "r10b",  { x64::R10, 1 } },
        { "r11",   { x64::R11, 8 } },
        { "r11d",  { x64::R11, 4 } },
        { "r11w",  { x64::R11, 2 } },
        { "r11b",  { x64::R11, 1 } },
        { "r12",   { x64::R12, 8 } },
        { "r12d",  { x64::R12, 4 } },
        { "r12w",  { x64::R12, 2 } },
        { "r12b",  { x64::R12, 1 } },
        { "r13",   { x64::R13, 8 } },
        { "r13d",  { x64::R13, 4 } },
        { "r13w",  { x64::R13, 2 } },
        { "r13b",  { x64::R13, 1 } },
        { "r14",   { x64::R14, 8 } },
        { "r14d",  { x64::R14, 4 } },
        { "r14w",  { x64::R14, 2 } },
        { "r14b",  { x64::R14, 1 } },
        { "r15",   { x64::R15, 8 } },
        { "r15d",  { x64::R15, 4 } },
        { "r15w",  { x64::R15, 2 } },
        { "r15b",  { x64::R15, 1 } },
        { "rip",   { x64::RIP, 8 } },
        { "xmm0",  { x64::XMM0, 0 } },
        { "xmm1",  { x64::XMM1, 0 } },
        { "xmm2",  { x64::XMM2, 0 } },
        { "xmm3",  { x64::XMM3, 0 } },
        { "xmm4",  { x64::XMM4, 0 } },
        { "xmm5",  { x64::XMM5, 0 } },
        { "xmm6",  { x64::XMM6, 0 } },
        { "xmm7",  { x64::XMM7, 0 } },
        { "xmm8",  { x64::XMM8, 0 } },
        { "xmm9",  { x64::XMM9, 0 } },
        { "xmm10", { x64::XMM10, 0 } },
        { "xmm11", { x64::XMM11, 0 } },
        { "xmm12", { x64::XMM12, 0 } },
        { "xmm13", { x64::XMM13, 0 } },
        { "xmm14", { x64::XMM14, 0 } },
        { "xmm15", { x64::XMM15, 0 } },
    };

    auto it = regs.find(str);
    if (it != regs.end())
        return it->second;

    return { x64::NO_REG, 0 };
}

std::string x64::to_string(Opcode op) {
    switch (op) {
    case NOP:
        return "NOP";
    case JMP:
        return "JMP";
    case UD2:
        return "UD2";
    case CQO:
        return "CQO";
    case SYSCALL:
        return "SYSCALL";
    case MOV:
        return "MOV";
    case CALL64:
        return "CALL64";
    case RET64:
        return "RET64";
    case LEA32:
        return "LEA32";
    case LEA64:
        return "LEA64";
    case PUSH64:
        return "PUSH64";
    case POP64:
        return "POP64";
    case MOV8:
        return "MOV8";
    case MOV16:
        return "MOV16";
    case MOV32:
        return "MOV32";
    case MOV64:
        return "MOV64";
    case ADD8:
        return "ADD8";
    case ADD16:
        return "ADD16";
    case ADD32:
        return "ADD32";
    case ADD64:
        return "ADD64";
    case SUB8:
        return "SUB8";
    case SUB16:
        return "SUB16";
    case SUB32:
        return "SUB32";
    case SUB64:
        return "SUB64";
    case MUL8:
        return "MUL8";
    case MUL16:
        return "MUL16";
    case MUL32:
        return "MUL32";
    case MUL64:
        return "MUL64";
    case IMUL8:
        return "IMUL8";
    case IMUL16:
        return "IMUL16";
    case IMUL32:
        return "IMUL32";
    case IMUL64:
        return "IMUL64";
    case DIV8:
        return "DIV8";
    case DIV16:
        return "DIV16";
    case DIV32:
        return "DIV32";
    case DIV64:
        return "DIV64";
    case IDIV8:
        return "IDIV8";
    case IDIV16:
        return "IDIV16";
    case IDIV32:
        return "IDIV32";
    case IDIV64:
        return "IDIV64";
    case AND8:
        return "AND8";
    case AND16:
        return "AND16";
    case AND32:
        return "AND32";
    case AND64:
        return "AND64";
    case OR8:
        return "OR8";
    case OR16:
        return "OR16";
    case OR32:
        return "OR32";
    case OR64:
        return "OR64";
    case XOR8:
        return "XOR8";
    case XOR16:
        return "XOR16";
    case XOR32:
        return "XOR32";
    case XOR64:
        return "XOR64";
    case SHL8:
        return "SHL8";
    case SHL16:
        return "SHL16";
    case SHL32:
        return "SHL32";
    case SHL64:
        return "SHL64";
    case SHR8:
        return "SHR8";
    case SHR16:
        return "SHR16";
    case SHR32:
        return "SHR32";
    case SHR64:
        return "SHR64";
    case SAR8:
        return "SAR8";
    case SAR16:
        return "SAR16";
    case SAR32:
        return "SAR32";
    case SAR64:
        return "SAR64";
    case CMP8:
        return "CMP8";
    case CMP16:
        return "CMP16";
    case CMP32:
        return "CMP32";
    case CMP64:
        return "CMP64";
    case NOT8:
        return "NOT8";
    case NOT16:
        return "NOT16";
    case NOT32:
        return "NOT32";
    case NOT64:
        return "NOT64";
    case NEG8:
        return "NEG8";
    case NEG16:
        return "NEG16";
    case NEG32:
        return "NEG32";
    case NEG64:
        return "NEG64";
    case MOVABS:
        return "MOVABS";
    case MOVSX:
        return "MOVSX";
    case MOVSXD:
        return "MOVSXD";
    case MOVZX:
        return "MOVZX";
    case JE:
        return "JE";
    case JNE:
        return "JNE";
    case JZ:
        return "JZ";
    case JNZ:
        return "JNZ";
    case JL:
        return "JL";
    case JLE:
        return "JLE";
    case JG:
        return "JG";
    case JGE:
        return "JGE";
    case JA:
        return "JA";
    case JAE:
        return "JAE";
    case JB:
        return "JB";
    case JBE:
        return "JBE";
    case SETE:
        return "SETE";
    case SETNE:
        return "SETNE";
    case SETZ:
        return "SETZ";
    case SETNZ:
        return "SETNZ";
    case SETL:
        return "SETL";
    case SETLE:
        return "SETLE";
    case SETG:
        return "SETG";
    case SETGE:
        return "SETGE";
    case SETA:
        return "SETA";
    case SETAE:
        return "SETAE";
    case SETB:
        return "SETB";
    case SETBE:
        return "SETBE";
    case MOVSS:
        return "MOVSS";
    case MOVSD:
        return "MOVSD";
    case MOVAPS:
        return "MOVAPS";
    case MOVAPD:
        return "MOVAPD";
    case UCOMISS:
        return "UCOMISS";
    case UCOMISD:
        return "UCOMISD";
    case ADDSS:
        return "ADDSS";
    case ADDSD:
        return "ADDSD";
    case SUBSS:
        return "SUBSS";
    case SUBSD:
        return "SUBSD";
    case MULSS:
        return "MULSS";
    case MULSD:
        return "MULSD";
    case DIVSS:
        return "DIVSS";
    case DIVSD:
        return "DIVSD";
    case XORPS:
        return "XORPS";
    case XORPD:
        return "XORPD";
    case CVTSS2SD:
        return "CVTSS2SD";
    case CVTSD2SS:
        return "CVTSD2SS";
    case CVTSI2SS:
        return "CVTSI2SS";
    case CVTSI2SD:
        return "CVTSI2SD";
    case CVTTSS2SI8:
        return "CVTTSS2SI8";
    case CVTTSS2SI16:
        return "CVTTSS2SI16";
    case CVTTSS2SI32:
        return "CVTTSS2SI32";
    case CVTTSS2SI64:
        return "CVTTSS2SI64";
    case CVTTSD2SI8:
        return "CVTTSD2SI8";
    case CVTTSD2SI16:
        return "CVTTSD2SI16";
    case CVTTSD2SI32:
        return "CVTTSD2SI32";
    case CVTTSD2SI64:
        return "CVTTSD2SI64";
    default:
        assert(false && "unrecognized x64 opcode!");
    }
}

std::string x64::to_string(Register reg, uint16_t subreg) {
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
        assert(false && "unrecognized x64 physical register!");
    }
}

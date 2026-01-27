//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lir/machine/Register.hpp"

#include <cassert>

using namespace lir;

RegisterClass lir::get_register_class(X64_Register reg) {
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
            return RegisterClass::GeneralPurpose;
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
            return RegisterClass::FloatingPoint;
        default:
            assert(false && "invalid register!");
    }
}


const char* lir::to_string(X64_Register reg, uint16_t subreg) {
    switch (reg) {
        case RAX:
            switch (subreg) {
                case 8: return "rax";
                case 4: return "eax";
                case 2: return "ax";
                case 1: return "al";
                default: return "rax";
            }
        case RBX:
            switch (subreg) {
                case 8: return "rbx";
                case 4: return "ebx";
                case 2: return "bx";
                case 1: return "bl";
                default: return "rbx";
            }
        case RCX:
            switch (subreg) {
                case 8: return "rcx";
                case 4: return "ecx";
                case 2: return "cx";
                case 1: return "cl";
                default: return "rcx";
            }
        case RDX:
            switch (subreg) {
                case 8: return "rdx";
                case 4: return "edx";
                case 2: return "dx";
                case 1: return "dl";
                default: return "rdx";
            }
        case RDI:
            switch (subreg) {
                case 8: return "rdi";
                case 4: return "edi";
                case 2: return "di";
                case 1: return "dil";
                default: return "rdi";
            }
        case RSI:
            switch (subreg) {
                case 8: return "rsi";
                case 4: return "esi";
                case 2: return "si";
                case 1: return "sil";
                default: return "rsi";
            }
        case RBP:
            switch (subreg) {
                case 8: return "rbp";
                case 4: return "ebp";
                case 2: return "bp";
                case 1: return "bpl";
                default: return "rbp";
            }
        case RSP:
            switch (subreg) {
                case 8: return "rsp";
                case 4: return "esp";
                case 2: return "sp";
                case 1: return "spl";
                default: return "rsp";
            }
        case R8:
            switch (subreg) {
                case 8: return "r8";
                case 4: return "r8d";
                case 2: return "r8w";
                case 1: return "r8b";
                default: return "r8";
            }
        case R9:
            switch (subreg) {
                case 8: return "r9";
                case 4: return "r9d";
                case 2: return "r9w";
                case 1: return "r9b";
                default: return "r9";
            }
        case R10:
            switch (subreg) {
                case 8: return "r10";
                case 4: return "r10d";
                case 2: return "r10w";
                case 1: return "r10b";
                default: return "r10";
            }
        case R11:
            switch (subreg) {
                case 8: return "r11";
                case 4: return "r11d";
                case 2: return "r11w";
                case 1: return "r11b";
                default: return "r11";
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
                default: return "r13";
            }
        case R14:
            switch (subreg) {
                case 8: return "r14";
                case 4: return "r14d";
                case 2: return "r14w";
                case 1: return "r14b";
                default: return "r14";
            }
        case R15:
            switch (subreg) {
                case 8: return "r15";
                case 4: return "r15d";
                case 2: return "r15w";
                case 1: return "r15b";
                default: return "r15";
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

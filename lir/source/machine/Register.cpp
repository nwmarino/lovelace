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

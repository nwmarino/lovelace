//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lir/machine/MachOperand.hpp"
#include "lir/machine/Register.hpp"

#include <cassert>

using namespace lir;

MachOperand MachOperand::create_reg(Register reg, uint16_t subreg,
                                    bool is_def, bool is_implicit, 
                                    bool is_kill, bool is_dead) {
    assert(!(is_dead && !is_def));
    assert(!(is_kill && is_def));

    MachOperand operand = {};
    operand.m_kind = Reg;
    operand.m_reg = reg;
    operand.m_subreg = subreg;
    operand.m_is_def = is_def;
    operand.m_is_implicit = is_implicit;
    operand.is_kill_or_dead = is_kill | is_dead;
    return operand;
}

MachOperand MachOperand::create_mem(Register reg, int32_t disp) {
    MachOperand operand = {};
    operand.m_kind = Memory;
    operand.m_mem.reg = reg;
    operand.m_mem.disp = disp;
    return operand;
}

MachOperand MachOperand::create_stack_ref(uint32_t idx) {
    MachOperand operand = {};
    operand.m_kind = Stack;
    operand.m_stack = idx;
    return operand;
}

MachOperand MachOperand::create_imm(int64_t imm) {
    MachOperand operand = {};
    operand.m_kind = Immediate;
    operand.m_imm = imm;
    return operand;
}

MachOperand MachOperand::create_label(MachLabel* label) {
    MachOperand operand = {};
    operand.m_kind = Label;
    operand.m_label = label;
    return operand;
}

MachOperand MachOperand::create_constant_ref(uint32_t idx) {
    MachOperand operand = {};
    operand.m_kind = Constant;
    operand.m_const = idx;
    return operand;
}

MachOperand MachOperand::create_symbol(const char* symbol) {
    MachOperand operand = {};
    operand.m_kind = Symbol;
    operand.m_symbol = symbol;
    return operand;
}

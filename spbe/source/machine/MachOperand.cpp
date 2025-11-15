#include "../../include/machine/MachOperand.hpp"
#include "../../include/machine/MachRegister.hpp"
#include "../../include/target/x64.hpp"

#include <cassert>

using namespace spbe;

MachOperand MachOperand::create_reg(MachRegister reg, uint16_t subreg,
                                    bool is_def, bool is_implicit, 
                                    bool is_kill, bool is_dead) {
    assert(!(is_dead && !is_def));
    assert(!(is_kill && is_def));

    MachOperand operand = {};
    operand.m_kind = MO_Register;
    operand.m_reg = reg;
    operand.m_subreg = subreg;
    operand.m_is_def = is_def;
    operand.m_is_implicit = is_implicit;
    operand.is_kill_or_dead = is_kill | is_dead;
    return operand;
}

MachOperand MachOperand::create_mem(MachRegister reg, int32_t disp) {
    MachOperand operand = {};
    operand.m_kind = MO_Memory;
    operand.m_mem.reg = reg;
    operand.m_mem.disp = disp;
    return operand;
}

MachOperand MachOperand::create_stack_index(uint32_t idx) {
    MachOperand operand = {};
    operand.m_kind = MO_StackIdx;
    operand.m_stack_idx = idx;
    return operand;
}

MachOperand MachOperand::create_imm(int64_t imm) {
    MachOperand operand = {};
    operand.m_kind = MO_Immediate;
    operand.m_imm = imm;
    return operand;
}

MachOperand MachOperand::create_block(MachBasicBlock* mbb) {
    MachOperand operand = {};
    operand.m_kind = MO_BasicBlock;
    operand.m_mbb = mbb;
    return operand;
}

MachOperand MachOperand::create_constant_index(uint32_t idx) {
    MachOperand operand = {};
    operand.m_kind = MO_ConstantIdx;
    operand.m_constant_idx = idx;
    return operand;
}

MachOperand MachOperand::create_symbol(const char* symbol) {
    MachOperand operand = {};
    operand.m_kind = MO_Symbol;
    operand.m_symbol = symbol;
    return operand;
}

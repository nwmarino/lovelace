#include "../../include/machine/MachBasicBlock.hpp"
#include "../../include/machine/MachInstruction.hpp"
#include "../../include/machine/MachOperand.hpp"

using namespace spbe;

MachInstruction::MachInstruction(uint32_t opcode, 
                                 const std::vector<MachOperand>& operands, 
                                 MachBasicBlock* parent)
        : m_opcode(opcode), m_operands(operands), m_parent(parent) {
    if (m_parent != nullptr)
        m_parent->push_back(*this);
}

const MachFunction* MachInstruction::get_mf() const {
    return m_parent ? m_parent->get_parent() : nullptr;
}

uint32_t MachInstruction::num_defs() const {
    uint32_t num = 0;
    for (const auto& mo : m_operands)
        if (mo.is_reg() && mo.is_def()) ++num;

    return num;
}

uint32_t MachInstruction::num_implicit_operands() const {
    uint32_t num = 0;
    for (const auto& mo : m_operands)
        if (mo.is_reg() && mo.is_implicit()) ++num;

    return num;
}

uint32_t MachInstruction::num_explicit_operands() const {
    uint32_t num = 0;
    for (const auto& mo : m_operands)
        if (!mo.is_reg() || !mo.is_implicit()) ++num;

    return num;
}

uint32_t MachInstruction::num_implicit_defs() const {
    uint32_t num = 0;
    for (const auto& mo : m_operands)
        if (mo.is_reg() && mo.is_def() && mo.is_implicit()) ++num;

    return num;
}

uint32_t MachInstruction::num_explicit_defs() const {
    uint32_t num = 0;
    for (const auto& mo : m_operands)
        if (mo.is_reg() && mo.is_def() && !mo.is_implicit()) ++num;

    return num;
}

bool MachInstruction::has_implicit_def() const {
    for (const auto& mo : m_operands)
        if (mo.is_reg() && mo.is_implicit() && mo.is_def())
            return true;

    return false;
}

//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lir/machine/MachInst.hpp"
#include "lir/machine/MachLabel.hpp"
#include "lir/machine/MachOperand.hpp"

using namespace lir;

MachInst::MachInst(X64_Mnemonic op, X64_Size size, const Operands& ops, 
                   MachLabel* parent)
  : m_op(op), m_size(size), m_parent(parent), m_ops(ops) {
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

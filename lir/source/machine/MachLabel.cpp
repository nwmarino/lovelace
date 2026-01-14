//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lir/machine/MachFunction.hpp"
#include "lir/machine/MachLabel.hpp"

using namespace lir;

MachLabel::MachLabel(const BasicBlock* block, MachFunction* parent)
  : m_block(block), m_parent(parent) {
    if (m_parent)
        m_parent->append(this);
}

uint32_t MachLabel::position() const {
    uint32_t num = 0;
    const MachLabel* prev = m_prev;
    while (prev) {
        prev = prev->get_prev();
        ++num;
    }

    return num;
}

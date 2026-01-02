//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "spbe/machine/MachBasicBlock.hpp"
#include "spbe/machine/MachFunction.hpp"

using namespace spbe;

MachBasicBlock::MachBasicBlock(const BasicBlock* bb, MachFunction* parent)
        : m_bb(bb), m_parent(parent) {
    if (m_parent != nullptr)
        m_parent->append(this);
}

uint32_t MachBasicBlock::position() const {
    uint32_t num = 0;
    const MachBasicBlock* prev = m_prev;
    while (prev != nullptr) {
        prev = prev->prev();
        ++num;
    }

    return num;
}

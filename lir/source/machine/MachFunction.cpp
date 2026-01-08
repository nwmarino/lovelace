//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lir/graph/Function.hpp"
#include "lir/machine/MachFunction.hpp"

using namespace lir;

MachFunction::MachFunction(const Function* function, const Machine& mach)
  : m_function(function), m_mach(mach) {}

MachFunction::~MachFunction() {
    MachLabel* curr = m_head;
    while (curr) {
        MachLabel* tmp = curr->get_next();

        curr->set_prev(nullptr);
        curr->set_next(nullptr);
        delete curr;

        curr = tmp;
    }

    m_head = m_tail = nullptr;
}

const std::string& MachFunction::get_name() const {
    assert(m_function);
    return m_function->get_name();
}

const MachLabel* MachFunction::at(uint32_t i) const {
    const MachLabel* curr = m_head;
    uint32_t pos = 0;

    while (pos++ < i) {
        assert(curr && "index out of bounds!");
        curr = curr->get_next();
    }

    return curr;
}

uint32_t MachFunction::size() const {
    uint32_t size = 0;
    for (MachLabel* curr = m_head; curr; curr = curr->get_next()) 
        ++size;

    return size;
}

void MachFunction::prepend(MachLabel* label) {
    assert(label && "label cannot be null!");

    if (m_head) {
        m_head->set_prev(label);
        label->set_next(m_head);
        m_head = label;
    } else {    
        m_head = m_tail = label;
    }

    label->set_parent(this);
}

void MachFunction::append(MachLabel* label) {
    assert(label && "label cannot be null!");
    
    if (m_tail) {
        m_tail->set_next(label);
        label->set_prev(m_tail);
        m_tail = label;
    } else {
        m_head = m_tail = label;
    }

    label->set_parent(this);
}

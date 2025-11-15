#include "../../include/graph/Function.hpp"
#include "../../include/machine/MachFunction.hpp"

using namespace spbe;

MachFunction::MachFunction(const Function* fn, const Target& target)
    : m_fn(fn), m_target(target) {}

MachFunction::~MachFunction() {
    auto curr = m_front;
    while (curr != nullptr) {
        auto tmp = curr->next();
        curr->m_prev = nullptr;
        curr->m_next = nullptr;
        delete curr;

        curr = tmp;
    }

    m_front = m_back = nullptr;
}

const std::string& MachFunction::get_name() const {
    assert(m_fn != nullptr && 
        "machine function does not have an SPBE function!");
    return m_fn->get_name();
}

const MachBasicBlock* MachFunction::at(uint32_t idx) const {
    uint32_t pos = 0;
    for (auto* curr = m_front; curr != nullptr; curr = curr->next())
        if (pos++ == idx) return curr;

    return nullptr;
}

MachBasicBlock* MachFunction::at(uint32_t idx) {
    uint32_t pos = 0;
    for (auto* curr = m_front; curr != nullptr; curr = curr->next())
        if (pos++ == idx) return curr;

    return nullptr;
}

uint32_t MachFunction::size() const {
    uint32_t size = 0;
    for (auto* curr = m_front; curr != nullptr; curr = curr->next()) 
        ++size;

    return size;
}

void MachFunction::prepend(MachBasicBlock* mbb) {
    assert(mbb && "basic block cannot be null!");

    if (m_front != nullptr) {
        m_front->set_prev(mbb);
        mbb->set_next(m_front);
        m_front = mbb;
    } else {    
        m_front = mbb;
        m_back = mbb;
    }

    mbb->set_parent(this);
}

void MachFunction::append(MachBasicBlock* mbb) {
    assert(mbb && "basic block cannot be null!");
    
    if (m_back != nullptr) {
        m_back->set_next(mbb);
        mbb->set_prev(m_back);
        m_back = mbb;
    } else {
        m_front = mbb;
        m_back = mbb;
    }

    mbb->set_parent(this);
}

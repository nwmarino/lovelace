#include "graph/Basicblock.hpp"
#include "graph/Function.hpp"
#include "graph/Instruction.hpp"

using namespace spbe;

BasicBlock::BasicBlock(Function* parent) : m_parent(parent) {
    if (parent != nullptr)
        parent->push_back(this);
}

BasicBlock::~BasicBlock() {
    Instruction* curr = m_front;
    while (curr) {
        Instruction* tmp = curr->next();
        delete curr;
        curr = tmp;
    }

    m_front = m_back = nullptr;
    m_prev = m_next = nullptr;
    m_preds.clear();
    m_succs.clear();
}

void BasicBlock::append_to_function(Function* parent) {
    assert(parent && "new parent function cannot be null!");
    assert(!m_parent && "basic block already belongs to a function!");

    parent->push_back(this);
    m_parent = parent;
}

void BasicBlock::insert_before(BasicBlock* blk) {
    assert(blk && "blk cannot be null!");
    assert(!m_parent && "basic block already belongs to a function!");

    m_prev = blk->m_prev;
    m_next = blk;

    if (blk->m_prev != nullptr)
        blk->m_prev->m_next = this;

    blk->m_prev = this;
    m_parent = blk->m_parent;
}

void BasicBlock::insert_after(BasicBlock* blk) {
    assert(blk && "blk cannot be null!");
    assert(!m_parent && "basic block already belongs to a function!");

    m_prev = blk;
    m_next = blk->m_next;

    if (blk->m_next != nullptr)
        blk->m_next->m_prev = this;

    blk->m_next = this;
    m_parent = blk->m_parent;
}

void BasicBlock::remove_inst(Instruction* inst) {
    for (auto& curr = m_front; curr; curr = curr->next()) {
        if (curr != inst)
            continue;

        if (curr->prev())
            curr->prev()->set_next(inst->next());

        if (curr->next())
            curr->next()->set_prev(inst->prev());

        inst->set_prev(nullptr);
        inst->set_next(nullptr);
        inst->clear_parent();
    }
}

void BasicBlock::detach_from_parent() {
    if (m_parent != nullptr)
        m_parent->remove(this);
    
    m_prev = m_next = nullptr;
    m_parent = nullptr;
}

void BasicBlock::push_front(Instruction* inst) {
    assert(inst && "inst cannot be null!");

    if (m_front != nullptr) {
        inst->set_next(m_front);
        m_front->set_prev(inst);
        m_front = inst;
    } else {
        m_front = m_back = inst;
    }

    inst->set_parent(this);
}

void BasicBlock::push_back(Instruction* inst) {
    assert(inst && "inst cannot be null!");

    if (m_back != nullptr) {
        inst->set_prev(m_back);
        m_back->set_next(inst);
        m_back = inst;
    } else {
        m_front = m_back = inst;
    }

    inst->set_parent(this);
}

void BasicBlock::insert(Instruction* inst, uint32_t i) {
    assert(inst && "inst cannot be null!");
    
    uint32_t position = 0;
    for (auto& curr = m_front; curr != nullptr; curr = curr->next())
        if (position++ == i) return inst->insert_before(curr);

    push_back(inst);
}

void BasicBlock::insert(Instruction* inst, Instruction* insert_after) {
    inst->insert_after(insert_after);
}

uint32_t BasicBlock::get_number() const {
    uint32_t num = 0;
    const BasicBlock* curr = m_prev;
    while (curr != nullptr) {
        curr = curr->prev();
        num++;
    }

    return num;
}

bool BasicBlock::terminates() const {
    for (auto curr = m_back; curr != nullptr; curr = curr->prev())
        if (curr->is_terminator()) return true;

    return false;
}

uint32_t BasicBlock::terminators() const {
    uint32_t num = 0;
    for (auto curr = m_front; curr != nullptr; curr = curr->next())
        if (curr->is_terminator()) ++num;

    return num;
}

const Instruction* BasicBlock::terminator() const {
    for (auto curr = m_front; curr != nullptr; curr = curr->next())
        if (curr->is_terminator()) return curr;

    return nullptr;
}

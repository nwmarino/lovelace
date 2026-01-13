//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lir/graph/BasicBlock.hpp"
#include "lir/graph/Instruction.hpp"

using namespace lir;

void Instruction::detach() {
    assert(m_parent && "cannot detach a free-floating instruction!");
    m_parent->remove(this);
}

void Instruction::prepend_to(BasicBlock* block) {
    assert(block && "block cannot be null");
    block->prepend(this);
}

void Instruction::append_to(BasicBlock* block) {
    assert(block && "block cannot be null");
    block->append(this);
}

void Instruction::insert_before(Instruction* inst) {
    assert(inst && "inst cannot be null!");

    if (inst->get_prev())
        inst->get_prev()->set_next(this);

    m_prev = inst->get_prev();
    m_next = inst;

    inst->set_prev(this);
    m_parent = inst->get_parent();
}

void Instruction::insert_after(Instruction* inst) {
    assert(inst && "inst cannot be null!");

    if (inst->get_next())
        inst->get_next()->set_prev(this);

    m_prev = inst;
    m_next = inst->get_next();

    inst->set_next(this);
    m_parent = inst->get_parent();
}

bool Instruction::is_terminator() const {
    switch (m_op) {
        case OP_ABORT:
        case OP_JIF:
        case OP_JMP:
        case OP_RET:
        case OP_UNREACHABLE:
            return true;
        default:
            return false;
    }
}

bool Instruction::is_trivially_dead() const {
    return false; // @Todo: no DCE for now.

    if (get_def() == 0 || Value::used())
        return false;

    //@ Todo: not quite right.
    return m_op != OP_CALL;
}

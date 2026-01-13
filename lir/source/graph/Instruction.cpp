//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lir/graph/BasicBlock.hpp"
#include "lir/graph/Constant.hpp"
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

std::vector<const Value*> Instruction::get_jump_args() const {
    assert(op() == OP_JMP);

    std::vector<const Value*> args = {};
    args.reserve(num_operands() - 1);

    for (uint32_t i = 1, e = num_operands(); i < e; ++i) // Skip label.
        args.push_back(get_operand(i));

    return args;
}

std::vector<const Value*> Instruction::get_jif_true_args() const {
    assert(op() == OP_JIF);

    std::vector<const Value*> args = {};

    for (uint32_t i = 2, e = num_operands(); i < e; ++i) { // Skip condition and "true" label.
        const Value* oper = get_operand(i);
        if (dynamic_cast<const BlockAddress*>(oper))
            break; // Stop at first label.
        
        args.push_back(oper);
    }

    return args;
}

std::vector<const Value*> Instruction::get_jif_false_args() const {
    assert(op() == OP_JIF);

    uint32_t i = 2, e = num_operands(); // i: skip condition and first label.
    for (; i < e; ++i) {
        if (dynamic_cast<const BlockAddress*>(get_operand(i)))
            break; // Stop at second label.
    }

    i += 1; // move past the second label.

    std::vector<const Value*> args = {};
    for (; i < e; ++i)
        args.push_back(get_operand(i));
    
    return args;
}

bool Instruction::is_trivially_dead() const {
    return false; // @Todo: no DCE for now.

    if (get_def() == 0 || Value::used())
        return false;

    //@ Todo: not quite right.
    return m_op != OP_CALL;
}

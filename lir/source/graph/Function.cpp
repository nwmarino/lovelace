//
//  Copyright (c) 2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/graph/BasicBlock.hpp"
#include "lir/graph/CFG.hpp"
#include "lir/graph/Function.hpp"

using namespace lir;

Function::~Function() {
    for (Parameter *param : m_params)
        delete param;

    for (auto &[name, local] : m_locals)
        delete local;

    BasicBlock *curr = m_head;
    while (curr) {
        BasicBlock *tmp = curr->get_next();

        curr->set_prev(nullptr);
        curr->set_next(nullptr);
        delete curr;

        curr = tmp;
    }

    m_head = nullptr, m_tail = nullptr;
}

Function *Function::create(CFG &cfg, LinkageType linkage, FunctionType *type, 
                           const std::string &name, const Params &params) {
    Function* func = new Function(type, &cfg, linkage, name, params);
    assert(func);

    cfg.add_function(func);

    // Set the parent of each parameter to the newly created function (since 
    // the constructor doesn't propogate that info).
    for (Parameter *param : params) {
        assert(!param->has_parent() && 
            "parameter already belongs to a function!");

        param->set_parent(func);
    }

    return func;
}

void Function::detach() {
    assert(m_parent && "function does not have a parent graph!");

    m_parent->remove_function(this); // Updates parent pointer automatically.
}

bool Function::add_parameter(Parameter *param) {
    assert(!param->has_parent() && "parameter already belongs to a function!");

    if (param->is_named()) {
        if (get_parameter(param->get_name()))
            return false;
    }

    m_params.push_back(param);
    param->set_parent(this);
    return true;
}

const Local *Function::get_local(const std::string &name) const {
    auto it = m_locals.find(name);
    if (it != m_locals.end())
        return it->second;

    return nullptr;
}

bool Function::add_local(Local *local) {
    assert(!local->has_parent() && "local already belongs to a function!");

    if (get_local(local->get_name()))
        return false;
    
    m_locals.emplace(local->get_name(), local);
    local->set_parent(this);
    return true;
}

void Function::remove_local(Local *local) {
    assert(local && "local cannot be null!");

    if (local->get_parent() == this) {
        auto it = m_locals.find(local->get_name());
        if (it != m_locals.end())
            m_locals.erase(it);
    }
}

void Function::prepend(BasicBlock *block) {
    assert(block && "block cannot be null!");
    assert(!block->has_parent() && "block already belongs to a function!");

    if (m_head) {
        block->set_next(m_head);
        m_head->set_prev(block);
        m_head = block;
    } else {
        m_head = m_tail = block;
    }

    block->set_parent(this);
}

void Function::append(BasicBlock *block) {
    assert(block && "block cannot be null!");
    assert(!block->has_parent() && "block already belongs to a function!");

    if (m_tail) {
        block->set_prev(m_tail);
        m_tail->set_next(block);
        m_tail = block;
    } else {
        m_head = m_tail = block;
    }

    block->set_parent(this);
}

void Function::insert(BasicBlock *block, uint32_t i) {
    assert(block && "block cannot be null!");
    assert(!block->has_parent() && "block already belongs to a function!");

    uint32_t pos = 0;
    for (BasicBlock* curr = m_head; curr; curr = curr->get_next()) {
        if (pos == i)
            return block->insert_before(curr);

        pos++;
    }

    append(block);
}

void Function::insert(BasicBlock *block, BasicBlock *insert_after) {
    assert(block && "block cannot be null!");

    block->insert_after(insert_after);
}

void Function::remove(BasicBlock *block) {
    assert(block && "block cannot be null!");

    if (block->get_parent() == this) {
        // Update the next pointer for the block before the one to remove.
        if (block->get_prev()) {
            block->get_prev()->set_next(block->get_next());
        } else {
            if (block->get_next()) {
                // Block is at the front of the function, so update the head.
                m_head = block->get_next();
            } else {
                // Block was the last in the function.
                m_head = m_tail = nullptr;
            }
        }

        // Update the previous pointer for the block after the one to remove.
        if (block->get_next()) {
            block->get_next()->set_prev(block->get_prev());
        } else {
            if (block->get_prev()) {
                // Block is at the back of the function, so update the tail.
                m_tail = block->get_prev();
            } else {
                // Block was the last in the function.
                m_head = m_tail = nullptr;
            }
        }

        block->set_prev(nullptr);
        block->set_next(nullptr);
        block->clear_parent();
    }
}

uint32_t Function::size() const {
    uint32_t size = 0;

    const BasicBlock *curr = m_head;
    while (curr) {
        curr = curr->get_next();
        size++;
    }

    return size;
}

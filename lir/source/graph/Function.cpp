//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lir/graph/BasicBlock.hpp"
#include "lir/graph/CFG.hpp"
#include "lir/graph/Function.hpp"

using namespace lir;

FunctionArgument* FunctionArgument::create(
        Type *type, const std::string &name, Function* parent, Trait trait) {
    FunctionArgument* arg = new FunctionArgument(type, parent, name, trait);
    if (parent)
        parent->append_arg(arg);

    return arg;
}

uint32_t FunctionArgument::get_index() const {
    assert(m_parent && "argument does not belong to a function!");

    const Function::Args& args = m_parent->get_args();
    uint32_t i = 0;
    for (FunctionArgument* arg : args) {
        if (arg == this)
            return i;

        ++i;
    }

    __builtin_unreachable(); // The argument should be in the list...
}

Function::~Function() {
    for (FunctionArgument* arg : m_args)
        delete arg;

    for (auto& [name, local] : m_locals)
        delete local;

    m_args.clear();
    m_locals.clear();

    BasicBlock* curr = m_head;
    while (curr != nullptr) {
        BasicBlock* tmp = curr->get_next();

        curr->set_prev(nullptr);
        curr->set_next(nullptr);
        delete curr;

        curr = tmp;
    }

    m_head = m_tail = nullptr;
}

Function* Function::create(CFG &cfg, LinkageType linkage, FunctionType *type, 
                           const std::string &name, const Args &args) {
    Function* function = new Function(type, &cfg, linkage, name, args);
    cfg.add_function(function);

    for (FunctionArgument* arg : args) {
        assert(!arg->has_parent() && "argument already belongs to a function!");
        arg->set_parent(function);
    }

    return function;
}

void Function::detach() {
    assert(m_parent && "function does not have a parent graph!");

    m_parent->remove_function(this); // Updates parent pointer for us.
}

void Function::set_arg(uint32_t i, FunctionArgument* arg) {
    assert(i < num_args() && "index out of bounds!");
    assert(!arg->has_parent() && "argument already belongs to a function!");

    m_args[i] = arg;
    arg->set_parent(this);
}

void Function::append_arg(FunctionArgument* arg) {
    assert(!arg->has_parent() && "argument already belongs to a function!");

    m_args.push_back(arg);
    arg->set_parent(this);
}

const Local* Function::get_local(const std::string& name) const {
    auto it = m_locals.find(name);
    if (it != m_locals.end())
        return it->second;

    return nullptr;
}

void Function::add_local(Local* local) {
    assert(!get_local(local->get_name()) &&
        "local with same name already exists in function!");
    
    m_locals.emplace(local->get_name(), local);
    local->set_parent(this);
}

void Function::remove_local(Local* local) {
    assert(local && "local cannot be null!");

    auto it = m_locals.find(local->get_name());
    if (it != m_locals.end())
        m_locals.erase(it);
}

void Function::prepend(BasicBlock* block) {
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

void Function::append(BasicBlock* block) {
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

void Function::insert(BasicBlock* block, uint32_t i) {
    assert(block && "block cannot be null!");
    assert(!block->has_parent() && "block already belongs to a function!");

    uint32_t pos = 0;
    for (BasicBlock* curr = m_head; curr; curr = curr->get_next()) {
        if (pos == i)
            return block->insert_before(curr);

        ++pos;
    }

    append(block);
}

void Function::insert(BasicBlock* block, BasicBlock* insert_after) {
    assert(block && "block cannot be null!");

    block->insert_after(insert_after);
}

void Function::remove(BasicBlock* block) {
    assert(block && "block cannot be null!");
    assert(block->get_parent() == this && 
        "block does not belong to this function!");

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

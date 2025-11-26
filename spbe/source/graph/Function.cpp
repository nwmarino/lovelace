//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "spbe/graph/BasicBlock.hpp"
#include "spbe/graph/CFG.hpp"
#include "spbe/graph/Function.hpp"

using namespace spbe;

Argument::Argument(const Type* type, const std::string& name, uint32_t number, 
                   Function* parent)
    : Value(type), m_name(name), m_number(number), m_parent(parent) {}

Function::Function(CFG& cfg, LinkageType linkage, const FunctionType* type,
                   const std::string& name, const std::vector<Argument*>& args)
    : Value(type), m_linkage(linkage), m_name(name), m_args(args) {

    for (uint32_t idx = 0, e = args.size(); idx != e; ++idx) {
        args[idx]->set_number(idx);
        args[idx]->set_parent(this);
    }

    cfg.add_function(this);
}

Function::~Function() {
    for (auto& arg : m_args) {
        delete arg;
        arg = nullptr;
    }

    for (auto& [name, local] : m_locals) {
        delete local;
        local = nullptr;
    }

    m_args.clear();
    m_locals.clear();

    BasicBlock* curr = m_front;
    while (curr != nullptr) {
        BasicBlock* tmp = curr->next();

        curr->set_prev(nullptr);
        curr->set_next(nullptr);
        delete curr;

        curr = tmp;
    }

    m_front = m_back = nullptr;
}

void Function::detach_from_parent() {
    assert(m_parent != nullptr && "function does not have a parent graph!");

    m_parent->remove_function(this);
    m_parent = nullptr;
}

void Function::set_arg(uint32_t i, Argument* arg) {
    assert(i <= num_args() && "index out of bounds!");

    m_args[i] = arg;
    arg->set_number(i);
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
        "local with name already exists in function!");
    
    m_locals.emplace(local->get_name(), local);
    local->set_parent(this);
}

void Function::remove_local(Local* local) {
    assert(local && "local cannot be null!");

    auto it = m_locals.find(local->get_name());
    if (it != m_locals.end())
        m_locals.erase(it);
}

void Function::push_front(BasicBlock* blk) {
    assert(blk && "block cannot be null!");

    if (m_front != nullptr) {
        blk->set_next(m_front);
        m_front->set_prev(blk);
        m_front = blk;
    } else {
        m_front = m_back = blk;
    }
}

void Function::push_back(BasicBlock* blk) {
    assert(blk && "block cannot be null!");

    if (m_back != nullptr) {
        blk->set_prev(m_back);
        m_back->set_next(blk);
        m_back = blk;
    } else {
        m_front = m_back = blk;
    }
}

void Function::insert(BasicBlock* blk, uint32_t idx) {
    uint32_t pos = 0;
    for (auto& curr = m_front; curr != nullptr; curr = curr->next()) {
        if (pos == idx) {
            blk->insert_before(curr);
            return;
        }

        ++pos;
    }

    push_back(blk);
}

void Function::insert(BasicBlock* blk, BasicBlock* insert_after) {
    blk->insert_after(insert_after);
}

void Function::remove(BasicBlock* blk) {
    for (auto& curr = m_front; curr != nullptr; curr = curr->next()) {
        if (curr != blk)
            continue;

        if (curr->prev())
            curr->prev()->set_next(blk->next());

        if (curr->next())
            curr->next()->set_prev(blk->prev());

        blk->set_prev(nullptr);
        blk->set_next(nullptr);
        blk->clear_parent();
    } 
}

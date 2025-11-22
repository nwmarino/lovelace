//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "ast/Decl.hpp"
#include "ast/Scope.hpp"

#include <cassert>

using namespace scc;

void Scope::add(Decl* decl) {
    assert(!contains(decl->name()) && 
        "symbol with name already exists in scope!");

    m_decls.emplace(decl->name(), decl);
}

Decl* Scope::get(const std::string& name) const {
    auto it = m_decls.find(name);
    if (it != m_decls.end())
        return it->second;

    if (has_parent())
        return m_parent->get(name);

    return nullptr;
}

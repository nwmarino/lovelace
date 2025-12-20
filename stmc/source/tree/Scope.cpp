//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/tree/Decl.hpp"
#include "stmc/tree/Scope.hpp"

using namespace stm;

bool Scope::add(const NamedDecl* decl) {
    if (get(decl->get_name()))
        return false;

    m_symbols.emplace(decl->get_name(), decl);
    return true;
}

const NamedDecl* Scope::get(const string& name) const {
    auto it = m_symbols.find(name);
    if (it != m_symbols.end())
        return it->second;

    if (has_parent())
        return m_parent->get(name);

    return nullptr;
}

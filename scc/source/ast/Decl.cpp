//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "ast/Decl.hpp"
#include "ast/Stmt.hpp"

using namespace scc;

DeclContext::~DeclContext() {
    for (auto decl : m_decls)
        delete decl;

    m_decls.clear();
}

const NamedDecl* DeclContext::get_decl(const string& name) const {
    for (auto decl : m_decls) {
        const NamedDecl* nd = dynamic_cast<const NamedDecl*>(decl);
        if (nd && nd->get_name() == name)
            return nd;
    }

    return nullptr;
}

VariableDecl::~VariableDecl() {
    if (has_init()) {
        delete m_init;
        m_init = nullptr;
    }
}

FunctionDecl::~FunctionDecl() {
    if (has_body()) {
        delete m_body;
        m_body = nullptr;
    }
}

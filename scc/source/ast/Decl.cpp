//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "scc/ast/Decl.hpp"
#include "scc/ast/Stmt.hpp"
#include "scc/ast/TypeContext.hpp"

using namespace scc;

Decl::Decl(DeclContext* dctx, Kind kind, const SourceSpan& span) 
        : m_kind(kind), m_span(span) {
    if (dctx) {
        dctx->add(this);
    }
}

DeclContext::~DeclContext() {
    for (Decl* decl : m_decls)
        delete decl;

    m_decls.clear();
}

const NamedDecl* DeclContext::get_decl(const string& name) const {
    for (Decl* decl : m_decls) {
        const NamedDecl* nd = dynamic_cast<const NamedDecl*>(decl);
        if (nd && nd->get_name() == name)
            return nd;
    }

    if (has_parent())
        return m_parent->get_decl(name);

    return nullptr;
}

const TagTypeDecl* DeclContext::get_tag(const string& name) const {
    for (TagTypeDecl* tag : m_tags) {
        if (tag->get_name() == name)
            return tag;
    }

    if (has_parent())
        return m_parent->get_tag(name);

    return nullptr;
}

void DeclContext::add(Decl* decl) {
    if (decl->get_kind() == Decl::Record || decl->get_kind() == Decl::Enum) {
        m_tags.push_back(static_cast<TagTypeDecl*>(decl));
    } else {
        m_decls.push_back(decl); 
    }
}

TranslationUnitDecl::TranslationUnitDecl(const string& file)
    : DeclContext(), Decl(nullptr, Kind::TranslationUnit, SourceSpan(file)), 
      m_tctx(new TypeContext()) {}

TranslationUnitDecl::~TranslationUnitDecl() {
    delete m_tctx;
    m_tctx = nullptr;   
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

//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/tree/Decl.hpp"
#include "stmc/tree/Expr.hpp"
#include "stmc/tree/Scope.hpp"

using namespace stm;

LoadDecl* LoadDecl::create(Context& ctx, SourceSpan span, const string& path) {
    return new LoadDecl(span, path);
}

TranslationUnitDecl::TranslationUnitDecl(const string& file)
    : Decl(SourceLocation()), m_file(file), m_scope(new Scope()) {}

TranslationUnitDecl::~TranslationUnitDecl() {
    delete m_scope;
    m_scope = nullptr;
    
    for (Decl* decl : m_decls)
        delete decl;

    m_decls.clear();
}

TranslationUnitDecl* TranslationUnitDecl::create(const string &file) {
    return new TranslationUnitDecl(file);
}

NamedDecl::~NamedDecl() {
    for (Rune* rune : m_runes)
        delete rune;

    m_runes.clear();
}

VariableDecl::~VariableDecl() {
    if (has_init()) {
        delete m_init;
        m_init = nullptr;
    }
}

VariableDecl* VariableDecl::create(
        Context &ctx, SourceSpan span, const string &name, const RuneVec &runes, 
        const TypeUse &type, Expr *init) {
    return new VariableDecl(span, name, runes, type, init);
}

ParameterDecl* ParameterDecl::create(
        Context &ctx, SourceSpan span, const string &name, const TypeUse &type) {
    return new ParameterDecl(span, name, type);
}

FunctionDecl::~FunctionDecl() {
    delete m_scope;
    m_scope = nullptr;

    for (ParameterDecl* param : m_params)
        delete param;

    m_params.clear();

    if (has_body()) {
        delete m_body;
        m_body = nullptr;
    }
}

FunctionDecl* FunctionDecl::create(
        Context &ctx, SourceSpan span, const string &name, const RuneVec &runes, 
        const TypeUse &type, Scope *scope, const Params& params, 
        BlockStmt* body) {
    return new FunctionDecl(span, name, runes, type, scope, params, body);
}

FieldDecl* FieldDecl::create(
        Context &ctx, SourceSpan span, const string &name, const RuneVec &runes, 
        const TypeUse &type) {
    return new FieldDecl(span, name, runes, type);
}

VariantDecl* VariantDecl::create(
        Context &ctx, SourceSpan span, const string &name, const RuneVec &runes, 
        const TypeUse &type, int64_t value) {
    return new VariantDecl(span, name, runes, type, value);
}

AliasDecl* AliasDecl::create(
        Context &ctx, SourceSpan span, const string &name, const RuneVec &runes, 
        const Type *type) {
    return new AliasDecl(span, name, runes, type);
}

StructDecl::~StructDecl() {
    for (FieldDecl* field : m_fields)
        delete field;

    m_fields.clear();
}

StructDecl* StructDecl::create(
        Context &ctx, SourceSpan span, const string &name, const RuneVec &runes, 
        const Type *type) {
    return new StructDecl(span, name, runes, type);
}

EnumDecl::~EnumDecl() {
    for (VariantDecl* variant : m_variants)
        delete variant;

    m_variants.clear();
}

EnumDecl* EnumDecl::create(
        Context &ctx, SourceSpan span, const string &name, const RuneVec &runes, 
        const Type *type) {
    return new EnumDecl(span, name, runes, type);
}

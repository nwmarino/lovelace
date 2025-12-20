//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/parser/Parser.hpp"
#include "stmc/lexer/Lexer.hpp"
#include "stmc/tree/Context.hpp"
#include "stmc/tree/Decl.hpp"
#include "stmc/tree/Scope.hpp"
#include "stmc/tree/Type.hpp"

#include <cstring>

using namespace stm;

Parser::Parser(Diagnostics& diags, const string& path)
        : m_diags(diags), m_file(path), m_lexer(diags, path) {
    m_diags.set_path(path);
}

Parser::Parser(Diagnostics& diags, const string& path, const string& source)
        : m_diags(diags), m_file(path), m_lexer(diags, path, source) {
    m_diags.set_path(path);
}

TranslationUnitDecl* Parser::parse() {
    m_unit = TranslationUnitDecl::create(m_file);
    m_context = &m_unit->get_context();
    m_scope = m_unit->get_scope();

    next(); // Lex the first token.

    while (!m_lexer.is_eof()) {
        Decl* decl = parse_initial_declaration();
        if (decl)
            m_unit->get_decls().push_back(decl);
    }

    return m_unit;
}

bool Parser::match(Token::Kind kind) const {
    return last().kind == kind;
}

bool Parser::match(const char* keyword) const {
    return last().kind == Token::Identifier && last().value == keyword;
}

bool Parser::expect(Token::Kind kind) {
    if (!match(kind))
        return false;

    next();
    return true;
}

bool Parser::expect(const char* keyword) {
    if (!match(Token::Identifier))
        return false;

    if (0 != std::strcmp(last().value.data(), keyword))
        return false;

    next();
    return true;
}

Scope* Parser::enter_scope() {
    m_scope = new Scope(m_scope);
    return m_scope;
}

void Parser::exit_scope() {
    m_scope = m_scope->get_parent();
}

void Parser::parse_type_specifier(TypeUse& type) {
    if (!match(Token::Identifier))
        m_diags.fatal("expected identifier", loc());

    unordered_map<string, const Type*> types = {
        { "void", BuiltinType::get(*m_context, BuiltinType::Void) },
        { "bool", BuiltinType::get(*m_context, BuiltinType::Bool) },
        { "char", BuiltinType::get(*m_context, BuiltinType::Char) },
        { "s8", BuiltinType::get(*m_context, BuiltinType::Int8) },
        { "s16", BuiltinType::get(*m_context, BuiltinType::Int16) },
        { "s32", BuiltinType::get(*m_context, BuiltinType::Int32) },
        { "s64", BuiltinType::get(*m_context, BuiltinType::Int64) },
        { "u8", BuiltinType::get(*m_context, BuiltinType::UInt8) },
        { "u16", BuiltinType::get(*m_context, BuiltinType::UInt16) },
        { "u32", BuiltinType::get(*m_context, BuiltinType::UInt32) },
        { "u64", BuiltinType::get(*m_context, BuiltinType::UInt64) },
        { "f32", BuiltinType::get(*m_context, BuiltinType::Float32) },
        { "f64", BuiltinType::get(*m_context, BuiltinType::Float64) },
    };

    type.set_type(types[last().value]);
    next();
}

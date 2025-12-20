//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef STATIM_PARSER_H_
#define STATIM_PARSER_H_

#include "stmc/core/Diagnostics.hpp"
#include "stmc/lexer/Lexer.hpp"
#include "stmc/tree/Context.hpp"
#include "stmc/tree/Decl.hpp"
#include "stmc/tree/Expr.hpp"
#include "stmc/types/SourceLocation.hpp"

namespace stm {

/// Definition of a parser for a statim translation unit into a syntax tree.
class Parser final {
    Diagnostics& m_diags;
    string m_file;
    Lexer m_lexer;
    TranslationUnitDecl* m_unit = nullptr;
    Context* m_context = nullptr;
    Scope* m_scope = nullptr;
    vector<Rune*> m_runes = {};

    /// Returns the last token to be lexed.
    const Token& last() const { return m_lexer.get_last(); }

    /// Lex the next token.
    void next() { m_lexer.lex(); }

    /// Returns the most recent source location parsed.
    SourceLocation loc() { return last().loc; }

    /// Returns the span of source code since \p loc.
    SourceSpan since(SourceLocation loc) { 
        return SourceSpan(loc, last().loc); 
    }

    /// Test the kind of the current token to match with \p kind.
    bool match(Token::Kind kind) const;

    /// Test the kind of the current token to be an identifier of value
    /// \p keyword.
    bool match(const char* keyword) const;

    /// Expect the current token to be of kind \p kind. 
    ///
    /// If the token is a match, it will be consumed and the function will
    /// return true.
    bool expect(Token::Kind kind);

    /// Expect the current token to be an identifier of value \p keyword.
    ///
    /// If the token is a match, it will be consumed and the function will
    /// return true.
    bool expect(const char* keyword);

    /// Enter a new scope, with the current scope \c mScope as the parent.
    Scope* enter_scope();

    /// Exit the current scope, and move up to the parent node, if there is one.
    void exit_scope();

    /// Returns the closest equivelant binary operator for the token \p kind.
    BinaryOp::Operator get_binary_oper(Token::Kind kind) const;

    /// Returns the precedence for the binary operator equivelant of the token
    /// \p kind, and if \p kind is not a binary operator, returns -1.
    int32_t get_binary_oper_precedence(Token::Kind kind) const;

    /// Returns the closest equivelant unary operator for the token \p kind.
    UnaryOp::Operator get_unary_oper(Token::Kind kind) const;

    void parse_rune_decorator_list();
    
    TypeUse parse_type();

    Decl* parse_initial_declaration();
    Decl* parse_binding_declaration(const Token name);
    Decl* parse_load_declaration();
    
    Stmt* parse_initial_statement();
    Stmt* parse_inline_assembly_statement();
    Stmt* parse_block_statement();
    Stmt* parse_control_statement();
    Stmt* parse_declarative_statement();

    Expr* parse_initial_expression();
    Expr* parse_primary_expression();
    Expr* parse_identifier_expression();
    Expr* parse_prefix_unary_operator();
    Expr* parse_postfix_unary_operator();
    Expr* parse_binary_operator(Expr* base, int32_t precedence);

    Expr* parse_boolean_literal();
    Expr* parse_integer_literal();
    Expr* parse_floating_point_literal();
    Expr* parse_character_literal();
    Expr* parse_string_literal();
    Expr* parse_null_pointer_literal();

    Expr* parse_type_cast();
    Expr* parse_parentheses();
    Expr* parse_sizeof_operator();
    Expr* parse_named_reference();

public:
    Parser(Diagnostics& diags, const string& path);

    Parser(Diagnostics& diags, const string& path, const string& source);

    TranslationUnitDecl* parse();
};

} // namespace stm

#endif // STATIM_PARSER_H_

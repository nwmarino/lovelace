//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SCC_PARSER_H_
#define SCC_PARSER_H_

//
// This header file defines the parser that turns tokens created by the lexer
// into an abstract syntax tree on a per-translation unit basis.
//

#include "ast/Decl.hpp"
#include "ast/Expr.hpp"
#include "ast/QualType.hpp"
#include "ast/TypeContext.hpp"
#include "lexer/Lexer.hpp"
#include "lexer/Token.hpp"

#include <string>

namespace scc {

using std::string;

class Parser final {
    string m_file;
    Lexer m_lexer;
    TypeContext* m_tctx = nullptr;
    DeclContext* m_dctx = nullptr;

    /// Attempt to match the kind of the current token with \p kind. Returns
    /// true if the match is a success, and false otherwise. 
    bool match(TokenKind kind) const;

    /// Attempt to match the value of the current token with the keyword \p kw.
    /// Returns true if the token is an identifier and the value matches, and
    /// false otherwise.
    bool match(const char* kw) const;

    /// Attempt to lex the next token from source.
    void next();

    /// Skip the next \p n tokens.
    void skip(uint32_t n);

    /// Creates and returns a span between the location of the current token
    /// and \p loc as a starting point.
    Span since(const SourceLocation& loc);

    /// Returns the equivelant binary operator for the token kind \p kind.
    BinaryExpr::Op get_binary_operator(TokenKind kind) const;

    /// Returns the equivelant unary operator for the token kind \p kind.
    UnaryExpr::Op get_unary_operator(TokenKind kind) const;

    /// Returns the precedence for the binary operator equivelant of the token
    /// kind \p kind. Returns -1 for non-operator tokens.
    int32_t get_binary_operator_precedence(TokenKind kind) const;

    /// Returns true if \p ident is a reserved C keyword.
    bool is_reserved(const string& ident) const;

    /// Returns true if \p ident is a keyword reserved for storage classes.
    bool is_storage_class(const string& ident) const;

    /// Returns true if \p ident corresponds to some known type at the current
    /// state of the parser.
    bool is_typedef(const string& ident) const;

    /// Attemot to parse a storage class identifier.
    StorageClass parse_storage_class();

    /// Attempt to parse a possible qualified type. Returns true if the parse
    /// was successful and a typesu could be parsed, and false otherwise.
    bool parse_type(QualType& ty);

    Decl* parse_decl();
    Decl* parse_function(const SourceLocation& start, StorageClass sclass, 
                         const QualType& ty, const string& name);
    Decl* parse_variable(const SourceLocation& start, StorageClass sclass, 
                         const QualType& ty, const string& name);
    Decl* parse_typedef();
    Decl* parse_struct();
    Decl* parse_enum();

    Expr* parse_expr();
    Expr* parse_primary();
    Expr* parse_integer();
    Expr* parse_float();
    Expr* parse_character();
    Expr* parse_string();
    Expr* parse_binary(Expr* base, int32_t precedence);
    Expr* parse_unary_prefix();
    Expr* parse_unary_postfix();
    Expr* parse_ref();
    Expr* parse_sizeof();
    Expr* parse_ternary(Expr* base);

    Stmt* parse_stmt();
    Stmt* parse_compound();
    Stmt* parse_if();
    Stmt* parse_return();
    Stmt* parse_while();
    Stmt* parse_for();
    Stmt* parse_switch();

public:
    Parser(const string& file, const string& source = "");

    Parser(const Parser&) = delete;
    Parser& operator = (const Parser&) = delete;

    TranslationUnitDecl* parse();
};

} // namespace scc

#endif // SCC_PARSER_H_

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
#include "ast/Scope.hpp"
#include "core/TranslationUnit.hpp"
#include "lexer/Lexer.hpp"
#include "lexer/Token.hpp"

#include <memory>
#include <string>

namespace scc {

class Parser final {
    std::string m_file;
    Lexer m_lexer;
    Context* m_context = nullptr;
    Scope* m_scope = nullptr;

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

    /// Enter a new arbitrary scope.
    std::unique_ptr<Scope> enter_scope();

    /// Exit the current scope, and move up to its parent, if there is one.
    void exit_scope();

    /// Returns the equivelant binary operator for the token kind \p kind.
    BinaryExpr::Op get_binary_operator(TokenKind kind) const;

    /// Returns the equivelant unary operator for the token kind \p kind.
    UnaryExpr::Op get_unary_operator(TokenKind kind) const;

    /// Returns the precedence for the binary operator equivelant of the token
    /// kind \p kind. Returns -1 for non-operator tokens.
    int32_t get_binary_operator_precedence(TokenKind kind) const;

    /// Returns true if \p ident is a reserved C keyword.
    bool is_reserved(const std::string& ident) const;

    /// Returns true if \p ident is a keyword reserved for storage classes.
    bool is_storage_class(const std::string& ident) const;

    /// Returns true if \p ident corresponds to some known type at the current
    /// state of the parser.
    bool is_typedef(const std::string& ident) const;

    /// Attemot to parse a storage class identifier.
    StorageClass parse_storage_class();

    /// Attempt to parse a possible qualified type. Returns true if the parse
    /// was successful and a type could be parsed, and false otherwise.
    bool parse_type(QualType& ty);

    std::unique_ptr<Decl> parse_decl();
    std::unique_ptr<Decl> parse_function(const SourceLocation& start, 
                                         StorageClass sclass, 
                                         const QualType& ty, 
                                         const std::string& name);
    std::unique_ptr<Decl> parse_variable(const SourceLocation& start, 
                                         StorageClass sclass, 
                                         const QualType& ty, 
                                         const std::string& name);

    std::unique_ptr<Expr> parse_expr();
    std::unique_ptr<Expr> parse_primary();
    std::unique_ptr<Expr> parse_integer();
    std::unique_ptr<Expr> parse_float();
    std::unique_ptr<Expr> parse_character();
    std::unique_ptr<Expr> parse_string();
    std::unique_ptr<Expr> parse_binary(std::unique_ptr<Expr> base, 
                                       int32_t precedence);
    std::unique_ptr<Expr> parse_unary_prefix();
    std::unique_ptr<Expr> parse_unary_postfix();
    std::unique_ptr<Expr> parse_ref();
    std::unique_ptr<Expr> parse_call();
    std::unique_ptr<Expr> parse_cast();
    std::unique_ptr<Expr> parse_sizeof();

    std::unique_ptr<Stmt> parse_stmt();
    std::unique_ptr<Stmt> parse_compound();
    std::unique_ptr<Stmt> parse_if();
    std::unique_ptr<Stmt> parse_return();

public:
    Parser(const std::string& file, const std::string& source = "");

    Parser(const Parser&) = delete;
    Parser& operator = (const Parser&) = delete;

    ~Parser() = default;

    void parse(TranslationUnit& unit);
};

} // namespace scc

#endif // SCC_PARSER_H_

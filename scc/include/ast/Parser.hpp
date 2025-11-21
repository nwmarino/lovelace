#ifndef SCC_PARSER_H_
#define SCC_PARSER_H_

#include "Decl.hpp"
#include "Expr.hpp"
#include "Scope.hpp"
#include "TranslationUnit.hpp"
#include "../lexer/Lexer.hpp"
#include "../lexer/Token.hpp"

#include <memory>
#include <string>

namespace scc {

class Parser final {
    std::string m_file;
    Lexer m_lexer;
    std::shared_ptr<Scope> m_scope = nullptr;

    bool match(TokenKind kind) const;

    bool match(const char* kw) const;

    void next();

    void skip(uint32_t n);

    Span since(const SourceLocation& loc);

    std::shared_ptr<Scope> enter_scope();

    void exit_scope();

    std::unique_ptr<Decl> parse_decl();
    std::unique_ptr<Decl> parse_function();
    std::unique_ptr<Decl> parse_variable();
    std::unique_ptr<Decl> parse_struct();
    std::unique_ptr<Decl> parse_enum();
    std::unique_ptr<Decl> parse_union();

    std::unique_ptr<Expr> parse_expr();

public:
    Parser(const std::string& file);

    Parser(const Parser&) = delete;
    Parser& operator = (const Parser&) = delete;

    ~Parser() = default;

    void parse(TranslationUnit& unit);
};

} // namespace scc

#endif // SCC_PARSER_H_

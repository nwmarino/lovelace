#ifndef SCC_LEXER_H_
#define SCC_LEXER_H_

#include "Token.hpp"
#include "../core/Span.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace scc {

class Lexer final {
    friend class Parser;

    std::string m_file;
    std::string m_buffer;
    std::vector<Token> m_history;
    SourceLocation m_loc;
    uint32_t m_pos;

    char current() const;

    char peek(uint32_t n = 1) const;

    void move(uint32_t n = 1);

    void end_line();

public:
    Lexer() = default;

    Lexer(const std::string& file, const std::string& source = "");

    Lexer(const Lexer&) = delete;
    Lexer& operator = (const Lexer&) = delete;

    ~Lexer() = default;

    /// Returns the last n-th token created by this lexer.
    const Token& last(uint32_t n = 0) const;

    /// Lex a new token.
    const Token& lex();

    /// Returns true if this lexer has reached the end of its source.
    bool is_eof() const;
};

} // namespace scc

#endif // SCC_LEXER_H_

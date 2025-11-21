//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SCC_LEXER_H_
#define SCC_LEXER_H_

//
// This header file declares the lexer, a component of the compiler that turns
// source code into tokens recognized by the parser.
//

#include "lexer/Token.hpp"
#include "core/Span.hpp"

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace scc {

class Lexer final {
    friend class Parser;

    /// The buffer of source code this lexer is working on.
    std::string m_buf;

    /// The history of tokens lexed by this lexer instance.
    std::vector<Token> m_history;

    /// The current location of source code this lexer is at.
    SourceLocation m_loc;
    
    /// The position of the buffer \p m_buf this lexer is at.
    uint32_t m_pos;

    /// Returns the character at position \c m_pos in the buffer \c m_buf.
    char current() const {
        assert(!is_eof() && "iterator has exceeded buffer size!");
        return m_buf[m_pos];
    }

    /// Peek at the upcoming \p n characters in the buffer \c m_buf.
    char peek(uint32_t n = 1) const {
        assert(m_pos + n < m_buf.size() && "iterator would exceed buffer size!");
        return m_buf[m_pos + n];
    }

    /// Move the iterator \c m_pos \p n positions forward. Also updates
    void move(uint32_t n = 1) {
        m_pos += n;
        m_loc.column += n;
    }

    /// End the current line and update \c m_loc.
    void end_line() {
        ++m_loc.line;
        m_loc.column = 1;
    }

public:
    /// Create a new lexer, representing source file \p file. Providing a
    /// \p source argument will override any source contained in \p file.   
    Lexer(const std::string& file, const std::string& source = "");

    Lexer(const Lexer&) = delete;
    Lexer& operator = (const Lexer&) = delete;

    ~Lexer() = default;

    /// Returns the token created by this lexer \p n tokens ago.
    const Token& last(uint32_t n = 0) const {
        std::size_t size = m_history.size();
        assert((size - n - 1) > 0 && 
            "lexer has not lexed that many tokens yet!");

        return m_history[size - n - 1];
    }

    /// Lex and return a new token.
    const Token& lex();

    /// Returns true if this lexer has reached the end of its source.
    bool is_eof() const { return m_pos >= m_buf.size(); }
};

} // namespace scc

#endif // SCC_LEXER_H_

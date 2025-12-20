//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef STATIM_LEXER_H_
#define STATIM_LEXER_H_

#include "stmc/core/Diagnostics.hpp"
#include "stmc/lexer/Token.hpp"
#include "stmc/tools/Files.hpp"

#include <cassert>
#include <string>
#include <vector>

namespace stm {

using std::string;
using std::vector;

/// A lexical analyzer that turns source code into minimal tokens to be used
/// by the parser.
class Lexer final {
    Diagnostics& m_diags;
    string m_file;
    string m_source;
    SourceLocation m_location = {};
    vector<Token> m_history = {};

    /// The position in the \c m_source this lexer is at.
    uint32_t m_position = 0;

    /// Returns the current character \c m_position is pointing at.
    char get_current() const { return is_eof() ? '\0' : m_source[m_position]; }

    /// Returns the character \p n positions ahead in the source code buffer.
    /// If \p n exceeds the buffer size, then the null terminator is returned.
    char peek(uint32_t n = 1) const {
        return (m_position + n >= m_source.size()) 
            ? '\0' 
            : m_source[m_position + n];
    }

    /// Move the iterator \c m_position \p n positions ahead, and update the
    /// column field of \c m_location accordingly.
    void move(uint32_t n = 1) {
        m_position += n;
        m_location.col += n;
    }

    /// End the current line and update \c m_location accordingly.
    void end_line() {
        ++m_location.line;
        m_location.col = 1;
    }

public:
    /// Create a new lexer that reads in source code from the file at \p path.
    Lexer(Diagnostics& diags, const string& path);

    /// Create a new lexer based on the file at \p path, while interpreting
    /// \p source as source code.
    Lexer(Diagnostics& diags, const string& path, const string& source);

    /// Returns the most recent token that was lexed.
    const Token& get_last() const { return get_prev(0); }

    /// Returns the token that was lexed \p n iterations ago. 
    /// 
    /// If less than \p n tokens have been historically lexed, then this call 
    /// fails.
    const Token& get_prev(uint32_t n = 1) const {
        size_t size = m_history.size();
        assert(n < size && "lexer has not produced enough tokens yet!");
        
        return m_history[size - n - 1];
    }

    /// Test if the end of the source code buffer has been reached.
    bool is_eof() const { return m_position >= m_source.size(); }

    const Token& lex();    
};

} // namespace stm

#endif // STATIM_LEXER_H_

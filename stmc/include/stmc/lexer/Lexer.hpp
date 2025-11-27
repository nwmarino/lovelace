//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef STATIM_LEXER_H_
#define STATIM_LEXER_H_

#include "stmc/lexer/Token.hpp"

#include <string>
#include <vector>

namespace stm {

using std::string;
using std::vector;

/// A lexical analyzer that turns source code into minimal tokens to be used
/// by the parser.
class Lexer final {
    /// The path to the source file this lexer is working on.
    string m_file;

    /// The source code this lexer is working on.
    string m_source;

    /// The current location in source code this lexer is at.
    SourceLocation m_location = {};

    /// The position in the \c m_source this lexer is at.
    uint32_t m_position = 0;
    
    /// The previous tokens created by this lexer.
    vector<Token> m_history = {};

    /// Returns the current character \c m_position is pointing at.
    char current() const;

    /// Returns the character \p n positions ahead in the source code buffer.
    /// If \p n exceeds the buffer size, then the null terminator is returned.
    char peek(uint32_t n = 1) const;

    /// End the current line and update \c m_location accordingly.
    void end_line();

    /// Move the iterator \c m_position \p n positions ahead, and update the
    /// column field of \c m_location accordingly.
    void move(uint32_t n = 1);

public:
    /// Create a new lexer that reads in source code from the file at \p path.
    Lexer(const string& path);

    /// Create a new lexer based on the file at \p path, with source code
    /// \p source.
    Lexer(const string& path, const string& source);

    /// Returns the last token that was lexed.
    const Token& getCurrent() const;

    /// Returns the token that was lexed \p n iterations ago. 
    /// 
    /// If less than \p n tokens have been historically lexed, then this call 
    /// returns a token that signifies the end of the file.
    const Token& getPrevious(uint32_t n = 1) const;

    /// Lex a new token and store its fields to \p token, and return 'true'.
    /// 
    /// If a new token could not be created i.e. the lexer reached the end of 
    /// the file, then this call will return 'false'.
    bool lex(Token& token);

    /// Returns true if the end of the source code buffer has been reached.
    bool isEof() const;
};

} // namespace stm

#endif // STATIM_LEXER_H_

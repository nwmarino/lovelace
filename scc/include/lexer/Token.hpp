//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SCC_TOKEN_H_
#define SCC_TOKEN_H_

//
// This header file declares important types for tokens that represent the
// smallest usable pieces of C source code.
//
// These tokens are produced by the lexer and used by the parser to create a
// syntax tree which represents the entirety of a program.
//

#include "core/Span.hpp"

#include <cstdint>
#include <string>

namespace scc {

/// Different kinds of recognized C tokens.
enum class TokenKind : uint32_t {
    /// End of file.
    Eof = 0,
    
    /// Identifiers, e.g. `for`, `while`, etc.
    Identifier,

    /// Opening parentheses `(`.
    SetParen,

    /// Closing parentheses `)`.
    EndParen,

    /// Opening curly brace `{`.
    SetBrace,

    /// Closing curly brace `}`.
    EndBrace,

    /// Opening square bracket `[`.
    SetBrack,

    /// Closing square bracket `]`.
    EndBrack,

    /// Basic assignments `=`.
    Eq,

    /// Equality comparisons `==`.
    EqEq,

    /// Logical not operator and bang `!`.
    Bang,

    /// Non-equality comparsons `!=`.
    BangEq,

    /// Addition operator `+`.
    Plus,

    /// Unary increment operator `++`.
    PlusPlus,

    /// Shorthand addition assignment operator `+=`.
    PlusEq,

    /// Subtraction and related operator `-`.
    Minus,

    /// Unary decrement operator `--`.
    MinusMinus,

    /// Shorthand subtraction assignment operator `-=`.
    MinusEq,

    /// Multiplication and related operator `*`.
    Star,
    
    /// Shorthand multiplication assignment operator `*=`.
    StarEq,
    
    /// Division and related operator `/`.
    Slash,

    /// Shorthand division assignment operator `/=`.
    SlashEq,

    /// Modulo and related operator `%`.
    Percent,

    /// Shorthand modulo assignment operator `%=`.
    PercentEq,

    /// Ampersand `&` and bitwise and operator.
    And,

    /// Double ampersand `&&` and logical and operator.
    AndAnd,

    /// Shorthand bitwise and assignment operator `&=`.
    AndEq,

    /// Pipe `|` and bitwise or operator.
    Or,

    /// Double pipe `||` and logical or operator.
    OrOr,

    /// Shorthand bitwise or assignment operator `|=`.
    OrEq,

    /// Caret `^` and bitwise xor operator.
    Xor,

    /// Shorthand bitwise xor assignment operator `^=`.
    XorEq,

    /// Left angle `<` and less than operator.
    Lt,

    /// Left angle equals `<=` and less than equals operator.
    LtEq,

    /// Right angle `>` and greater than operator.
    Gt,
    
    /// Right angle equals `>=` and greater than equals operator.
    GtEq,

    /// Double left angle `<<` and left bitwise shift operator. 
    LShift,

    /// Shorthand left bitwise shift assignment operator `<<=`.
    LShiftEq,

    /// Double right angle `>>` and right bitwise shift operator.
    RShift,

    /// Shorthand right bitwise shift assignment operator `>>=`.
    RShiftEq,

    /// Period `.` and member operator.
    Dot,

    /// Comma delimiter `,`.
    Comma,

    /// Colon delimiter `:`.
    Colon,

    /// Semicolon `;`.
    Semi,

    /// Integer literal, e.g. `1`, `2`, etc.
    Integer,

    /// Floating point literal, e.g. `1.01`, `3.14`, etc.
    Float,

    /// Single ASCII character literal, e.g. `'a'`, `'0'`, etc.
    Character,

    /// Charcter array string literal, e.g. `"hello"`, `"world"`, etc.
    String,
};

/// Represents a token lexed from source code.
struct Token final {
    /// The location that this token was lexed from in the source code.
    SourceLocation loc;

    /// The kind of token this is.
    TokenKind kind;
    
    /// The value of this token. This only applies to literals and identfiers,
    /// and only loosely asserts them as being valid for their kind.
    std::string value;
};

} // namespace scc

#endif // SCC_TOKEN_H_

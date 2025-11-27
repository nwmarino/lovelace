//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef STATIM_TOKEN_H_
#define STATIM_TOKEN_H_

#include "stmc/types/SourceLocation.hpp"

#include <string>

namespace stm {

using std::string;

/// Different kinds of tokens & literals.
enum TokenKind : uint32_t {
    TOKEN_KIND_END_OF_FILE,
    TOKEN_KIND_IDENTIFIER,
    TOKEN_KIND_SET_PAREN,
    TOKEN_KIND_END_PAREN,
    TOKEN_KIND_SET_BRACE,
    TOKEN_KIND_END_BRACE,
    TOKEN_KIND_SET_BRACKET,
    TOKEN_KIND_END_BRACKET,
    TOKEN_KIND_EQUALS,
    TOKEN_KIND_EQUALS_EQUALS,
    TOKEN_KIND_BANG,
    TOKEN_KIND_BANG_EQUALS,
    TOKEN_KIND_PLUS,
    TOKEN_KIND_PLUS_PLUS,
    TOKEN_KIND_PLUS_EQUALS,
    TOKEN_KIND_MINUS,
    TOKEN_KIND_MINUS_MINUS,
    TOKEN_KIND_MINUS_EQUALS,
    TOKEN_KIND_STAR,
    TOKEN_KIND_STAR_EQUALS,
    TOKEN_KIND_SLASH,
    TOKEN_KIND_SLASH_EQUALS,
    TOKEN_KIND_PERCENT,
    TOKEN_KIND_PERCENT_EQUALS,
    TOKEN_KIND_AND,
    TOKEN_KIND_AND_AND,
    TOKEN_KIND_AND_EQUALS,
    TOKEN_KIND_OR,
    TOKEN_KIND_OR_OR,
    TOKEN_KIND_OR_EQUALS,
    TOKEN_KIND_XOR,
    TOKEN_KIND_XOR_EQUALS,
    TOKEN_KIND_LEFT,
    TOKEN_KIND_LEFT_LEFT,
    TOKEN_KIND_LEFT_LEFT_EQUALS,
    TOKEN_KIND_LEFT_EQUALS,
    TOKEN_KIND_RIGHT,
    TOKEN_KIND_RIGHT_RIGHT,
    TOKEN_KIND_RIGHT_RIGHT_EQUALS,
    TOKEN_KIND_RIGHT_EQUALS,
    TOKEN_KIND_ARROW,
    TOKEN_KIND_FAT_ARROW,
    TOKEN_KIND_SIGN,
    TOKEN_KIND_DOT,
    TOKEN_KIND_COMMA,
    TOKEN_KIND_COLON,
    TOKEN_KIND_PATH,
    TOKEN_KIND_SEMICOLON,
    TOKEN_KIND_APOSTROPHE,
    TOKEN_KIND_GRAVE,
    TOKEN_KIND_TILDE,
    TOKEN_KIND_INTEGER,
    TOKEN_KIND_FLOAT,
    TOKEN_KIND_CHARACTER,
    TOKEN_KIND_STRING,
};

/// Represents a token lexed from source.
struct Token final {
    /// The kind of token this is.
    TokenKind kind;

    /// The location of this token in source.
    SourceLocation loc;    
     
    /// The attached value of this token for literals and identifiers. 
    string value;

    Token() = default;

    Token(TokenKind kind, SourceLocation loc) 
        : kind(kind), loc(loc) {}
    
    Token(TokenKind kind, SourceLocation loc, const string& value)
        : kind(kind), loc(loc), value(value) {}

    bool operator == (const Token& other) const {
        return kind == other.kind && loc == other.loc && value == other.value;
    }

    bool operator != (const Token& other) const {
        return kind != other.kind || loc != other.loc || value != other.value;
    }
};

} // namespace stm

#endif // STATIM_TOKEN_H_

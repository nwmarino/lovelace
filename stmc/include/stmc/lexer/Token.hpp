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

/// Represents a token lexed from source.
struct Token final {
    /// The kind of token this is.
    enum Kind {
        EndOfFile = 0,
        SetParen,
        EndParen,
        SetBrace,
        EndBrace,
        SetBrack,
        EndBrack,
        Eq,
        EqEq,
        Bang,
        BangEq,
        Plus,
        PlusPlus,
        PlusEq,
        Minus,
        MinusMinus,
        MinusEq,
        Star,
        StarEq,
        Slash,
        SlashEq,
        Percent,
        PercentEq,
        Left,
        LeftLeft,
        LeftEq,
        LeftLeftEq,
        Right,
        RightRight,
        RightEq,
        RightRightEq,
        And,
        AndAnd,
        AndEq,
        Or,
        OrOr,
        OrEq,
        Xor,
        XorEq,
        Arrow,
        FatArrow,
        Dot,
        Comma,
        Colon,
        Path,
        Semi,
        Grave,
        Tilde,
        Sign,
        Question,
        Identifier,
        Integer,
        Float,
        Character,
        String,
    } kind;

    /// The location of this token in source.
    SourceLocation loc = {};    
     
    /// The attached value of this token for literals and identifiers. 
    string value = "";

    Token(Kind kind) 
      : kind(kind) {}
    
    Token(Kind kind, SourceLocation loc) 
      : kind(kind), loc(loc) {}

    Token(Kind kind, SourceLocation loc, const string& value) 
      : kind(kind), loc(loc), value(value) {}

    bool operator==(const Token& other) const {
        return kind == other.kind && loc == other.loc && value == other.value;
    }

    bool operator!=(const Token& other) const {
        return kind != other.kind || loc != other.loc || value != other.value;
    }

    bool is_eof() const { return kind == Token::EndOfFile; }
};

} // namespace stm

#endif // STATIM_TOKEN_H_

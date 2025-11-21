#ifndef SCC_TOKEN_H_
#define SCC_TOKEN_H_

#include "../core/Span.hpp"

#include <cstdint>
#include <string>

namespace scc {

/// Different kinds of recognized C tokens.
enum class TokenKind : uint32_t {
    Eof = 0,
    Identifier,
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
    And,
    AndAnd,
    AndEq,
    Or,
    OrOr,
    OrEq,
    Xor,
    XorEq,
    Lt,
    LtEq,
    Gt,
    GtEq,
    LShift,
    LShiftEq,
    RShift,
    RShiftEq,
    Dot,
    Comma,
    Colon,
    Semi,
    Integer,
    Float,
    Character,
    String,
};

/// Represents a token lexed from source.
struct Token final {
    SourceLocation loc;
    TokenKind kind;
    std::string value;
};
 
} // namespace scc

#endif // SCC_TOKEN_H_

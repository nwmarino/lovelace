//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "core/Logger.hpp"
#include "core/Tools.hpp"
#include "lexer/Token.hpp"
#include <cctype>
#include "lexer/Lexer.hpp"

using namespace scc;

/// Returns true if \p c is an octal digit, i.e. a digit in [0, 7].
static bool is_octal_digit(char c) {
    return c >= '0' && c <= '7';
}

/// Returns true if \p c is a whitespace or tab.
static bool is_whitespace(char c) {
    return c == ' ' || c == '\t';
}

Lexer::Lexer(const std::string& file, const std::string& source)
    : m_buf(source.empty() ? read_file(file) : source), m_loc(file, 1, 1) {}

const Token& Lexer::lex() {
    if (is_eof()) {
        if (last().kind != TokenKind::Eof)
            m_history.push_back(Token { .kind = TokenKind::Eof });

        return last();
    }

    if (current() == '\n') {
        ++m_pos;
        end_line();
        return lex();
    } else if (is_whitespace(current())) {
        while(is_whitespace(current()))
            move();

        return lex();
    }

    m_history.push_back(Token { .loc = m_loc });
    Token& token = m_history.back();

    switch (current()) {
        case '+':
            if (peek() == '+') {
                token.kind = TokenKind::PlusPlus;
                move(2);
            } else if (peek() == '=') {
                token.kind = TokenKind::PlusEq;
                move(2);
            } else {
                token.kind = TokenKind::Plus;
                move();
            }

            break;
        
        case '-':
            if (peek() == '>') {
                token.kind = TokenKind::Arrow;
                move(2);
            } else if (peek() == '-') {
                token.kind = TokenKind::MinusMinus;
                move(2);
            } else if (peek() == '=') {
                token.kind = TokenKind::MinusEq;
                move(2);
            } else {
                token.kind = TokenKind::Minus;
                move(1);
            }

            break;

        case '*':
            if (peek() == '=') {
                token.kind = TokenKind::StarEq;
                move(2);
            } else {
                token.kind = TokenKind::Star;
                move();
            }
            
            break;

        case '/':
            if (peek() == '/') {
                move(2);
                while (current() != '\n' && current() != '\0')
                    move();
                
                return lex();
            } else if (peek() == '*') {
                move(2); // /*

                while (current() != '\0') {
                    if (current() == '*' && peek() == '/')
                        break;

                    if (current() == '\n') {
                        ++m_pos;
                        end_line();
                    } else {
                        move(1);
                    }
                }

                move(2); // */
                return lex();
            } else if (peek() == '=') {
                token.kind = TokenKind::SlashEq;
                move(2);
            } else {
                token.kind = TokenKind::Slash;
                move();
            }

            break;
        
        case '<':
            if (peek() == '<') {
                if (peek(2) == '=') {
                    token.kind = TokenKind::LShiftEq;
                    move(3);
                } else {
                    token.kind = TokenKind::LShift;
                    move(2);
                }
            } else if (peek() == '=') {
                token.kind = TokenKind::LtEq;
                move(2);
            } else {
                token.kind = TokenKind::Lt;
                move();
            }

            break;

        case '>':
            if (peek() == '>') {
                if (peek(2) == '=') {
                    token.kind = TokenKind::RShiftEq;
                    move(3);
                } else {
                    token.kind = TokenKind::RShift;
                    move(2);
                }
            } else if (peek() == '=') {
                token.kind = TokenKind::GtEq;
                move(2);
            } else {
                token.kind = TokenKind::Gt;
                move();
            }

            break;

        case '&':
            if (peek() == '&') {
                token.kind = TokenKind::AndAnd;
                move(2);
            } else if (peek() == '=') {
                token.kind = TokenKind::AndEq;
                move(2);
            } else {
                token.kind = TokenKind::And;
                move();
            }

            break;

        case '|':
            if (peek() == '|') {
                token.kind = TokenKind::OrOr;
                move(2);
            } else if (peek() == '=') {
                token.kind = TokenKind::OrEq;
                move(2);
            } else {
                token.kind = TokenKind::Or;
                move();
            }

            break;

        case '^':
            if (peek() == '=') {
                token.kind = TokenKind::XorEq;
                move(2);
            } else {
                token.kind = TokenKind::Xor;
                move();
            }

            break;

        case '%':
            if (peek() == '=') {
                token.kind = TokenKind::PercentEq;
                move(2);
            } else {
                token.kind = TokenKind::Percent;
                move();
            }

            break;

        case '=':
            if (peek() == '=') {
                token.kind = TokenKind::EqEq;
                move(2);
            } else {
                token.kind = TokenKind::Eq;
                move();
            }

            break;

        case '!':
            if (peek() == '=') {
                token.kind = TokenKind::BangEq;
                move(2);
            } else {
                token.kind = TokenKind::Bang;
                move();
            }

            break;
        
        case '~':
            token.kind = TokenKind::Tilde;
            move();
            break;

        case '?':
            token.kind = TokenKind::Question;
            move();
            break;

        case ':':
            token.kind = TokenKind::Colon;
            move();
            break;

        case '(':
            token.kind = TokenKind::SetParen;
            move();
            break;

        case ')':
            token.kind = TokenKind::EndParen;
            move();
            break;

        case '{':
            token.kind = TokenKind::SetBrace;
            move();
            break;

        case '}':
            token.kind = TokenKind::EndBrace;
            move();
            break;

        case '[':
            token.kind = TokenKind::SetBrack;
            move();
            break;

        case ']':
            token.kind = TokenKind::EndBrack;
            move();
            break;

        case ',':
            token.kind = TokenKind::Comma;
            move();
            break;

        case ';':
            token.kind = TokenKind::Semi;
            move();
            break;

        case '.':
            if (std::isdigit(peek())) {
                token.kind = TokenKind::Float;
                token.value = ".";
                move();

                while (std::isdigit(current())) {
                    token.value += current();
                    move();
                }
            } else {
                token.kind = TokenKind::Dot;
                move();
            }

            break;

        case '\'':
            move();
            token.kind = TokenKind::Character;

            if (current() == '\\') {
                move();
                switch (current()) {
                    case '0': token.value = "\0"; break;
                    case 'n': token.value = "\n"; break;
                    case 't': token.value = "\t"; break;
                    case 'r': token.value = "\r"; break;
                    case 'b': token.value = "\b"; break;
                    case 'f': token.value = "\f"; break;
                    case 'v': token.value = "\v"; break;
                    case '\\': token.value = "\\"; break;
                    case '\'': token.value = "\'"; break;
                    case '\"': token.value = "\""; break;
                    default: 
                        Logger::error("unknown character escape sequence.");
                }
            } else {
                token.value = current();
            }

            move(2);    
            break;

        case '"':
            move();
            token.kind = TokenKind::String;
            token.value = "";

            while (current() != '"') {
                if (current() == '\\') {
                    move();
                    switch (current()) {
                        case '0': token.value += '\0'; break;
                        case 'n': token.value += '\n'; break;
                        case 't': token.value += '\t'; break;
                        case 'r': token.value += '\r'; break;
                        case 'b': token.value += '\b'; break;
                        case 'f': token.value += '\f'; break;
                        case 'v': token.value += '\v'; break;
                        case '\\': token.value += '\\'; break;
                        case '\'': token.value += '\''; break;
                        case '\"': token.value += '\"'; break;
                        default: if (is_octal_digit(current())) {
                            int32_t oct_val = 0;
                            int32_t digits = 0;

                            while (digits < 3 && is_octal_digit(current())) {
                                oct_val = (oct_val << 3) + (current() - '0');
                                move();
                                digits++;
                            }

                            token.value += static_cast<char>(oct_val);
                            continue;
                        } else {
                            Logger::error("unknown string escape sequence.");
                        }
                    }
                } else {
                    token.value += current();  
                }

                move();
            }

            move();
            break;

        default: if (std::isdigit(current()) || current() == '-') {
            token.kind = TokenKind::Integer;

            if (current() == '-') {
                token.value += current();
                move();
            }

            while (std::isdigit(current()) || current() == '.') {
                if (current() == '.') {
                    if (token.kind == TokenKind::Float)
                        break;

                    token.kind = TokenKind::Float;
                }
                    
                token.value += current();
                move();
            }
        } else if (std::isalpha(current()) || current() == '_') {
            token.kind = TokenKind::Identifier;
            
            while (std::isalnum(current()) || current() == '_') {
                token.value += current();
                move();
            }
        } else {
            Logger::error("unrecognized token.");
        }
    } // end switch

    return token;
}

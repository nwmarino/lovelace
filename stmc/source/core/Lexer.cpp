//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/lexer/Lexer.hpp"
#include "stmc/lexer/Token.hpp"

using namespace stm;

/// Returns true if \p c is an octal digit.
static bool is_octal_digit(char c) {
    return '0' <= c && c <= '7';
}

/// Returns true if \p c is a space or tab.
static bool is_whitespace(char c) {
    return c == ' ' || c == '\t';
}

Lexer::Lexer(Diagnostics& diags, const string& path) 
        : m_diags(diags), m_file(path), m_source(read_file(path)) {
    m_diags.set_path(path);
}

Lexer::Lexer(Diagnostics& diags, const string& path, const string& source)
        : m_diags(diags), m_file(path), m_source(source) {
    m_diags.set_path(path);
}

const Token& Lexer::lex() {
    if (is_eof()) {
        if (m_history.empty() || !get_last().is_eof())
            m_history.emplace_back(Token::EndOfFile);

        return get_last();
    }

    if (get_current() == '\n') {
        ++m_position;
        end_line();
        return lex();
    } else if (is_whitespace(get_current())) {
        while(is_whitespace(get_current()))
            move();

        return lex();
    }

    m_history.emplace_back(Token::EndOfFile, m_location);
    Token& token = m_history.back();

    switch (get_current()) {
        case '+':
            if (peek() == '+') {
                token.kind = Token::PlusPlus;
                move(2);
            } else if (peek() == '=') {
                token.kind = Token::PlusEq;
                move(2);
            } else {
                token.kind = Token::Plus;
                move();
            }

            break;
        
        case '-':
            if (peek() == '>') {
                token.kind = Token::Arrow;
                move(2);
            } else if (peek() == '-') {
                token.kind = Token::MinusMinus;
                move(2);
            } else if (peek() == '=') {
                token.kind = Token::MinusEq;
                move(2);
            } else {
                token.kind = Token::Minus;
                move(1);
            }

            break;

        case '*':
            if (peek() == '=') {
                token.kind = Token::StarEq;
                move(2);
            } else {
                token.kind = Token::Star;
                move();
            }
            
            break;

        case '/':
            if (peek() == '/') {
                move(2);
                while (get_current() != '\n' && get_current() != '\0')
                    move();
                
                return lex();
            } else if (peek() == '*') {
                move(2); // /*

                while (get_current() != '\0') {
                    if (get_current() == '*' && peek() == '/')
                        break;

                    if (get_current() == '\n') {
                        ++m_position;
                        end_line();
                    } else {
                        move(1);
                    }
                }

                move(2); // */
                return lex();
            } else if (peek() == '=') {
                token.kind = Token::SlashEq;
                move(2);
            } else {
                token.kind = Token::Slash;
                move();
            }

            break;
        
        case '<':
            if (peek() == '<') {
                if (peek(2) == '=') {
                    token.kind = Token::LeftLeftEq;
                    move(3);
                } else {
                    token.kind = Token::LeftLeft;
                    move(2);
                }
            } else if (peek() == '=') {
                token.kind = Token::LeftEq;
                move(2);
            } else {
                token.kind = Token::Left;
                move();
            }

            break;

        case '>':
            if (peek() == '>') {
                if (peek(2) == '=') {
                    token.kind = Token::RightRightEq;
                    move(3);
                } else {
                    token.kind = Token::RightRight;
                    move(2);
                }
            } else if (peek() == '=') {
                token.kind = Token::RightEq;
                move(2);
            } else {
                token.kind = Token::Right;
                move();
            }

            break;

        case '&':
            if (peek() == '&') {
                token.kind = Token::AndAnd;
                move(2);
            } else if (peek() == '=') {
                token.kind = Token::AndEq;
                move(2);
            } else {
                token.kind = Token::And;
                move();
            }

            break;

        case '|':
            if (peek() == '|') {
                token.kind = Token::OrOr;
                move(2);
            } else if (peek() == '=') {
                token.kind = Token::OrEq;
                move(2);
            } else {
                token.kind = Token::Or;
                move();
            }

            break;

        case '^':
            if (peek() == '=') {
                token.kind = Token::XorEq;
                move(2);
            } else {
                token.kind = Token::Xor;
                move();
            }

            break;

        case '%':
            if (peek() == '=') {
                token.kind = Token::PercentEq;
                move(2);
            } else {
                token.kind = Token::Percent;
                move();
            }

            break;

        case '=':
            if (peek() == '=') {
                token.kind = Token::EqEq;
                move(2);
            } else {
                token.kind = Token::Eq;
                move();
            }

            break;

        case '!':
            if (peek() == '=') {
                token.kind = Token::BangEq;
                move(2);
            } else {
                token.kind = Token::Bang;
                move();
            }

            break;

        case ':':
            if (peek() == ':') {
                token.kind = Token::Path;
                move(2);
            } else {
                token.kind = Token::Colon;
                move();
            }

            break;
        
        case '~':
            token.kind = Token::Tilde;
            move();
            break;

        case '?':
            token.kind = Token::Question;
            move();
            break;

        case '(':
            token.kind = Token::SetParen;
            move();
            break;

        case ')':
            token.kind = Token::EndParen;
            move();
            break;

        case '{':
            token.kind = Token::SetBrace;
            move();
            break;

        case '}':
            token.kind = Token::EndBrace;
            move();
            break;

        case '[':
            token.kind = Token::SetBrack;
            move();
            break;

        case ']':
            token.kind = Token::EndBrack;
            move();
            break;

        case ',':
            token.kind = Token::Comma;
            move();
            break;

        case ';':
            token.kind = Token::Semi;
            move();
            break;

        case '.':
            if (std::isdigit(peek())) {
                token.kind = Token::Float;
                token.value = ".";
                move();

                while (std::isdigit(get_current())) {
                    token.value += get_current();
                    move();
                }
            } else {
                token.kind = Token::Dot;
                move();
            }

            break;

        case '\'':
            move();
            token.kind = Token::Character;

            if (get_current() == '\\') {
                move();
                switch (get_current()) {
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
                        m_diags.fatal("unknown character escape sequence", m_location);
                }
            } else {
                token.value = get_current();
            }

            move(2);    
            break;

        case '"':
            move();
            token.kind = Token::String;
            token.value = "";

            while (get_current() != '"') {
                if (get_current() == '\\') {
                    move();
                    switch (get_current()) {
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
                        default: if (is_octal_digit(get_current())) {
                            int32_t oct_val = 0;
                            int32_t digits = 0;

                            while (digits < 3 && is_octal_digit(get_current())) {
                                oct_val = (oct_val << 3) + (get_current() - '0');
                                move();
                                digits++;
                            }

                            token.value += static_cast<char>(oct_val);
                            continue;
                        } else {
                            m_diags.fatal("unknown string escape sequence", m_location);
                        }
                    }
                } else {
                    token.value += get_current();  
                }

                move();
            }

            move();
            break;

        default: if (std::isdigit(get_current()) || get_current() == '-') {
            token.kind = Token::Integer;

            if (get_current() == '-') {
                token.value += get_current();
                move();
            }

            while (std::isdigit(get_current()) || get_current() == '.') {
                if (get_current() == '.') {
                    if (token.kind == Token::Float)
                        break;

                    token.kind = Token::Float;
                }
                    
                token.value += get_current();
                move();
            }
        } else if (std::isalpha(get_current()) || get_current() == '_') {
            token.kind = Token::Identifier;
            
            while (std::isalnum(get_current()) || get_current() == '_') {
                token.value += get_current();
                move();
            }
        } else {
            m_diags.fatal("unrecognized token", m_location);
        }
    }

    return token;
}

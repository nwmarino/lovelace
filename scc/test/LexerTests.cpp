//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "lexer/Lexer.hpp"
#include "lexer/Token.hpp"

#include <gtest/gtest.h>

namespace scc::test {

class LexerTests : public ::testing::Test {};

TEST_F(LexerTests, LexIdentifier) {
    Lexer lexer("test", "test");

    const Token& t1 = lexer.lex();
    EXPECT_EQ(t1.kind, TokenKind::Identifier);
    EXPECT_EQ(t1.value, "test");

    const Token& t2 = lexer.lex();
    EXPECT_EQ(t2.kind, TokenKind::Eof);
}

TEST_F(LexerTests, LexIdentifierMany) {
    Lexer lexer("test", "one_ _two three_ _four");

    const Token& t1 = lexer.lex();
    EXPECT_EQ(t1.kind, TokenKind::Identifier);
    EXPECT_EQ(t1.value, "one_");

    const Token& t2 = lexer.lex();
    EXPECT_EQ(t2.kind, TokenKind::Identifier);
    EXPECT_EQ(t2.value, "_two");

    const Token& t3 = lexer.lex();
    EXPECT_EQ(t3.kind, TokenKind::Identifier);
    EXPECT_EQ(t3.value, "three_");

    const Token& t4 = lexer.lex();
    EXPECT_EQ(t4.kind, TokenKind::Identifier);
    EXPECT_EQ(t4.value, "_four");
}

TEST_F(LexerTests, LexIntegerLiteral) {
    Lexer lexer("test", "1 0u 1L 5ul");

    const Token& t1 = lexer.lex();
    EXPECT_EQ(t1.kind, TokenKind::Integer);
    EXPECT_EQ(t1.value, "1");

    const Token& t2 = lexer.lex();
    EXPECT_EQ(t2.kind, TokenKind::Integer);
    EXPECT_EQ(t2.value, "0");

    const Token& t3 = lexer.lex();
    EXPECT_EQ(t3.kind, TokenKind::Identifier);
    EXPECT_EQ(t3.value, "u");

    const Token& t4 = lexer.lex();
    EXPECT_EQ(t4.kind, TokenKind::Integer);
    EXPECT_EQ(t4.value, "1");

    const Token& t5 = lexer.lex();
    EXPECT_EQ(t5.kind, TokenKind::Identifier);
    EXPECT_EQ(t5.value, "L");

    const Token& t6 = lexer.lex();
    EXPECT_EQ(t6.kind, TokenKind::Integer);
    EXPECT_EQ(t6.value, "5");

    const Token& t7 = lexer.lex();
    EXPECT_EQ(t7.kind, TokenKind::Identifier);
    EXPECT_EQ(t7.value, "ul");
}

TEST_F(LexerTests, LexFloatLiteral) {
    Lexer lexer("test", "1.0 1.f .1 3.14F");

    const Token& t1 = lexer.lex();
    EXPECT_EQ(t1.kind, TokenKind::Float);
    EXPECT_EQ(t1.value, "1.0");

    const Token& t2 = lexer.lex();
    EXPECT_EQ(t2.kind, TokenKind::Float);
    EXPECT_EQ(t2.value, "1.");

    const Token& t3 = lexer.lex();
    EXPECT_EQ(t3.kind, TokenKind::Identifier);
    EXPECT_EQ(t3.value, "f");

    const Token& t4 = lexer.lex();
    EXPECT_EQ(t4.kind, TokenKind::Float);
    EXPECT_EQ(t4.value, ".1");

    const Token& t5 = lexer.lex();
    EXPECT_EQ(t5.kind, TokenKind::Float);
    EXPECT_EQ(t5.value, "3.14");

    const Token& t6 = lexer.lex();
    EXPECT_EQ(t6.kind, TokenKind::Identifier);
    EXPECT_EQ(t6.value, "F");
}

TEST_F(LexerTests, LexCharacterLiteral) {
    Lexer lexer("test", "'a' '0'");

    const Token& t1 = lexer.lex();
    EXPECT_EQ(t1.kind, TokenKind::Character);
    EXPECT_EQ(t1.value, "a");

    const Token& t2 = lexer.lex();
    EXPECT_EQ(t2.kind, TokenKind::Character);
    EXPECT_EQ(t2.value, "0");
}

TEST_F(LexerTests, LexCharacterLiteralEscapeSequences) {
    Lexer lexer("test", "'\v' '\n' '\t' '\''");

    const Token& t1 = lexer.lex();
    EXPECT_EQ(t1.kind, TokenKind::Character);
    EXPECT_EQ(t1.value, "\v");

    const Token& t2 = lexer.lex();
    EXPECT_EQ(t2.kind, TokenKind::Character);
    EXPECT_EQ(t2.value, "\n");

    const Token& t3 = lexer.lex();
    EXPECT_EQ(t3.kind, TokenKind::Character);
    EXPECT_EQ(t3.value, "\t");

    const Token& t4 = lexer.lex();
    EXPECT_EQ(t4.kind, TokenKind::Character);
    EXPECT_EQ(t4.value, "'");
}

TEST_F(LexerTests, LexStringLiteral) {
    Lexer lexer("test", "\"hello, world!\"");

    const Token& t1 = lexer.lex();
    EXPECT_EQ(t1.kind, TokenKind::String);
    EXPECT_EQ(t1.value, "hello, world!");
}

TEST_F(LexerTests, LexStringLiteralEscapeSequences) {
    Lexer lexer("test", "\"hello,\tworld!\n\"");

    const Token& t1 = lexer.lex();
    EXPECT_EQ(t1.kind, TokenKind::String);
    EXPECT_EQ(t1.value, "hello,\tworld!\n");
}

TEST_F(LexerTests, LexIsolated) {
    Lexer lexer("test", ".");

    const Token& t1 = lexer.lex();
    EXPECT_EQ(t1.kind, TokenKind::Dot);
}

TEST_F(LexerTests, LexCompound) {
    Lexer lexer("test", "++");

    const Token& t1 = lexer.lex();
    EXPECT_EQ(t1.kind, TokenKind::PlusPlus);
}

TEST_F(LexerTests, LexComplete1) {
    Lexer lexer(
        "test", "int main(int argc, char* argv[]) { return argc * 3; }");

    const Token& t1 = lexer.lex();
    EXPECT_EQ(t1.kind, TokenKind::Identifier);
    EXPECT_EQ(t1.value, "int");

    const Token& t2 = lexer.lex();
    EXPECT_EQ(t2.kind, TokenKind::Identifier);
    EXPECT_EQ(t2.value, "main");

    const Token& t3 = lexer.lex();
    EXPECT_EQ(t3.kind, TokenKind::SetParen);

    const Token& t4 = lexer.lex();
    EXPECT_EQ(t4.kind, TokenKind::Identifier);
    EXPECT_EQ(t4.value, "int");

    const Token& t5 = lexer.lex();
    EXPECT_EQ(t5.kind, TokenKind::Identifier);
    EXPECT_EQ(t5.value, "argc");

    const Token& t6 = lexer.lex();
    EXPECT_EQ(t6.kind, TokenKind::Comma);

    const Token& t7 = lexer.lex();
    EXPECT_EQ(t7.kind, TokenKind::Identifier);
    EXPECT_EQ(t7.value, "char");

    const Token& t8 = lexer.lex();
    EXPECT_EQ(t8.kind, TokenKind::Star);

    const Token& t9 = lexer.lex();
    EXPECT_EQ(t9.kind, TokenKind::Identifier);
    EXPECT_EQ(t9.value, "argv");

    const Token& t10 = lexer.lex();
    EXPECT_EQ(t10.kind, TokenKind::SetBrack);

    const Token& t11 = lexer.lex();
    EXPECT_EQ(t11.kind, TokenKind::EndBrack);

    const Token& t12 = lexer.lex();
    EXPECT_EQ(t12.kind, TokenKind::EndParen);

    const Token& t13 = lexer.lex();
    EXPECT_EQ(t13.kind, TokenKind::SetBrace);

    const Token& t14 = lexer.lex();
    EXPECT_EQ(t14.kind, TokenKind::Identifier);
    EXPECT_EQ(t14.value, "return");

    const Token& t15 = lexer.lex();
    EXPECT_EQ(t15.kind, TokenKind::Identifier);
    EXPECT_EQ(t15.value, "argc");

    const Token& t16 = lexer.lex();
    EXPECT_EQ(t16.kind, TokenKind::Star);

    const Token& t17 = lexer.lex();
    EXPECT_EQ(t17.kind, TokenKind::Integer);
    EXPECT_EQ(t17.value, "3");

    const Token& t18 = lexer.lex();
    EXPECT_EQ(t18.kind, TokenKind::Semi);

    const Token& t19 = lexer.lex();
    EXPECT_EQ(t19.kind, TokenKind::EndBrace);
}

} // namespace scc::test

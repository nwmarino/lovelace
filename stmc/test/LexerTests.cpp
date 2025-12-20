//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/core/Diagnostics.hpp"
#include "stmc/lexer/Lexer.hpp"
#include "stmc/lexer/Token.hpp"

#include <gtest/gtest.h>

namespace stm::test {

class LexerTests : public ::testing::Test {
protected:
    Diagnostics diags;

    void SetUp() override {
        diags = {};
    }
};

TEST_F(LexerTests, Identifier) {
    Lexer lexer(diags, "test", "test");

    const Token& t1 = lexer.lex();
    EXPECT_EQ(t1.kind, Token::Identifier);
    EXPECT_EQ(t1.value, "test");

    const Token& t2 = lexer.lex();
    EXPECT_EQ(t2.kind, Token::EndOfFile);
}

TEST_F(LexerTests, MultipleIdentifiers) {
    Lexer lexer(diags, "test", "one_ _two three_ _four");

    const Token& t1 = lexer.lex();
    EXPECT_EQ(t1.kind, Token::Identifier);
    EXPECT_EQ(t1.value, "one_");

    const Token& t2 = lexer.lex();
    EXPECT_EQ(t2.kind, Token::Identifier);
    EXPECT_EQ(t2.value, "_two");

    const Token& t3 = lexer.lex();
    EXPECT_EQ(t3.kind, Token::Identifier);
    EXPECT_EQ(t3.value, "three_");

    const Token& t4 = lexer.lex();
    EXPECT_EQ(t4.kind, Token::Identifier);
    EXPECT_EQ(t4.value, "_four");
}

TEST_F(LexerTests, IntegerLiteral) {
    Lexer lexer(diags, "test", "1 0u 1L 5ul");

    const Token& t1 = lexer.lex();
    EXPECT_EQ(t1.kind, Token::Integer);
    EXPECT_EQ(t1.value, "1");

    const Token& t2 = lexer.lex();
    EXPECT_EQ(t2.kind, Token::Integer);
    EXPECT_EQ(t2.value, "0");

    const Token& t3 = lexer.lex();
    EXPECT_EQ(t3.kind, Token::Identifier);
    EXPECT_EQ(t3.value, "u");

    const Token& t4 = lexer.lex();
    EXPECT_EQ(t4.kind, Token::Integer);
    EXPECT_EQ(t4.value, "1");

    const Token& t5 = lexer.lex();
    EXPECT_EQ(t5.kind, Token::Identifier);
    EXPECT_EQ(t5.value, "L");

    const Token& t6 = lexer.lex();
    EXPECT_EQ(t6.kind, Token::Integer);
    EXPECT_EQ(t6.value, "5");

    const Token& t7 = lexer.lex();
    EXPECT_EQ(t7.kind, Token::Identifier);
    EXPECT_EQ(t7.value, "ul");
}

TEST_F(LexerTests, FloatLiteral) {
    Lexer lexer(diags, "test", "1.0 1.f .1 3.14F");

    const Token& t1 = lexer.lex();
    EXPECT_EQ(t1.kind, Token::Float);
    EXPECT_EQ(t1.value, "1.0");

    const Token& t2 = lexer.lex();
    EXPECT_EQ(t2.kind, Token::Float);
    EXPECT_EQ(t2.value, "1.");

    const Token& t3 = lexer.lex();
    EXPECT_EQ(t3.kind, Token::Identifier);
    EXPECT_EQ(t3.value, "f");

    const Token& t4 = lexer.lex();
    EXPECT_EQ(t4.kind, Token::Float);
    EXPECT_EQ(t4.value, ".1");

    const Token& t5 = lexer.lex();
    EXPECT_EQ(t5.kind, Token::Float);
    EXPECT_EQ(t5.value, "3.14");

    const Token& t6 = lexer.lex();
    EXPECT_EQ(t6.kind, Token::Identifier);
    EXPECT_EQ(t6.value, "F");
}

TEST_F(LexerTests, CharacterLiteral) {
    Lexer lexer(diags, "test", "'a' '0'");

    const Token& t1 = lexer.lex();
    EXPECT_EQ(t1.kind, Token::Character);
    EXPECT_EQ(t1.value, "a");

    const Token& t2 = lexer.lex();
    EXPECT_EQ(t2.kind, Token::Character);
    EXPECT_EQ(t2.value, "0");
}

TEST_F(LexerTests, CharacterEscapeSequences) {
    Lexer lexer(diags, "test", "'\v' '\n' '\t' '\''");

    const Token& t1 = lexer.lex();
    EXPECT_EQ(t1.kind, Token::Character);
    EXPECT_EQ(t1.value, "\v");

    const Token& t2 = lexer.lex();
    EXPECT_EQ(t2.kind, Token::Character);
    EXPECT_EQ(t2.value, "\n");

    const Token& t3 = lexer.lex();
    EXPECT_EQ(t3.kind, Token::Character);
    EXPECT_EQ(t3.value, "\t");

    const Token& t4 = lexer.lex();
    EXPECT_EQ(t4.kind, Token::Character);
    EXPECT_EQ(t4.value, "'");
}

TEST_F(LexerTests, StringLiteral) {
    Lexer lexer(diags, "test", "\"hello, world!\"");

    const Token& t1 = lexer.lex();
    EXPECT_EQ(t1.kind, Token::String);
    EXPECT_EQ(t1.value, "hello, world!");
}

TEST_F(LexerTests, StringEscapeSequences) {
    Lexer lexer(diags, "test", "\"hello,\tworld!\n\"");

    const Token& t1 = lexer.lex();
    EXPECT_EQ(t1.kind, Token::String);
    EXPECT_EQ(t1.value, "hello,\tworld!\n");
}

TEST_F(LexerTests, IsolatedToken) {
    Lexer lexer(diags, "test", ".");

    const Token& t1 = lexer.lex();
    EXPECT_EQ(t1.kind, Token::Dot);
}

TEST_F(LexerTests, CompoundToken) {
    Lexer lexer(diags, "test", "++");

    const Token& t1 = lexer.lex();
    EXPECT_EQ(t1.kind, Token::PlusPlus);
}

TEST_F(LexerTests, Complete) {
    Lexer lexer(diags, 
        "test", 
        "int main(int argc, char* argv[]) { return argc * 3; }");

    const Token& t1 = lexer.lex();
    EXPECT_EQ(t1.kind, Token::Identifier);
    EXPECT_EQ(t1.value, "int");

    const Token& t2 = lexer.lex();
    EXPECT_EQ(t2.kind, Token::Identifier);
    EXPECT_EQ(t2.value, "main");

    const Token& t3 = lexer.lex();
    EXPECT_EQ(t3.kind, Token::SetParen);

    const Token& t4 = lexer.lex();
    EXPECT_EQ(t4.kind, Token::Identifier);
    EXPECT_EQ(t4.value, "int");

    const Token& t5 = lexer.lex();
    EXPECT_EQ(t5.kind, Token::Identifier);
    EXPECT_EQ(t5.value, "argc");

    const Token& t6 = lexer.lex();
    EXPECT_EQ(t6.kind, Token::Comma);

    const Token& t7 = lexer.lex();
    EXPECT_EQ(t7.kind, Token::Identifier);
    EXPECT_EQ(t7.value, "char");

    const Token& t8 = lexer.lex();
    EXPECT_EQ(t8.kind, Token::Star);

    const Token& t9 = lexer.lex();
    EXPECT_EQ(t9.kind, Token::Identifier);
    EXPECT_EQ(t9.value, "argv");

    const Token& t10 = lexer.lex();
    EXPECT_EQ(t10.kind, Token::SetBrack);

    const Token& t11 = lexer.lex();
    EXPECT_EQ(t11.kind, Token::EndBrack);

    const Token& t12 = lexer.lex();
    EXPECT_EQ(t12.kind, Token::EndParen);

    const Token& t13 = lexer.lex();
    EXPECT_EQ(t13.kind, Token::SetBrace);

    const Token& t14 = lexer.lex();
    EXPECT_EQ(t14.kind, Token::Identifier);
    EXPECT_EQ(t14.value, "return");

    const Token& t15 = lexer.lex();
    EXPECT_EQ(t15.kind, Token::Identifier);
    EXPECT_EQ(t15.value, "argc");

    const Token& t16 = lexer.lex();
    EXPECT_EQ(t16.kind, Token::Star);

    const Token& t17 = lexer.lex();
    EXPECT_EQ(t17.kind, Token::Integer);
    EXPECT_EQ(t17.value, "3");

    const Token& t18 = lexer.lex();
    EXPECT_EQ(t18.kind, Token::Semi);

    const Token& t19 = lexer.lex();
    EXPECT_EQ(t19.kind, Token::EndBrace);
}

} // namespace stm::test

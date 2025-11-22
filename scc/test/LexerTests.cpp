//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "lexer/Lexer.hpp"

#include <gtest/gtest.h>

namespace scc::test {

class LexerTests : public ::testing::Test {};

TEST_F(LexerTests, LexIdentifier) {
    Lexer lexer("test", "test");

    lexer.lex();
    
    const Token& t1 = lexer.last();
    EXPECT_EQ(t1.kind, TokenKind::Identifier);
    EXPECT_EQ(t1.value, "test");

    lexer.lex();
    const Token& t2 = lexer.last();

    EXPECT_EQ(t2.kind, TokenKind::Eof);
}

} // namespace scc::test

//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/core/Diagnostics.hpp"
#include "stmc/lexer/Lexer.hpp"
#include "stmc/lexer/Token.hpp"
#include "stmc/parser/Parser.hpp"
#include "stmc/tree/Decl.hpp"
#include "stmc/tree/Expr.hpp"
#include "stmc/tree/Stmt.hpp"

#include <gtest/gtest.h>

namespace stm::test {

class DeclParserTests : public ::testing::Test {
protected:
    Diagnostics diags;
    TranslationUnitDecl* unit;

    void SetUp() override {
        diags = {};
        unit = nullptr;
    }

    void TearDown() override {
        if (unit) { 
            delete unit;
            unit = nullptr;
        }
    }
};

TEST_F(DeclParserTests, Positive_SimpleVoidFunction) {
    Parser parser(diags, "test", "test :: () -> void;");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    FunctionDecl* FD = dynamic_cast<FunctionDecl*>(unit->get_decls()[0]);
    EXPECT_NE(FD, nullptr);
    EXPECT_EQ(FD->get_name(), "test");
    EXPECT_FALSE(FD->has_runes());
    EXPECT_FALSE(FD->has_params());
    EXPECT_FALSE(FD->has_body());
}

TEST_F(DeclParserTests, Positive_ReturnZero) {
    Parser parser(diags, "test", "test :: () -> i64 { ret 0; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    FunctionDecl* FD = dynamic_cast<FunctionDecl*>(unit->get_decls()[0]);
    EXPECT_NE(FD, nullptr);
    EXPECT_EQ(FD->get_name(), "test");
    EXPECT_TRUE(FD->has_body());

    BlockStmt* BS = dynamic_cast<BlockStmt*>(FD->get_body());
    EXPECT_NE(BS, nullptr);
    EXPECT_EQ(BS->num_stmts(), 1);
    
    RetStmt* RS = dynamic_cast<RetStmt*>(BS->get_stmt(0));
    EXPECT_NE(RS, nullptr);
    EXPECT_TRUE(RS->has_expr());

    IntegerLiteral* IL = dynamic_cast<IntegerLiteral*>(RS->get_expr());
    EXPECT_NE(IL, nullptr);
    EXPECT_EQ(IL->get_value(), 0);
}

} // namespace stm::test

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

class ExprParserTests : public ::testing::Test {
protected:
    Diagnostics diags;
    TranslationUnitDecl* unit;

    void SetUp() override {
        diags = Diagnostics(&std::cerr, false);
        unit = nullptr;
    }

    void TearDown() override {
        if (unit) { 
            delete unit;
            unit = nullptr;
        }
    }
};

TEST_F(ExprParserTests, IntegerLiteral_TypeSuffixes) {
    Parser parser(diags, "test", "test :: () -> void { 1b; 2ub; 3s; 4us; 5i; 6ui; 7l; 8ul; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    FunctionDecl* FD = dynamic_cast<FunctionDecl*>(unit->get_decls()[0]);
    EXPECT_NE(FD, nullptr);
    EXPECT_TRUE(FD->has_body());

    BlockStmt* BS = dynamic_cast<BlockStmt*>(FD->get_body());
    EXPECT_NE(BS, nullptr);
    EXPECT_EQ(BS->num_stmts(), 8);

    IntegerLiteral* IL1 = dynamic_cast<IntegerLiteral*>(BS->get_stmt(0));
    EXPECT_NE(IL1, nullptr);
    EXPECT_EQ(IL1->get_value(), 1);
    EXPECT_EQ(IL1->get_type().to_string(), "s8");

    IntegerLiteral* IL2 = dynamic_cast<IntegerLiteral*>(BS->get_stmt(1));
    EXPECT_NE(IL2, nullptr);
    EXPECT_EQ(IL2->get_value(), 2);
    EXPECT_EQ(IL2->get_type().to_string(), "u8");

    IntegerLiteral* IL3 = dynamic_cast<IntegerLiteral*>(BS->get_stmt(2));
    EXPECT_NE(IL3, nullptr);
    EXPECT_EQ(IL3->get_value(), 3);
    EXPECT_EQ(IL3->get_type().to_string(), "s16");

    IntegerLiteral* IL4 = dynamic_cast<IntegerLiteral*>(BS->get_stmt(3));
    EXPECT_NE(IL4, nullptr);
    EXPECT_EQ(IL4->get_value(), 4);
    EXPECT_EQ(IL4->get_type().to_string(), "u16");

    IntegerLiteral* IL5 = dynamic_cast<IntegerLiteral*>(BS->get_stmt(4));
    EXPECT_NE(IL5, nullptr);
    EXPECT_EQ(IL5->get_value(), 5);
    EXPECT_EQ(IL5->get_type().to_string(), "s32");

    IntegerLiteral* IL6 = dynamic_cast<IntegerLiteral*>(BS->get_stmt(5));
    EXPECT_NE(IL6, nullptr);
    EXPECT_EQ(IL6->get_value(), 6);
    EXPECT_EQ(IL6->get_type().to_string(), "u32");

    IntegerLiteral* IL7 = dynamic_cast<IntegerLiteral*>(BS->get_stmt(6));
    EXPECT_NE(IL7, nullptr);
    EXPECT_EQ(IL7->get_value(), 7);
    EXPECT_EQ(IL7->get_type().to_string(), "s64");

    IntegerLiteral* IL8 = dynamic_cast<IntegerLiteral*>(BS->get_stmt(7));
    EXPECT_NE(IL8, nullptr);
    EXPECT_EQ(IL8->get_value(), 8);
    EXPECT_EQ(IL8->get_type().to_string(), "u64");
}

TEST_F(ExprParserTests, FloatLiteral_TypeSuffixes) {
    Parser parser(diags, "test", "test :: () -> void { 1.f; 2.d; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    FunctionDecl* FD = dynamic_cast<FunctionDecl*>(unit->get_decls()[0]);
    EXPECT_NE(FD, nullptr);
    EXPECT_TRUE(FD->has_body());

    BlockStmt* BS = dynamic_cast<BlockStmt*>(FD->get_body());
    EXPECT_NE(BS, nullptr);
    EXPECT_EQ(BS->num_stmts(), 2);

    FPLiteral* FP1 = dynamic_cast<FPLiteral*>(BS->get_stmt(0));
    EXPECT_NE(FP1, nullptr);
    EXPECT_EQ(FP1->get_type().to_string(), "f32");

    FPLiteral* FP2 = dynamic_cast<FPLiteral*>(BS->get_stmt(1));
    EXPECT_NE(FP2, nullptr);
    EXPECT_EQ(FP2->get_type().to_string(), "f64");
}

} // namespace stm::test

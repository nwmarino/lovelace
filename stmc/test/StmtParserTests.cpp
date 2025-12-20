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

class StmtParserTests : public ::testing::Test {
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

TEST_F(StmtParserTests, IfStatement_Positive) {
    Parser parser(diags, "test", "test :: () -> s64 { if 5 { ret 0; }}");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    FunctionDecl* FD = dynamic_cast<FunctionDecl*>(unit->get_decls()[0]);
    EXPECT_NE(FD, nullptr);
    EXPECT_TRUE(FD->has_body());

    BlockStmt* BS = dynamic_cast<BlockStmt*>(FD->get_body());
    EXPECT_NE(BS, nullptr);
    EXPECT_EQ(BS->num_stmts(), 1);

    IfStmt* IS = dynamic_cast<IfStmt*>(BS->get_stmt(0));
    EXPECT_NE(IS, nullptr);
    EXPECT_FALSE(IS->has_else());

    IntegerLiteral* IL = dynamic_cast<IntegerLiteral*>(IS->get_cond());
    EXPECT_NE(IL, nullptr);
    EXPECT_EQ(IL->get_value(), 5);

    BlockStmt* BS2 = dynamic_cast<BlockStmt*>(IS->get_then());
    EXPECT_NE(BS2, nullptr);
    EXPECT_EQ(BS2->num_stmts(), 1);

    RetStmt* RS = dynamic_cast<RetStmt*>(BS2->get_stmt(0));
    EXPECT_NE(RS, nullptr);
}

TEST_F(StmtParserTests, IfElseStatement_Positive) {
    Parser parser(diags, "test", "test :: () -> s64 { if 5 ret 0; else ret 1; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    FunctionDecl* FD = dynamic_cast<FunctionDecl*>(unit->get_decls()[0]);
    EXPECT_NE(FD, nullptr);
    EXPECT_TRUE(FD->has_body());

    BlockStmt* BS = dynamic_cast<BlockStmt*>(FD->get_body());
    EXPECT_NE(BS, nullptr);
    EXPECT_EQ(BS->num_stmts(), 1);

    IfStmt* IS = dynamic_cast<IfStmt*>(BS->get_stmt(0));
    EXPECT_NE(IS, nullptr);
    EXPECT_TRUE(IS->has_else());

    IntegerLiteral* IL = dynamic_cast<IntegerLiteral*>(IS->get_cond());
    EXPECT_NE(IL, nullptr);
    EXPECT_EQ(IL->get_value(), 5);

    RetStmt* RS = dynamic_cast<RetStmt*>(IS->get_then());
    EXPECT_NE(RS, nullptr);

    RetStmt* RS2 = dynamic_cast<RetStmt*>(IS->get_else());
    EXPECT_NE(RS2, nullptr);
}

TEST_F(StmtParserTests, WhileStatement_Positive) {
    Parser parser(diags, "test", "test :: () -> s64 { while 1 continue; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    FunctionDecl* FD = dynamic_cast<FunctionDecl*>(unit->get_decls()[0]);
    EXPECT_NE(FD, nullptr);
    EXPECT_TRUE(FD->has_body());

    BlockStmt* BS = dynamic_cast<BlockStmt*>(FD->get_body());
    EXPECT_NE(BS, nullptr);
    EXPECT_EQ(BS->num_stmts(), 1);

    WhileStmt* WS = dynamic_cast<WhileStmt*>(BS->get_stmt(0));
    EXPECT_NE(WS, nullptr);
    EXPECT_TRUE(WS->has_body());

    ContinueStmt* CS = dynamic_cast<ContinueStmt*>(WS->get_body());
    EXPECT_NE(CS, nullptr);
}

TEST_F(StmtParserTests, WhileStatementNoBody_Positive) {
    Parser parser(diags, "test", "test :: () -> s64 { while 1; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    FunctionDecl* FD = dynamic_cast<FunctionDecl*>(unit->get_decls()[0]);
    EXPECT_NE(FD, nullptr);
    EXPECT_TRUE(FD->has_body());

    BlockStmt* BS = dynamic_cast<BlockStmt*>(FD->get_body());
    EXPECT_NE(BS, nullptr);
    EXPECT_EQ(BS->num_stmts(), 1);

    WhileStmt* WS = dynamic_cast<WhileStmt*>(BS->get_stmt(0));
    EXPECT_NE(WS, nullptr);
    EXPECT_FALSE(WS->has_body());
}

} // namespace stm::test

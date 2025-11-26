//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "ast/Decl.hpp"
#include "ast/Expr.hpp"
#include "ast/Parser.hpp"
#include "ast/Sema.hpp"
#include "ast/Stmt.hpp"
#include "ast/Type.hpp"
#include "core/Logger.hpp"

#include "gtest/gtest.h"

namespace scc::test {

class SemaTests : public ::testing::Test {
protected:
    TranslationUnitDecl* unit;

    void SetUp() override {
        Logger::init();
        unit = nullptr;
    }

    void TearDown() override {
        if (unit != nullptr) { 
            delete unit;
            unit = nullptr;
        }
    }
};

TEST_F(SemaTests, MainFunctionCheck_Positive_ReturnType) {
    Parser parser("test", "int main();");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    Sema sema(unit);
    EXPECT_NO_FATAL_FAILURE(unit->accept(sema));
}

TEST_F(SemaTests, MainFunctionCheck_Positive_AliasedReturnType) {
    Parser parser("test", "typedef int int32_t; int32_t main();");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    Sema sema(unit);
    EXPECT_NO_FATAL_FAILURE(unit->accept(sema));
}

TEST_F(SemaTests, MainFunctionCheck_Negative_ReturnType) {
    Parser parser("test", "char main();");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    Sema sema(unit);
    EXPECT_DEATH(unit->accept(sema), "");
}

TEST_F(SemaTests, VariableTypeCheck_Positive) {
    Parser parser("test", "int x = 1;");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    Sema sema(unit);
    EXPECT_NO_FATAL_FAILURE(unit->accept(sema));
}

TEST_F(SemaTests, VariableTypeCheck_Negative) {
    Parser parser("test", "int x = \"test\";");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    Sema sema(unit);
    EXPECT_DEATH(unit->accept(sema), "");
}

TEST_F(SemaTests, IfConditionCheck_Positive) {
    Parser parser("test", "int foo() { if (1) { return 0; } }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    Sema sema(unit);
    EXPECT_NO_FATAL_FAILURE(unit->accept(sema));
}

TEST_F(SemaTests, IfConditionCheck_Negative) {
    Parser parser("test", "void bar(); int foo() { if (bar()) { return 0; } }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    Sema sema(unit);
    EXPECT_DEATH(unit->accept(sema), "");
}

TEST_F(SemaTests, ReturnTypeCheck_Positive_Void) {
    Parser parser("test", "void foo() { return; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    Sema sema(unit);
    EXPECT_NO_FATAL_FAILURE(unit->accept(sema));
}

TEST_F(SemaTests, ReturnTypeCheck_Positive_NotVoid) {
    Parser parser("test", "int foo() { return 1; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    Sema sema(unit);
    EXPECT_NO_FATAL_FAILURE(unit->accept(sema));
}

TEST_F(SemaTests, ReturnTypeCheck_Positive_ImplicitCast) {
    Parser parser("test", "float foo() { return 1; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    Sema sema(unit);
    EXPECT_NO_FATAL_FAILURE(unit->accept(sema));
}

TEST_F(SemaTests, ReturnTypeCheck_Negative_Void) {
    Parser parser("test", "int foo() { return; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    Sema sema(unit);
    EXPECT_DEATH(unit->accept(sema), "");
}

TEST_F(SemaTests, ReturnTypeCheck_Negative_CannotCast) {
    Parser parser("test", "int foo() { return \"test\"; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    Sema sema(unit);
    EXPECT_DEATH(unit->accept(sema), "");
}

} // namespace scc::test

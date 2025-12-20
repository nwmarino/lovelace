//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/core/Diagnostics.hpp"
#include "stmc/parser/Parser.hpp"
#include "stmc/tree/Decl.hpp"
#include "stmc/tree/SemanticAnalysis.hpp"
#include "stmc/tree/SymbolAnalysis.hpp"

#include "gtest/gtest.h"

namespace stm::test {

class SemanticAnalysisTests : public ::testing::Test {
protected:
    Options opts;
    Diagnostics diags;
    TranslationUnitDecl* unit;

    void SetUp() override {
        opts = Options();
        diags = Diagnostics(&std::cerr, false);
        unit = nullptr;
    }

    void TearDown() override {
        if (unit != nullptr) { 
            delete unit;
            unit = nullptr;
        }
    }
};

TEST_F(SemanticAnalysisTests, MainCheck_ReturnType_Positive) {
    Parser parser(diags, "test", "main :: () -> s64;");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    SymbolAnalysis syma(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(syma));

    SemanticAnalysis sema(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(sema));
}

TEST_F(SemanticAnalysisTests, MainCheck_ReturnType_Negative) {
    Parser parser(diags, "test", "main :: () -> s8;");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    SymbolAnalysis syma(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(syma));

    SemanticAnalysis sema(diags, opts);
    EXPECT_DEATH(unit->accept(sema), "");
}

TEST_F(SemanticAnalysisTests, ConditionCheck_IfCondition_Positive) {
    Parser parser(diags, "test", "foo :: () -> s64 { if 1 { ret 0; } }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    SymbolAnalysis syma(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(syma));

    SemanticAnalysis sema(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(sema));
}

TEST_F(SemanticAnalysisTests, ConditionCheck_IfCondition_Negative) {
    Parser parser(diags, "test", "bar :: () -> void; foo :: () -> s64 { if bar() { ret 0; } }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    SymbolAnalysis syma(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(syma));

    SemanticAnalysis sema(diags, opts);
    EXPECT_DEATH(unit->accept(sema), "");
}

TEST_F(SemanticAnalysisTests, ConditionCheck_WhileCondition_Positive) {
    Parser parser(diags, "test", "foo :: () -> s64 { while 1 continue; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    SymbolAnalysis syma(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(syma));

    SemanticAnalysis sema(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(sema));
}

TEST_F(SemanticAnalysisTests, ConditionCheck_WhileCondition_Negative) {
    Parser parser(diags, "test", "bar :: () -> void; foo :: () -> s64 { while bar() continue; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    SymbolAnalysis syma(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(syma));

    SemanticAnalysis sema(diags, opts);
    EXPECT_DEATH(unit->accept(sema), "");
}

TEST_F(SemanticAnalysisTests, ControlCheck_BreakInLoop_Positive) {
    Parser parser(diags, "test", "foo :: () -> s64 { while 1 break; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    SymbolAnalysis syma(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(syma));

    SemanticAnalysis sema(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(sema));
}

TEST_F(SemanticAnalysisTests, ControlCheck_BreakInLoop_Negative) {
    Parser parser(diags, "test", "foo :: () -> s64 { break; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    SymbolAnalysis syma(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(syma));

    SemanticAnalysis sema(diags, opts);
    EXPECT_DEATH(unit->accept(sema), "");
}

TEST_F(SemanticAnalysisTests, ControlCheck_ContinueInLoop_Positive) {
    Parser parser(diags, "test", "foo :: () -> s64 { while 1 continue; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    SymbolAnalysis syma(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(syma));

    SemanticAnalysis sema(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(sema));
}

TEST_F(SemanticAnalysisTests, ControlCheck_ContinueInLoop_Negative) {
    Parser parser(diags, "test", "foo :: () -> s64 { continue; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    SymbolAnalysis syma(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(syma));

    SemanticAnalysis sema(diags, opts);
    EXPECT_DEATH(unit->accept(sema), "");
}

TEST_F(SemanticAnalysisTests, TypeCheck_VariableInitializer_Positive) {
    Parser parser(diags, "test", "test :: () -> void { let x: s64 = 1; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    SymbolAnalysis syma(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(syma));

    SemanticAnalysis sema(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(sema));
}

TEST_F(SemanticAnalysisTests, TypeCheck_VariableInitializer_Negative) {
    Parser parser(diags, "test", "test :: () -> void { let x: s64 = \"test\"; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    SymbolAnalysis syma(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(syma));

    SemanticAnalysis sema(diags, opts);
    EXPECT_DEATH(unit->accept(sema), "");
}

TEST_F(SemanticAnalysisTests, TypeCheck_VoidReturn_Positive) {
    Parser parser(diags, "test", "foo :: () -> void { ret; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    SymbolAnalysis syma(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(syma));

    SemanticAnalysis sema(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(sema));
}

TEST_F(SemanticAnalysisTests, TypeCheck_VoidReturn_Negative) {
    Parser parser(diags, "test", "foo :: () -> s64 { ret; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    SymbolAnalysis syma(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(syma));

    SemanticAnalysis sema(diags, opts);
    EXPECT_DEATH(unit->accept(sema), "");
}

TEST_F(SemanticAnalysisTests, TypeCheck_Return_Positive) {
    Parser parser(diags, "test", "foo :: () -> s64 { ret 1; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    SymbolAnalysis syma(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(syma));

    SemanticAnalysis sema(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(sema));
}

TEST_F(SemanticAnalysisTests, TypeCheck_Return_Negative) {
    Parser parser(diags, "test", "foo :: () -> s64 { ret \"test\"; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    SymbolAnalysis syma(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(syma));

    SemanticAnalysis sema(diags, opts);
    EXPECT_DEATH(unit->accept(sema), "");
}

TEST_F(SemanticAnalysisTests, TypeCheck_CastReturn_Positive) {
    Parser parser(diags, "test", "foo :: () -> f32 { ret 1; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    SymbolAnalysis syma(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(syma));

    SemanticAnalysis sema(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(sema));
}

} // namespace stm::test

//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/core/Diagnostics.hpp"
#include "stmc/parser/Parser.hpp"
#include "stmc/tree/Decl.hpp"
#include "stmc/tree/SymbolAnalysis.hpp"

#include "gtest/gtest.h"

namespace stm::test {

class SymbolAnalysisTests : public ::testing::Test {
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

TEST_F(SymbolAnalysisTests, VariableRef_Positive) {
    Parser parser(diags, "test", "test :: () -> s64 { let x: s64 = 0; ret x; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    SymbolAnalysis syma(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(syma));
}

TEST_F(SymbolAnalysisTests, VariableRef_Negative) {
    Parser parser(diags, "test", "test :: () -> s64 { let x: s64 = 0; ret y; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    SymbolAnalysis syma(diags, opts);
    EXPECT_DEATH(unit->accept(syma), "");
}

TEST_F(SymbolAnalysisTests, CalleeRef_Positive) {
    Parser parser(diags, "test", "foo :: () -> s64 { ret bar(); } bar :: () -> s64 { ret 0; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    SymbolAnalysis syma(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(syma));
}

TEST_F(SymbolAnalysisTests, ParamRef_Positive) {
    Parser parser(diags, "test", "foo :: (a: s64) -> s64 { ret a; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    SymbolAnalysis syma(diags, opts);
    EXPECT_NO_FATAL_FAILURE(unit->accept(syma));
}

} // namespace stm::test

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

} // namespace stm::test

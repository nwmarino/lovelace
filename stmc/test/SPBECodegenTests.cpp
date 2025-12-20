//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/core/Diagnostics.hpp"
#include "stmc/parser/Parser.hpp"
#include "stmc/tree/Decl.hpp"
#include "stmc/tree/SemanticAnalysis.hpp"
#include "stmc/tree/SymbolAnalysis.hpp"
#include "stmc/codegen/SPBECodegen.hpp"

#include "gtest/gtest.h"

namespace stm::test {

class SPBECodegenTests : public ::testing::Test {
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

} // namespace stm::test

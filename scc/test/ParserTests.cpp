//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "ast/Decl.hpp"
#include "ast/Parser.hpp"
#include "core/TranslationUnit.hpp"

#include <gtest/gtest.h>

namespace scc::test {

class ParserTests : public ::testing::Test {};

TEST_F(ParserTests, ParseEmptyFunction) {
    TranslationUnit unit {};

    Parser parser("test", "int main();");
    parser.parse(unit);

    EXPECT_EQ(unit.num_decls(), 1);

    auto decl = unit.get_decl(0);
    EXPECT_NE(decl, nullptr);

    auto fn = dynamic_cast<const FunctionDecl*>(decl);
    EXPECT_NE(fn, nullptr);
    EXPECT_EQ(fn->name(), "main");
    EXPECT_EQ(fn->get_type().to_string(), "int ()");
    EXPECT_FALSE(fn->has_params());
    EXPECT_FALSE(fn->has_body());
}

} // namespace scc::test

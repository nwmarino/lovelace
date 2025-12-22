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
#include "stmc/tree/Type.hpp"

#include <gtest/gtest.h>

namespace stm::test {

class DeclParserTests : public ::testing::Test {
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

TEST_F(DeclParserTests, EmptyFunction) {
    Parser parser(diags, "test", "test :: () -> void;");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    const FunctionDecl* FD = dynamic_cast<FunctionDecl*>(unit->get_decls()[0]);
    EXPECT_NE(FD, nullptr);
    EXPECT_EQ(FD->get_name(), "test");
    EXPECT_FALSE(FD->has_runes());
    EXPECT_FALSE(FD->has_params());
    EXPECT_FALSE(FD->has_body());
}

TEST_F(DeclParserTests, FunctionWithBody) {
    Parser parser(diags, "test", "test :: () -> s64 { ret 0; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    const FunctionDecl* FD = dynamic_cast<FunctionDecl*>(unit->get_decls()[0]);
    EXPECT_NE(FD, nullptr);
    EXPECT_EQ(FD->get_name(), "test");
    EXPECT_TRUE(FD->has_body());

    const BlockStmt* BS = dynamic_cast<const BlockStmt*>(FD->get_body());
    EXPECT_NE(BS, nullptr);
    EXPECT_EQ(BS->num_stmts(), 1);
    
    const RetStmt* RS = dynamic_cast<const RetStmt*>(BS->get_stmt(0));
    EXPECT_NE(RS, nullptr);
    EXPECT_TRUE(RS->has_expr());

    const IntegerLiteral* IL = dynamic_cast<const IntegerLiteral*>(RS->get_expr());
    EXPECT_NE(IL, nullptr);
    EXPECT_EQ(IL->get_value(), 0);
}

TEST_F(DeclParserTests, FunctionParameters) {
    Parser parser(diags, "test", "test :: (a: s64, b: char) -> void;");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    const FunctionDecl* FD = dynamic_cast<FunctionDecl*>(unit->get_decls()[0]);
    EXPECT_NE(FD, nullptr);
    EXPECT_EQ(FD->get_name(), "test");
    EXPECT_TRUE(FD->has_params());
    EXPECT_EQ(FD->num_params(), 2);

    const ParameterDecl* P1 = FD->get_param(0);
    EXPECT_NE(P1, nullptr);
    EXPECT_EQ(P1->get_name(), "a");
    EXPECT_EQ(P1->get_type().to_string(), "s64");

    const ParameterDecl* P2 = FD->get_param(1);
    EXPECT_NE(P2, nullptr);
    EXPECT_EQ(P2->get_name(), "b");
    EXPECT_EQ(P2->get_type().to_string(), "char");
}

TEST_F(DeclParserTests, Global) {
    Parser parser(diags, "test", "glob :: s64");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    const VariableDecl* VD = dynamic_cast<VariableDecl*>(unit->get_decls()[0]);
    EXPECT_NE(VD, nullptr);
    EXPECT_EQ(VD->get_name(), "glob");
    EXPECT_EQ(VD->get_type().to_string(), "s64");
    EXPECT_FALSE(VD->has_init());
}

TEST_F(DeclParserTests, GlobalWithInitializer) {
    Parser parser(diags, "test", "glob :: s64 = 5");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    const VariableDecl* VD = dynamic_cast<VariableDecl*>(unit->get_decls()[0]);
    EXPECT_NE(VD, nullptr);
    EXPECT_EQ(VD->get_name(), "glob");
    EXPECT_EQ(VD->get_type().to_string(), "s64");
    EXPECT_TRUE(VD->has_init());

    const IntegerLiteral* IL = dynamic_cast<const IntegerLiteral*>(VD->get_init());
    EXPECT_NE(IL, nullptr);
    EXPECT_EQ(IL->get_value(), 5);
}

TEST_F(DeclParserTests, EnumDefaultType) {
    Parser parser(diags, "test", "Colors :: enum { Red, Blue = 0, Yellow = -7 }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    const EnumDecl* ED = dynamic_cast<EnumDecl*>(unit->get_decl(0));
    EXPECT_NE(ED, nullptr);
    EXPECT_EQ(ED->get_name(), "Colors");
    EXPECT_EQ(ED->num_variants(), 3);

    const EnumType* ET = dynamic_cast<const EnumType*>(ED->get_type());
    EXPECT_NE(ET, nullptr);
    EXPECT_EQ(ET->to_string(), "Colors");

    const TypeUse& underlying = ET->get_underlying();
    EXPECT_EQ(underlying.to_string(), "s64");

    const VariantDecl* V1 = ED->get_variant(0);
    EXPECT_NE(V1, nullptr);
    EXPECT_EQ(V1->get_name(), "Red");
    EXPECT_EQ(V1->get_value(), 0);

    const VariantDecl* V2 = ED->get_variant(1);
    EXPECT_NE(V2, nullptr);
    EXPECT_EQ(V2->get_name(), "Blue");
    EXPECT_EQ(V2->get_value(), 0);

    const VariantDecl* V3 = ED->get_variant(2);
    EXPECT_NE(V3, nullptr);
    EXPECT_EQ(V3->get_name(), "Yellow");
    EXPECT_EQ(V3->get_value(), -7);
}

TEST_F(DeclParserTests, EnumCustomType) {
    Parser parser(diags, "test", "Colors :: enum u16 { Red, Blue = 0, Yellow = -7 }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    const EnumDecl* ED = dynamic_cast<EnumDecl*>(unit->get_decl(0));
    EXPECT_NE(ED, nullptr);
    EXPECT_EQ(ED->get_name(), "Colors");
    EXPECT_EQ(ED->num_variants(), 3);

    const EnumType* ET = dynamic_cast<const EnumType*>(ED->get_type());
    EXPECT_NE(ET, nullptr);
    EXPECT_EQ(ET->to_string(), "Colors");

    const TypeUse& underlying = ET->get_underlying();
    EXPECT_EQ(underlying.to_string(), "u16");
}

} // namespace stm::test

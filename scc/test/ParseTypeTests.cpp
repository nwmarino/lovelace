//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "ast/Decl.hpp"
#include "ast/Expr.hpp"
#include "ast/Parser.hpp"
#include "ast/Stmt.hpp"
#include "ast/Type.hpp"
#include "core/Logger.hpp"

#include "gtest/gtest.h"
#include <gtest/gtest.h>

namespace scc::test {

class ParseTypeTests : public ::testing::Test {
protected:
    void SetUp() override {
        Logger::init();
    }
};

TEST_F(ParseTypeTests, RedefinePreStructValid) {
    Parser parser("test", "struct A; struct A { int a; };");
    TranslationUnitDecl* unit = nullptr;
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 0);
    EXPECT_EQ(unit->num_tags(), 1);

    auto structure = dynamic_cast<const RecordDecl*>(unit->get_tag("A"));
    EXPECT_NE(structure, nullptr);
    EXPECT_EQ(structure->get_name(), "A");
    EXPECT_EQ(structure->get_type()->to_string(), "struct A");
    EXPECT_EQ(structure->num_fields(), 1);

    auto f1 = structure->get_field("a");
    EXPECT_NE(f1, nullptr);
    EXPECT_EQ(f1->get_type().to_string(), "int");
}

TEST_F(ParseTypeTests, RedefinePostStructValid) {
    Parser parser("test", "struct A { int a; }; struct A; ");
    TranslationUnitDecl* unit = nullptr;
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 0);
    EXPECT_EQ(unit->num_tags(), 1);

    auto structure = dynamic_cast<const RecordDecl*>(unit->get_tag("A"));
    EXPECT_NE(structure, nullptr);
    EXPECT_EQ(structure->get_name(), "A");
    EXPECT_EQ(structure->get_type()->to_string(), "struct A");
    EXPECT_EQ(structure->num_fields(), 1);

    auto f1 = structure->get_field("a");
    EXPECT_NE(f1, nullptr);
    EXPECT_EQ(f1->get_type().to_string(), "int");
}

TEST_F(ParseTypeTests, RedefineStructInvalid) {
    Parser parser("test", "struct A { int a; } struct A { int a; };");
    TranslationUnitDecl* unit = nullptr;
    ASSERT_DEATH(unit = parser.parse(), "");
}

TEST_F(ParseTypeTests, StructForwardDeclaration) {
    Parser parser("test", "struct A { struct B* b; };");
    TranslationUnitDecl* unit = nullptr;
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 0);
    EXPECT_EQ(unit->num_tags(), 2);

    auto t1 = dynamic_cast<const RecordDecl*>(unit->get_tag("A"));
    EXPECT_NE(t1, nullptr);
    EXPECT_EQ(t1->num_decls(), 1);

    auto f1 = dynamic_cast<const FieldDecl*>(t1->get_field(0));
    EXPECT_NE(f1, nullptr);
    EXPECT_EQ(f1->get_name(), "b");
    EXPECT_EQ(f1->get_type().to_string(), "struct B*");
}

} // namespace scc::test

//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "scc/ast/Decl.hpp"
#include "scc/ast/Expr.hpp"
#include "scc/ast/Parser.hpp"
#include "scc/ast/Type.hpp"
#include "scc/core/Logger.hpp"

#include "gtest/gtest.h"

namespace scc::test {

class ParseTypeTests : public ::testing::Test {
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

TEST_F(ParseTypeTests, Struct_Positive_LaterDefinitionWithBody) {
    Parser parser("test", "struct A; struct A { int a; };");
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

TEST_F(ParseTypeTests, Struct_Positive_RedefinitionWithoutBody) {
    Parser parser("test", "struct A { int a; }; struct A; ");
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

TEST_F(ParseTypeTests, Struct_Negative_Redefinition) {
    Parser parser("test", "struct A { int a; } struct A { int a; };");
    EXPECT_DEATH(unit = parser.parse(), "");
}

TEST_F(ParseTypeTests, Struct_Positive_ForwardDeclaration) {
    Parser parser("test", "struct A { struct B* b; };");
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

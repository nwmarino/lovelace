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
#include "stmc/tree/Type.hpp"

#include <gtest/gtest.h>

namespace stm::test {

class TypeParserTests : public ::testing::Test {
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

TEST_F(TypeParserTests, BuiltinType) {
    Parser parser(diags, "test", "test :: () -> s64;");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    FunctionDecl* FD = dynamic_cast<FunctionDecl*>(unit->get_decls()[0]);
    EXPECT_NE(FD, nullptr);

    const TypeUse& return_type = FD->get_return_type();
    EXPECT_EQ(return_type.to_string(), "s64");

    const BuiltinType* BT = dynamic_cast<const BuiltinType*>(return_type.get_type());
    EXPECT_NE(BT, nullptr);
    EXPECT_EQ(BT->get_kind(), BuiltinType::Int64);
}

TEST_F(TypeParserTests, PointerType) {
    Parser parser(diags, "test", "test :: () -> *bool;");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    FunctionDecl* FD = dynamic_cast<FunctionDecl*>(unit->get_decls()[0]);
    EXPECT_NE(FD, nullptr);

    const TypeUse& return_type = FD->get_return_type();
    EXPECT_EQ(return_type.to_string(), "*bool");

    const PointerType* PT = dynamic_cast<const PointerType*>(return_type.get_type());
    EXPECT_NE(PT, nullptr);
    
    const TypeUse& pointee = PT->get_pointee();
    EXPECT_EQ(pointee.to_string(), "bool");

    const BuiltinType* BT = dynamic_cast<const BuiltinType*>(pointee.get_type());
    EXPECT_NE(BT, nullptr);
    EXPECT_EQ(BT->get_kind(), BuiltinType::Bool);
}

TEST_F(TypeParserTests, MutableType) {
    Parser parser(diags, "test", "test :: () -> mut void;");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    FunctionDecl* FD = dynamic_cast<FunctionDecl*>(unit->get_decls()[0]);
    EXPECT_NE(FD, nullptr);

    const TypeUse& return_type = FD->get_return_type();
    EXPECT_TRUE(return_type.is_mut());
    EXPECT_EQ(return_type.to_string(), "mut void");

    const BuiltinType* BT = dynamic_cast<const BuiltinType*>(return_type.get_type());
    EXPECT_NE(BT, nullptr);
    EXPECT_EQ(BT->get_kind(), BuiltinType::Void);
}

TEST_F(TypeParserTests, MutablePointerToVoidType) {
    Parser parser(diags, "test", "test :: () -> mut *void;");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    FunctionDecl* FD = dynamic_cast<FunctionDecl*>(unit->get_decls()[0]);
    EXPECT_NE(FD, nullptr);

    const TypeUse& return_type = FD->get_return_type();
    EXPECT_TRUE(return_type.is_mut());
    EXPECT_EQ(return_type.to_string(), "mut *void");

    const PointerType* PT = dynamic_cast<const PointerType*>(return_type.get_type());
    EXPECT_NE(PT, nullptr);
    EXPECT_EQ(PT->to_string(), "*void");

    const TypeUse& pointee = PT->get_pointee();
    EXPECT_FALSE(pointee.is_mut());
    EXPECT_EQ(pointee.to_string(), "void");

    const BuiltinType* BT = dynamic_cast<const BuiltinType*>(pointee.get_type());
    EXPECT_NE(BT, nullptr);
    EXPECT_EQ(BT->get_kind(), BuiltinType::Void);
}

TEST_F(TypeParserTests, PointerToMutableVoidType) {
    Parser parser(diags, "test", "test :: () -> *mut void;");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    FunctionDecl* FD = dynamic_cast<FunctionDecl*>(unit->get_decls()[0]);
    EXPECT_NE(FD, nullptr);

    const TypeUse& return_type = FD->get_return_type();
    EXPECT_FALSE(return_type.is_mut());
    EXPECT_EQ(return_type.to_string(), "*mut void");

    const PointerType* PT = dynamic_cast<const PointerType*>(return_type.get_type());
    EXPECT_NE(PT, nullptr);
    EXPECT_EQ(PT->to_string(), "*mut void");

    const TypeUse& pointee = PT->get_pointee();
    EXPECT_TRUE(pointee.is_mut());
    EXPECT_EQ(pointee.to_string(), "mut void");

    const BuiltinType* BT = dynamic_cast<const BuiltinType*>(pointee.get_type());
    EXPECT_NE(BT, nullptr);
    EXPECT_EQ(BT->get_kind(), BuiltinType::Void);
}

TEST_F(TypeParserTests, MutablePointerToMutableVoidType) {
    Parser parser(diags, "test", "test :: () -> mut *mut void;");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    FunctionDecl* FD = dynamic_cast<FunctionDecl*>(unit->get_decls()[0]);
    EXPECT_NE(FD, nullptr);

    const TypeUse& return_type = FD->get_return_type();
    EXPECT_TRUE(return_type.is_mut());
    EXPECT_EQ(return_type.to_string(), "mut *mut void");

    const PointerType* PT = dynamic_cast<const PointerType*>(return_type.get_type());
    EXPECT_NE(PT, nullptr);
    EXPECT_EQ(PT->to_string(), "*mut void");

    const TypeUse& pointee = PT->get_pointee();
    EXPECT_TRUE(pointee.is_mut());
    EXPECT_EQ(pointee.to_string(), "mut void");

    const BuiltinType* BT = dynamic_cast<const BuiltinType*>(pointee.get_type());
    EXPECT_NE(BT, nullptr);
    EXPECT_EQ(BT->get_kind(), BuiltinType::Void);
}

} // namespace stm::test

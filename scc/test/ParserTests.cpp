//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "ast/Decl.hpp"
#include "ast/Expr.hpp"
#include "ast/Parser.hpp"
#include "ast/Stmt.hpp"
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

TEST_F(ParserTests, ParseExternFunction) {
    TranslationUnit unit {};

    Parser parser("test", "extern int main();");
    parser.parse(unit);

    EXPECT_EQ(unit.num_decls(), 1);

    auto decl = unit.get_decl(0);
    EXPECT_NE(decl, nullptr);
    EXPECT_EQ(decl->storage_class(), StorageClass::Extern);

    auto fn = dynamic_cast<const FunctionDecl*>(decl);
    EXPECT_NE(fn, nullptr);
    EXPECT_EQ(fn->name(), "main");
    EXPECT_EQ(fn->get_type().to_string(), "int ()"); 
    EXPECT_FALSE(fn->has_params());
    EXPECT_FALSE(fn->has_body());
}

TEST_F(ParserTests, ParseStaticFunction) {
    TranslationUnit unit {};

    Parser parser("test", "static int main();");
    parser.parse(unit);

    EXPECT_EQ(unit.num_decls(), 1);

    auto decl = unit.get_decl(0);
    EXPECT_NE(decl, nullptr);
    EXPECT_EQ(decl->storage_class(), StorageClass::Static);

    auto fn = dynamic_cast<const FunctionDecl*>(decl);
    EXPECT_NE(fn, nullptr);
    EXPECT_EQ(fn->name(), "main");
    EXPECT_EQ(fn->get_type().to_string(), "int ()");
    EXPECT_FALSE(fn->has_params());
    EXPECT_FALSE(fn->has_body());
}

TEST_F(ParserTests, ParseUninitializedGlobal) {
    TranslationUnit unit {};

    Parser parser("test", "int x;");
    parser.parse(unit);

    EXPECT_EQ(unit.num_decls(), 1);

    auto decl = unit.get_decl(0);
    EXPECT_NE(decl, nullptr);

    auto var = dynamic_cast<const VariableDecl*>(decl);
    EXPECT_NE(var, nullptr);
    EXPECT_EQ(var->name(), "x");
    EXPECT_EQ(var->get_type().to_string(), "int");
    EXPECT_FALSE(var->has_initializer());
}

TEST_F(ParserTests, ParseExternGlobal) {
    TranslationUnit unit {};

    Parser parser("test", "extern int x;");
    parser.parse(unit);

    EXPECT_EQ(unit.num_decls(), 1);

    auto decl = unit.get_decl(0);
    EXPECT_NE(decl, nullptr);
    EXPECT_EQ(decl->storage_class(), StorageClass::Extern);

    auto var = dynamic_cast<const VariableDecl*>(decl);
    EXPECT_NE(var, nullptr);
    EXPECT_EQ(var->name(), "x");
    EXPECT_EQ(var->get_type().to_string(), "int"); 
    EXPECT_FALSE(var->has_initializer());
}

TEST_F(ParserTests, ParseStaticGlobal) {
    TranslationUnit unit {};

    Parser parser("test", "static int x;");
    parser.parse(unit);

    EXPECT_EQ(unit.num_decls(), 1);

    auto decl = unit.get_decl(0);
    EXPECT_NE(decl, nullptr);
    EXPECT_EQ(decl->storage_class(), StorageClass::Static);

    auto var = dynamic_cast<const VariableDecl*>(decl);
    EXPECT_NE(var, nullptr);
    EXPECT_EQ(var->name(), "x");
    EXPECT_EQ(var->get_type().to_string(), "int"); 
    EXPECT_FALSE(var->has_initializer());
}

TEST_F(ParserTests, ParseAutoGlobal) {
    TranslationUnit unit {};

    Parser parser("test", "auto x = 5;");
    parser.parse(unit);

    EXPECT_EQ(unit.num_decls(), 1);

    auto decl = unit.get_decl(0);
    EXPECT_NE(decl, nullptr);
    EXPECT_EQ(decl->storage_class(), StorageClass::Auto);

    auto var = dynamic_cast<const VariableDecl*>(decl);
    EXPECT_NE(var, nullptr);
    EXPECT_EQ(var->name(), "x");
    EXPECT_EQ(var->get_type().to_string(), "int"); 
    EXPECT_TRUE(var->has_initializer());

    auto init = dynamic_cast<const IntegerLiteral*>(var->get_initializer());
    EXPECT_NE(init, nullptr);
    EXPECT_EQ(init->get_value(), 5);
}

TEST_F(ParserTests, ParseUninitializedLocal) {
    TranslationUnit unit {};

    Parser parser("test", "int main() { int x; }");
    parser.parse(unit);

    EXPECT_EQ(unit.num_decls(), 1);

    auto decl = unit.get_decl(0);
    EXPECT_NE(decl, nullptr);

    auto fn = dynamic_cast<const FunctionDecl*>(decl);
    EXPECT_NE(fn, nullptr);
    EXPECT_TRUE(fn->has_body());
    
    auto compound = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(compound, nullptr);
    EXPECT_EQ(compound->num_stmts(), 1);

    auto dstmt = dynamic_cast<const DeclStmt*>(compound->get_stmt(0));
    EXPECT_NE(dstmt, nullptr);

    auto var = dynamic_cast<const VariableDecl*>(dstmt->get_decl());
    EXPECT_NE(var, nullptr);
    EXPECT_EQ(var->name(), "x");
    EXPECT_EQ(var->get_type().to_string(), "int");
    EXPECT_FALSE(var->has_initializer());
}

TEST_F(ParserTests, ParseStaticLocal) {
    TranslationUnit unit {};

    Parser parser("test", "int main() { static int x; }");
    parser.parse(unit);

    EXPECT_EQ(unit.num_decls(), 1);

    auto decl = unit.get_decl(0);
    EXPECT_NE(decl, nullptr);

    auto fn = dynamic_cast<const FunctionDecl*>(decl);
    EXPECT_NE(fn, nullptr);
    EXPECT_TRUE(fn->has_body());
    
    auto compound = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(compound, nullptr);
    EXPECT_EQ(compound->num_stmts(), 1);

    auto dstmt = dynamic_cast<const DeclStmt*>(compound->get_stmt(0));
    EXPECT_NE(dstmt, nullptr);

    auto var = dynamic_cast<const VariableDecl*>(dstmt->get_decl());
    EXPECT_NE(var, nullptr);
    EXPECT_EQ(var->storage_class(), StorageClass::Static);
    EXPECT_EQ(var->name(), "x");
    EXPECT_EQ(var->get_type().to_string(), "int");
    EXPECT_FALSE(var->has_initializer());
}

TEST_F(ParserTests, ParseAutoLocal) {
    TranslationUnit unit {};

    Parser parser("test", "int main() { auto x = 12; }");
    parser.parse(unit);

    EXPECT_EQ(unit.num_decls(), 1);

    auto decl = unit.get_decl(0);
    EXPECT_NE(decl, nullptr);

    auto fn = dynamic_cast<const FunctionDecl*>(decl);
    EXPECT_NE(fn, nullptr);
    EXPECT_TRUE(fn->has_body());
    
    auto compound = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(compound, nullptr);
    EXPECT_EQ(compound->num_stmts(), 1);

    auto dstmt = dynamic_cast<const DeclStmt*>(compound->get_stmt(0));
    EXPECT_NE(dstmt, nullptr);

    auto var = dynamic_cast<const VariableDecl*>(dstmt->get_decl());
    EXPECT_NE(var, nullptr);
    EXPECT_EQ(var->storage_class(), StorageClass::Auto);
    EXPECT_EQ(var->name(), "x");
    EXPECT_EQ(var->get_type().to_string(), "int");
    EXPECT_TRUE(var->has_initializer());
}

TEST_F(ParserTests, ParseCastBasic) {
    TranslationUnit unit {};

    Parser parser("test", "int main() { float x = (float) 3.14; }");
    parser.parse(unit);

    EXPECT_EQ(unit.num_decls(), 1);

    auto decl = unit.get_decl(0);
    EXPECT_NE(decl, nullptr);

    auto fn = dynamic_cast<const FunctionDecl*>(decl);
    EXPECT_NE(fn, nullptr);
    EXPECT_TRUE(fn->has_body());
    
    auto compound = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(compound, nullptr);
    EXPECT_EQ(compound->num_stmts(), 1);

    auto dstmt = dynamic_cast<const DeclStmt*>(compound->get_stmt(0));
    EXPECT_NE(dstmt, nullptr);

    auto var = dynamic_cast<const VariableDecl*>(dstmt->get_decl());
    EXPECT_NE(var, nullptr);
    EXPECT_EQ(var->name(), "x");
    EXPECT_EQ(var->get_type().to_string(), "float");
    EXPECT_TRUE(var->has_initializer());

    auto init = dynamic_cast<const CastExpr*>(var->get_initializer());
    EXPECT_NE(init, nullptr);
    EXPECT_EQ(init->get_type().to_string(), "float");
}

} // namespace scc::test

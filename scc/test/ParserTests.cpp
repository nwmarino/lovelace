//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "ast/Decl.hpp"
#include "ast/Expr.hpp"
#include "ast/Parser.hpp"
#include "ast/Stmt.hpp"
#include "core/TranslationUnit.hpp"

#include "gtest/gtest.h"
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

TEST_F(ParserTests, ParseCallNamed) {
    TranslationUnit unit {};

    Parser parser("test", "int foo(); int main() { return foo(); }");
    parser.parse(unit);

    EXPECT_EQ(unit.num_decls(), 2);

    auto foo_decl = unit.get_decl("foo");
    EXPECT_NE(foo_decl, nullptr);

    auto main_decl = unit.get_decl("main");
    EXPECT_NE(main_decl, nullptr);

    auto fn = dynamic_cast<const FunctionDecl*>(main_decl);
    EXPECT_NE(fn, nullptr);
    EXPECT_TRUE(fn->has_body());
    
    auto compound = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(compound, nullptr);
    EXPECT_EQ(compound->num_stmts(), 1);

    auto ret = dynamic_cast<const ReturnStmt*>(compound->get_stmt(0));
    EXPECT_NE(ret, nullptr);
    EXPECT_TRUE(ret->has_expr());

    auto call = dynamic_cast<const CallExpr*>(ret->get_expr());
    EXPECT_NE(call, nullptr);
    EXPECT_FALSE(call->has_args());

    auto callee = dynamic_cast<const RefExpr*>(call->get_callee());
    EXPECT_NE(callee, nullptr);
    EXPECT_EQ(callee->get_name(), "foo");
    EXPECT_EQ(callee->get_decl(), foo_decl);
}

TEST_F(ParserTests, ParseCallNamedArgs) {
    TranslationUnit unit {};

    Parser parser("test", "int foo(int x); int main() { return foo(1); }");
    parser.parse(unit);

    EXPECT_EQ(unit.num_decls(), 2);

    auto foo_decl = unit.get_decl("foo");
    EXPECT_NE(foo_decl, nullptr);

    auto main_decl = unit.get_decl("main");
    EXPECT_NE(main_decl, nullptr);

    auto fn = dynamic_cast<const FunctionDecl*>(main_decl);
    EXPECT_NE(fn, nullptr);
    EXPECT_TRUE(fn->has_body());
    
    auto compound = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(compound, nullptr);
    EXPECT_EQ(compound->num_stmts(), 1);

    auto ret = dynamic_cast<const ReturnStmt*>(compound->get_stmt(0));
    EXPECT_NE(ret, nullptr);
    EXPECT_TRUE(ret->has_expr());

    auto call = dynamic_cast<const CallExpr*>(ret->get_expr());
    EXPECT_NE(call, nullptr);
    EXPECT_TRUE(call->has_args());
    EXPECT_EQ(call->num_args(), 1);

    auto callee = dynamic_cast<const RefExpr*>(call->get_callee());
    EXPECT_NE(callee, nullptr);
    EXPECT_EQ(callee->get_name(), "foo");
    EXPECT_EQ(callee->get_decl(), foo_decl);

    auto arg = dynamic_cast<const IntegerLiteral*>(call->get_arg(0));
    EXPECT_NE(arg, nullptr);
    EXPECT_EQ(arg->get_value(), 1);
}

TEST_F(ParserTests, ParseParenBasic) {
    TranslationUnit unit {};

    Parser parser("test", "int main() { return (1); }");
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

    auto ret = dynamic_cast<const ReturnStmt*>(compound->get_stmt(0));
    EXPECT_NE(ret, nullptr);
    EXPECT_TRUE(ret->has_expr());

    auto paren = dynamic_cast<const ParenExpr*>(ret->get_expr());
    EXPECT_NE(paren, nullptr);
    EXPECT_NE(paren->get_expr(), nullptr);

    auto integer = dynamic_cast<const IntegerLiteral*>(paren->get_expr());
    EXPECT_NE(integer, nullptr);
    EXPECT_EQ(integer->get_value(), 1);
}

TEST_F(ParserTests, ParseParenReference) {
    TranslationUnit unit {};

    Parser parser("test", "int main() { int x = 5; return (x); }");
    parser.parse(unit);

    EXPECT_EQ(unit.num_decls(), 1);

    auto decl = unit.get_decl(0);
    EXPECT_NE(decl, nullptr);

    auto fn = dynamic_cast<const FunctionDecl*>(decl);
    EXPECT_NE(fn, nullptr);
    EXPECT_TRUE(fn->has_body());
    
    auto compound = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(compound, nullptr);
    EXPECT_EQ(compound->num_stmts(), 2);

    auto dstmt = dynamic_cast<const DeclStmt*>(compound->get_stmt(0));
    EXPECT_NE(dstmt, nullptr);
    
    auto var = dynamic_cast<const VariableDecl*>(dstmt->get_decl());
    EXPECT_NE(var, nullptr);
    EXPECT_EQ(var->name(), "x");
    EXPECT_TRUE(var->has_initializer());

    auto ret = dynamic_cast<const ReturnStmt*>(compound->get_stmt(1));
    EXPECT_NE(ret, nullptr);
    EXPECT_TRUE(ret->has_expr());

    auto paren = dynamic_cast<const ParenExpr*>(ret->get_expr());
    EXPECT_NE(paren, nullptr);
    EXPECT_NE(paren->get_expr(), nullptr);

    auto ref = dynamic_cast<const RefExpr*>(paren->get_expr());
    EXPECT_NE(ref, nullptr);
    EXPECT_EQ(ref->get_name(), "x");
    EXPECT_EQ(ref->get_decl(), var);
}

TEST_F(ParserTests, ParseFunctionRedefine) {
    TranslationUnit unit {};

    Parser parser("test", "int foo(); int foo() { return 1; }");
    EXPECT_NO_FATAL_FAILURE(parser.parse(unit));

    EXPECT_EQ(unit.num_decls(), 1);

    auto decl = unit.get_decl(0);
    EXPECT_NE(decl, nullptr);

    auto fn = dynamic_cast<const FunctionDecl*>(decl);
    EXPECT_NE(fn, nullptr);
    EXPECT_EQ(fn->name(), "foo");
    EXPECT_TRUE(fn->has_body());

    auto body = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(body, nullptr);
    EXPECT_FALSE(body->empty());
}

TEST_F(ParserTests, ParseFunctionRedefineInvalid) {
    TranslationUnit unit {};

    Parser parser("test", "int foo(); int foo(int x) { return 1; }");
    ASSERT_DEATH(parser.parse(unit), "");
}

TEST_F(ParserTests, ParseFunctionDoubleDefinition) {
    TranslationUnit unit {};

    Parser parser("test", "int foo() { return 0; } int foo() { return 1; }");
    ASSERT_DEATH(parser.parse(unit), "");
}

TEST_F(ParserTests, ParseFunctionRedefineWithParams) {
    TranslationUnit unit {};

    Parser parser("test", "int foo(int x); int foo(int x) { return 1; }");
    EXPECT_NO_FATAL_FAILURE(parser.parse(unit));

    EXPECT_EQ(unit.num_decls(), 1);

    auto decl = unit.get_decl(0);
    EXPECT_NE(decl, nullptr);

    auto fn = dynamic_cast<const FunctionDecl*>(decl);
    EXPECT_NE(fn, nullptr);
    EXPECT_EQ(fn->name(), "foo");
    EXPECT_TRUE(fn->has_params());
    EXPECT_TRUE(fn->has_body());

    auto param = dynamic_cast<const ParameterDecl*>(fn->get_param(0));
    EXPECT_NE(param, nullptr);
    EXPECT_EQ(param->name(), "x");

    auto body = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(body, nullptr);
    EXPECT_FALSE(body->empty());
}

TEST_F(ParserTests, ParseArrayTypeVariable) {
    TranslationUnit unit {};

    Parser parser("test", "int x[5];");
    parser.parse(unit);

    EXPECT_EQ(unit.num_decls(), 1);

    auto var = dynamic_cast<const VariableDecl*>(unit.get_scope()->get("x"));
    EXPECT_NE(var, nullptr);
    EXPECT_EQ(var->get_type().to_string(), "int[5]");
}

TEST_F(ParserTests, ParseArrayTypeParameter) {
    TranslationUnit unit {};

    Parser parser("test", "int foo(int x[5]);");
    parser.parse(unit);

    EXPECT_EQ(unit.num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit.get_scope()->get("foo"));
    EXPECT_NE(fn, nullptr);
    EXPECT_EQ(fn->get_type().to_string(), "int (int[5])");
}

TEST_F(ParserTests, ParseWhileLoop) {
    TranslationUnit unit {};

    Parser parser("test", "int main() { while (1) continue; }");
    parser.parse(unit);

    EXPECT_EQ(unit.num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit.get_scope()->get("main"));
    EXPECT_NE(fn, nullptr);
    EXPECT_TRUE(fn->has_body());

    auto body = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(body, nullptr);
    EXPECT_EQ(body->num_stmts(), 1);

    auto loop = dynamic_cast<const WhileStmt*>(body->get_stmt(0));
    EXPECT_NE(loop, nullptr);
    EXPECT_TRUE(loop->has_body());

    auto cond = dynamic_cast<const IntegerLiteral*>(loop->get_cond());
    EXPECT_NE(cond, nullptr);
    EXPECT_EQ(cond->get_value(), 1);

    auto cont = dynamic_cast<const ContinueStmt*>(loop->get_body());
    EXPECT_NE(cont, nullptr);
}

TEST_F(ParserTests, ParseWhileLoopEmpty) {
    TranslationUnit unit {};

    Parser parser("test", "int main() { while (1); }");
    parser.parse(unit);

    EXPECT_EQ(unit.num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit.get_scope()->get("main"));
    EXPECT_NE(fn, nullptr);
    EXPECT_TRUE(fn->has_body());

    auto body = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(body, nullptr);
    EXPECT_EQ(body->num_stmts(), 1);

    auto loop = dynamic_cast<const WhileStmt*>(body->get_stmt(0));
    EXPECT_NE(loop, nullptr);
    EXPECT_FALSE(loop->has_body());
}

TEST_F(ParserTests, ParseForLoop) {
    TranslationUnit unit {};

    Parser parser("test", "int main() { for (int i = 0; i < 5; ++i) break; }");
    parser.parse(unit);

    EXPECT_EQ(unit.num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit.get_scope()->get("main"));
    EXPECT_NE(fn, nullptr);
    EXPECT_TRUE(fn->has_body());

    auto body = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(body, nullptr);
    EXPECT_EQ(body->num_stmts(), 1);

    auto loop = dynamic_cast<const ForStmt*>(body->get_stmt(0));
    EXPECT_NE(loop, nullptr);
    EXPECT_TRUE(loop->has_init());
    EXPECT_TRUE(loop->has_cond());
    EXPECT_TRUE(loop->has_step());
    EXPECT_TRUE(loop->has_body());

    auto init = dynamic_cast<const DeclStmt*>(loop->get_init());
    EXPECT_NE(init, nullptr);

    auto iter = dynamic_cast<const VariableDecl*>(init->get_decl());
    EXPECT_NE(iter, nullptr);
    EXPECT_EQ(iter->name(), "i");

    auto cond = dynamic_cast<const BinaryExpr*>(loop->get_cond());
    EXPECT_NE(cond, nullptr);
    EXPECT_EQ(cond->get_operator(), BinaryExpr::LessThan);

    auto step = dynamic_cast<const UnaryExpr*>(loop->get_step());
    EXPECT_NE(step, nullptr);
    EXPECT_EQ(step->get_operator(), UnaryExpr::Increment);
    EXPECT_FALSE(step->is_postfix());

    auto brk = dynamic_cast<const BreakStmt*>(loop->get_body());
    EXPECT_NE(brk, nullptr);
}

TEST_F(ParserTests, ParseForLoopEmpty) {
    TranslationUnit unit {};

    Parser parser("test", "int main() { for (;;); }");
    parser.parse(unit);

    EXPECT_EQ(unit.num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit.get_scope()->get("main"));
    EXPECT_NE(fn, nullptr);
    EXPECT_TRUE(fn->has_body());

    auto body = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(body, nullptr);
    EXPECT_EQ(body->num_stmts(), 1);

    auto loop = dynamic_cast<const ForStmt*>(body->get_stmt(0));
    EXPECT_NE(loop, nullptr);
    EXPECT_FALSE(loop->has_init());
    EXPECT_FALSE(loop->has_cond());
    EXPECT_FALSE(loop->has_step());
    EXPECT_FALSE(loop->has_body());
}

TEST_F(ParserTests, ParseForLoopNoInit) {
    TranslationUnit unit {};

    Parser parser("test", "int main() { for (;5;1) {} }");
    parser.parse(unit);

    EXPECT_EQ(unit.num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit.get_scope()->get("main"));
    EXPECT_NE(fn, nullptr);
    EXPECT_TRUE(fn->has_body());

    auto body = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(body, nullptr);
    EXPECT_EQ(body->num_stmts(), 1);

    auto loop = dynamic_cast<const ForStmt*>(body->get_stmt(0));
    EXPECT_NE(loop, nullptr);
    EXPECT_FALSE(loop->has_init());
    EXPECT_TRUE(loop->has_cond());
    EXPECT_TRUE(loop->has_step());
    EXPECT_TRUE(loop->has_body());
}

TEST_F(ParserTests, ParseForLoopNoCond) {
    TranslationUnit unit {};

    Parser parser("test", "int main() { for (int i = 0;;++i) {} }");
    parser.parse(unit);

    EXPECT_EQ(unit.num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit.get_scope()->get("main"));
    EXPECT_NE(fn, nullptr);
    EXPECT_TRUE(fn->has_body());

    auto body = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(body, nullptr);
    EXPECT_EQ(body->num_stmts(), 1);

    auto loop = dynamic_cast<const ForStmt*>(body->get_stmt(0));
    EXPECT_NE(loop, nullptr);
    EXPECT_TRUE(loop->has_init());
    EXPECT_FALSE(loop->has_cond());
    EXPECT_TRUE(loop->has_step());
    EXPECT_TRUE(loop->has_body());
}

TEST_F(ParserTests, ParseForLoopNoStep) {
    TranslationUnit unit {};

    Parser parser("test", "int main() { for (int i = 0; i < 5;) {} }");
    parser.parse(unit);

    EXPECT_EQ(unit.num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit.get_scope()->get("main"));
    EXPECT_NE(fn, nullptr);
    EXPECT_TRUE(fn->has_body());

    auto body = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(body, nullptr);
    EXPECT_EQ(body->num_stmts(), 1);

    auto loop = dynamic_cast<const ForStmt*>(body->get_stmt(0));
    EXPECT_NE(loop, nullptr);
    EXPECT_TRUE(loop->has_init());
    EXPECT_TRUE(loop->has_cond());
    EXPECT_FALSE(loop->has_step());
    EXPECT_TRUE(loop->has_body());
}

TEST_F(ParserTests, ParseForLoopNoBody) {
    TranslationUnit unit {};

    Parser parser("test", "int main() { for (int i = 0; i < 5; ++i); }");
    parser.parse(unit);

    EXPECT_EQ(unit.num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit.get_scope()->get("main"));
    EXPECT_NE(fn, nullptr);
    EXPECT_TRUE(fn->has_body());

    auto body = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(body, nullptr);
    EXPECT_EQ(body->num_stmts(), 1);

    auto loop = dynamic_cast<const ForStmt*>(body->get_stmt(0));
    EXPECT_NE(loop, nullptr);
    EXPECT_TRUE(loop->has_init());
    EXPECT_TRUE(loop->has_cond());
    EXPECT_TRUE(loop->has_step());
    EXPECT_FALSE(loop->has_body());
}

TEST_F(ParserTests, ParseSubscriptBasic) {
    TranslationUnit unit {};

    Parser parser("test", "int main() { 1[5]; }");
    parser.parse(unit);

    EXPECT_EQ(unit.num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit.get_scope()->get("main"));
    EXPECT_NE(fn, nullptr);
    EXPECT_TRUE(fn->has_body());

    auto body = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(body, nullptr);
    EXPECT_EQ(body->num_stmts(), 1);

    auto expr = dynamic_cast<const ExprStmt*>(body->get_stmt(0));
    EXPECT_NE(expr, nullptr);

    auto ss = dynamic_cast<const SubscriptExpr*>(expr->get_expr());
    EXPECT_NE(ss, nullptr);

    auto base = dynamic_cast<const IntegerLiteral*>(ss->get_base());
    EXPECT_NE(base, nullptr);
    EXPECT_EQ(base->get_value(), 1);

    auto index = dynamic_cast<const IntegerLiteral*>(ss->get_index());
    EXPECT_NE(index, nullptr);
    EXPECT_EQ(index->get_value(), 5);
}

TEST_F(ParserTests, ParseTernaryBasic) {
    TranslationUnit unit {};

    Parser parser("test", "int main() { return 5 ? 0 : 1; }");
    parser.parse(unit);

    EXPECT_EQ(unit.num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit.get_scope()->get("main"));
    EXPECT_NE(fn, nullptr);
    EXPECT_TRUE(fn->has_body());

    auto body = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(body, nullptr);
    EXPECT_EQ(body->num_stmts(), 1);

    auto ret = dynamic_cast<const ReturnStmt*>(body->get_stmt(0));
    EXPECT_NE(ret, nullptr);
    EXPECT_TRUE(ret->has_expr());

    auto ternary = dynamic_cast<const TernaryExpr*>(ret->get_expr());
    EXPECT_NE(ternary, nullptr);

    auto cond = dynamic_cast<const IntegerLiteral*>(ternary->get_cond());
    EXPECT_NE(cond, nullptr);
    EXPECT_EQ(cond->get_value(), 5);

    auto tval = dynamic_cast<const IntegerLiteral*>(ternary->get_true_value());
    EXPECT_NE(tval, nullptr);
    EXPECT_EQ(tval->get_value(), 0);

    auto fval = dynamic_cast<const IntegerLiteral*>(ternary->get_false_value());
    EXPECT_NE(fval, nullptr);
    EXPECT_EQ(fval->get_value(), 1);
}

/*
TEST_F(ParserTests, ParseMemberBasic) {
    TranslationUnit unit {};

    Parser parser("test", "struct A { int a; }; int main() { struct A x; x.a; }");
    parser.parse(unit);

    EXPECT_EQ(unit.num_decls(), 2);

    auto fn = dynamic_cast<const FunctionDecl*>(unit.get_scope()->get("main"));
    EXPECT_NE(fn, nullptr);
    EXPECT_TRUE(fn->has_body());

    auto body = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(body, nullptr);
    EXPECT_EQ(body->num_stmts(), 2);

    auto dstmt = dynamic_cast<const DeclStmt*>(body->get_stmt(0));
    EXPECT_NE(dstmt, nullptr);

    auto expr = dynamic_cast<const ExprStmt*>(body->get_stmt(1));
    EXPECT_NE(expr, nullptr);

    auto member = dynamic_cast<const MemberExpr*>(expr->get_expr());
    EXPECT_NE(member, nullptr);
}
*/

} // namespace scc::test

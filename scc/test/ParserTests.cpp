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

#include <gtest/gtest.h>

namespace scc::test {

class ParserTests : public ::testing::Test {
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

TEST_F(ParserTests, ParseEmptyFunction) {
    Parser parser("test", "int main();");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit->get_decl("main"));
    EXPECT_NE(fn, nullptr);
    EXPECT_EQ(fn->get_name(), "main");
    EXPECT_EQ(fn->get_type().to_string(), "int ()");
    EXPECT_FALSE(fn->has_params());
    EXPECT_FALSE(fn->has_body());
}

TEST_F(ParserTests, ParseExternFunction) {
    Parser parser("test", "extern int main();");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit->get_decl("main"));
    EXPECT_NE(fn, nullptr);
    EXPECT_EQ(fn->get_storage_class(), StorageClass::Extern);
    EXPECT_EQ(fn->get_name(), "main");
    EXPECT_EQ(fn->get_type().to_string(), "int ()"); 
    EXPECT_FALSE(fn->has_params());
    EXPECT_FALSE(fn->has_body());
}

TEST_F(ParserTests, ParseStaticFunction) {
    Parser parser("test", "static int main();");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit->get_decl("main"));
    EXPECT_NE(fn, nullptr);
    EXPECT_EQ(fn->get_storage_class(), StorageClass::Static);
    EXPECT_EQ(fn->get_name(), "main");
    EXPECT_EQ(fn->get_type().to_string(), "int ()");
    EXPECT_FALSE(fn->has_params());
    EXPECT_FALSE(fn->has_body());
}

TEST_F(ParserTests, ParseUninitializedGlobal) {
    Parser parser("test", "int x;");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto var = dynamic_cast<const VariableDecl*>(unit->get_decl("x"));
    EXPECT_NE(var, nullptr);
    EXPECT_EQ(var->get_name(), "x");
    EXPECT_EQ(var->get_type().to_string(), "int");
    EXPECT_FALSE(var->has_init());
}

TEST_F(ParserTests, ParseExternGlobal) {
    Parser parser("test", "extern int x;");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto var = dynamic_cast<const VariableDecl*>(unit->get_decl("x"));
    EXPECT_NE(var, nullptr);
    EXPECT_EQ(var->get_storage_class(), StorageClass::Extern);
    EXPECT_EQ(var->get_name(), "x");
    EXPECT_EQ(var->get_type().to_string(), "int"); 
    EXPECT_FALSE(var->has_init());
}

TEST_F(ParserTests, ParseStaticGlobal) {
    Parser parser("test", "static int x;");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto var = dynamic_cast<const VariableDecl*>(unit->get_decl("x"));
    EXPECT_NE(var, nullptr);
    EXPECT_EQ(var->get_storage_class(), StorageClass::Static);
    EXPECT_EQ(var->get_name(), "x");
    EXPECT_EQ(var->get_type().to_string(), "int"); 
    EXPECT_FALSE(var->has_init());
}

TEST_F(ParserTests, ParseAutoGlobal) {
    Parser parser("test", "auto x = 5;");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto var = dynamic_cast<const VariableDecl*>(unit->get_decl("x"));
    EXPECT_NE(var, nullptr);
    EXPECT_EQ(var->get_storage_class(), StorageClass::Auto);
    EXPECT_EQ(var->get_name(), "x");
    EXPECT_EQ(var->get_type().to_string(), "int"); 
    EXPECT_TRUE(var->has_init());

    auto init = dynamic_cast<const IntegerLiteral*>(var->get_init());
    EXPECT_NE(init, nullptr);
    EXPECT_EQ(init->get_value(), 5);
}

TEST_F(ParserTests, ParseUninitializedLocal) {
    Parser parser("test", "int main() { int x; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit->get_decl("main"));
    EXPECT_NE(fn, nullptr);
    EXPECT_TRUE(fn->has_body());
    
    auto compound = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(compound, nullptr);
    EXPECT_EQ(compound->num_stmts(), 1);

    auto dstmt = dynamic_cast<const DeclStmt*>(compound->get_stmt(0));
    EXPECT_NE(dstmt, nullptr);

    auto var = dynamic_cast<const VariableDecl*>(dstmt->get_decl());
    EXPECT_NE(var, nullptr);
    EXPECT_EQ(var->get_name(), "x");
    EXPECT_EQ(var->get_type().to_string(), "int");
    EXPECT_FALSE(var->has_init());
}

TEST_F(ParserTests, ParseStaticLocal) {
    Parser parser("test", "int main() { static int x; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit->get_decl("main"));
    EXPECT_NE(fn, nullptr);
    EXPECT_TRUE(fn->has_body());
    
    auto compound = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(compound, nullptr);
    EXPECT_EQ(compound->num_stmts(), 1);

    auto dstmt = dynamic_cast<const DeclStmt*>(compound->get_stmt(0));
    EXPECT_NE(dstmt, nullptr);

    auto var = dynamic_cast<const VariableDecl*>(dstmt->get_decl());
    EXPECT_NE(var, nullptr);
    EXPECT_EQ(var->get_storage_class(), StorageClass::Static);
    EXPECT_EQ(var->get_name(), "x");
    EXPECT_EQ(var->get_type().to_string(), "int");
    EXPECT_FALSE(var->has_init());
}

TEST_F(ParserTests, ParseAutoLocal) {
    Parser parser("test", "int main() { auto x = 12; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit->get_decl("main"));
    EXPECT_NE(fn, nullptr);
    EXPECT_TRUE(fn->has_body());
    
    auto compound = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(compound, nullptr);
    EXPECT_EQ(compound->num_stmts(), 1);

    auto dstmt = dynamic_cast<const DeclStmt*>(compound->get_stmt(0));
    EXPECT_NE(dstmt, nullptr);

    auto var = dynamic_cast<const VariableDecl*>(dstmt->get_decl());
    EXPECT_NE(var, nullptr);
    EXPECT_EQ(var->get_storage_class(), StorageClass::Auto);
    EXPECT_EQ(var->get_name(), "x");
    EXPECT_EQ(var->get_type().to_string(), "int");
    EXPECT_TRUE(var->has_init());
}

TEST_F(ParserTests, ParseCastBasic) {
    Parser parser("test", "int main() { float x = (float) 3.14; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit->get_decl("main"));
    EXPECT_NE(fn, nullptr);
    EXPECT_TRUE(fn->has_body());
    
    auto compound = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(compound, nullptr);
    EXPECT_EQ(compound->num_stmts(), 1);

    auto dstmt = dynamic_cast<const DeclStmt*>(compound->get_stmt(0));
    EXPECT_NE(dstmt, nullptr);

    auto var = dynamic_cast<const VariableDecl*>(dstmt->get_decl());
    EXPECT_NE(var, nullptr);
    EXPECT_EQ(var->get_name(), "x");
    EXPECT_EQ(var->get_type().to_string(), "float");
    EXPECT_TRUE(var->has_init());

    auto init = dynamic_cast<const CastExpr*>(var->get_init());
    EXPECT_NE(init, nullptr);
    EXPECT_EQ(init->get_type().to_string(), "float");
}

TEST_F(ParserTests, ParseCallNamed) {
    Parser parser("test", "int foo(); int main() { return foo(); }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 2);

    auto foo_decl = unit->get_decl("foo");
    EXPECT_NE(foo_decl, nullptr);

    auto fn = dynamic_cast<const FunctionDecl*>(unit->get_decl("main"));
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
    Parser parser("test", "int foo(int x); int main() { return foo(1); }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 2);

    auto foo_decl = unit->get_decl("foo");
    EXPECT_NE(foo_decl, nullptr);

    auto main_decl = unit->get_decl("main");
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
    Parser parser("test", "int main() { return (1); }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit->get_decl("main"));
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
    Parser parser("test", "int main() { int x = 5; return (x); }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit->get_decl("main"));
    EXPECT_NE(fn, nullptr);
    EXPECT_TRUE(fn->has_body());
    
    auto compound = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(compound, nullptr);
    EXPECT_EQ(compound->num_stmts(), 2);

    auto dstmt = dynamic_cast<const DeclStmt*>(compound->get_stmt(0));
    EXPECT_NE(dstmt, nullptr);
    
    auto var = dynamic_cast<const VariableDecl*>(dstmt->get_decl());
    EXPECT_NE(var, nullptr);
    EXPECT_EQ(var->get_name(), "x");
    EXPECT_TRUE(var->has_init());

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
    Parser parser("test", "int foo(); int foo() { return 1; }");
    TranslationUnitDecl* unit = nullptr;
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit->get_decl("foo"));
    EXPECT_NE(fn, nullptr);
    EXPECT_EQ(fn->get_name(), "foo");
    EXPECT_TRUE(fn->has_body());

    auto body = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(body, nullptr);
    EXPECT_FALSE(body->empty());
}

TEST_F(ParserTests, ParseFunctionRedefineInvalid) {
    Parser parser("test", "int foo(); int foo(int x) { return 1; }");
    ASSERT_DEATH(EXPECT_NO_FATAL_FAILURE(unit = parser.parse());, "");
}

TEST_F(ParserTests, ParseFunctionDoubleDefinition) {
    Parser parser("test", "int foo() { return 0; } int foo() { return 1; }");
    ASSERT_DEATH(EXPECT_NO_FATAL_FAILURE(unit = parser.parse());, "");
}

TEST_F(ParserTests, ParseFunctionRedefineWithParams) {
    Parser parser("test", "int foo(int x); int foo(int x) { return 1; }");
    TranslationUnitDecl* unit = nullptr;
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit->get_decl("foo"));
    EXPECT_NE(fn, nullptr);
    EXPECT_EQ(fn->get_name(), "foo");
    EXPECT_TRUE(fn->has_params());
    EXPECT_TRUE(fn->has_body());

    auto param = dynamic_cast<const ParameterDecl*>(fn->get_param(0));
    EXPECT_NE(param, nullptr);
    EXPECT_EQ(param->get_name(), "x");

    auto body = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(body, nullptr);
    EXPECT_FALSE(body->empty());
}

TEST_F(ParserTests, ParseArrayTypeVariable) {
    Parser parser("test", "int x[5];");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto var = dynamic_cast<const VariableDecl*>(unit->get_decl("x"));
    EXPECT_NE(var, nullptr);
    EXPECT_EQ(var->get_type().to_string(), "int[5]");
}

TEST_F(ParserTests, ParseArrayTypeParameter) {
    Parser parser("test", "int foo(int x[5]);");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit->get_decl("foo"));
    EXPECT_NE(fn, nullptr);
    EXPECT_EQ(fn->get_type().to_string(), "int (int[5])");
}

TEST_F(ParserTests, ParseWhileLoop) {
    Parser parser("test", "int main() { while (1) continue; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit->get_decl("main"));
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
    Parser parser("test", "int main() { while (1); }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit->get_decl("main"));
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
    Parser parser("test", "int main() { for (int i = 0; i < 5; ++i) break; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit->get_decl("main"));
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
    EXPECT_EQ(iter->get_name(), "i");

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
    Parser parser("test", "int main() { for (;;); }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit->get_decl("main"));
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
    Parser parser("test", "int main() { for (;5;1) {} }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit->get_decl("main"));
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
    Parser parser("test", "int main() { for (int i = 0;;++i) {} }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit->get_decl("main"));
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
    Parser parser("test", "int main() { for (int i = 0; i < 5;) {} }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit->get_decl("main"));
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
    Parser parser("test", "int main() { for (int i = 0; i < 5; ++i); }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit->get_decl("main"));
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
    Parser parser("test", "int main() { 1[5]; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit->get_decl("main"));
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
    Parser parser("test", "int main() { return 5 ? 0 : 1; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit->get_decl("main"));
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

TEST_F(ParserTests, ParseSwitchBasic) {
    Parser parser("test", "int main() { switch (1) { case 0: return 0; case 1: return 1; } }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit->get_decl("main"));
    EXPECT_NE(fn, nullptr);
    EXPECT_TRUE(fn->has_body());

    auto body = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(body, nullptr);
    EXPECT_EQ(body->num_stmts(), 1);

    auto sw = dynamic_cast<const SwitchStmt*>(body->get_stmt(0));
    EXPECT_NE(sw, nullptr);
    EXPECT_EQ(sw->num_cases(), 2);
    EXPECT_FALSE(sw->has_default());

    auto c1 = dynamic_cast<const CaseStmt*>(sw->get_case(0));
    EXPECT_NE(c1, nullptr);

    auto c1b = dynamic_cast<const ReturnStmt*>(c1->get_body());
    EXPECT_NE(c1b, nullptr);

    auto c2 = dynamic_cast<const CaseStmt*>(sw->get_case(1));
    EXPECT_NE(c1, nullptr);

    auto c2b = dynamic_cast<const ReturnStmt*>(c2->get_body());
    EXPECT_NE(c1b, nullptr);
}

TEST_F(ParserTests, ParseSwitchDefault) {
    Parser parser("test", "int main() { switch (1) { case 0: return 0; case 1: return 1; default: return 2; } }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto fn = dynamic_cast<const FunctionDecl*>(unit->get_decl("main"));
    EXPECT_NE(fn, nullptr);
    EXPECT_TRUE(fn->has_body());

    auto body = dynamic_cast<const CompoundStmt*>(fn->get_body());
    EXPECT_NE(body, nullptr);
    EXPECT_EQ(body->num_stmts(), 1);

    auto sw = dynamic_cast<const SwitchStmt*>(body->get_stmt(0));
    EXPECT_NE(sw, nullptr);
    EXPECT_EQ(sw->num_cases(), 2);
    EXPECT_TRUE(sw->has_default());

    auto c1 = dynamic_cast<const CaseStmt*>(sw->get_case(0));
    EXPECT_NE(c1, nullptr);

    auto c1b = dynamic_cast<const ReturnStmt*>(c1->get_body());
    EXPECT_NE(c1b, nullptr);

    auto c2 = dynamic_cast<const CaseStmt*>(sw->get_case(1));
    EXPECT_NE(c1, nullptr);

    auto c2b = dynamic_cast<const ReturnStmt*>(c2->get_body());
    EXPECT_NE(c1b, nullptr);

    auto def = dynamic_cast<const ReturnStmt*>(sw->get_default());
    EXPECT_NE(def, nullptr);
}

TEST_F(ParserTests, ParseTypedefDecl) {
    Parser parser("test", "typedef unsigned long long uint64_t;");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 1);

    auto td = dynamic_cast<const TypedefDecl*>(unit->get_decl("uint64_t"));
    EXPECT_NE(td, nullptr);
    EXPECT_EQ(td->get_name(), "uint64_t");
    EXPECT_EQ(td->get_type()->to_string(), "uint64_t");

    const QualType& ty = td->get_type();
    
    const TypedefType* tdt = dynamic_cast<const TypedefType*>(ty.get_type());
    EXPECT_NE(tdt, nullptr);

    const QualType& underlying = tdt->get_underlying();
    EXPECT_EQ(underlying.to_string(), "unsigned long long");
}

TEST_F(ParserTests, ParseTypedefDeclRef) {
    Parser parser("test", "typedef unsigned long long uint64_t; const uint64_t main();");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 2);

    auto fn = dynamic_cast<const FunctionDecl*>(unit->get_decl("main"));
    EXPECT_NE(fn, nullptr);
    EXPECT_EQ(fn->get_type().to_string(), "const uint64_t ()");
}

TEST_F(ParserTests, ParseStructDecl) {
    Parser parser("test", "struct Box { long long x; const int y; };");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 0);
    EXPECT_EQ(unit->num_tags(), 1);

    auto decl = dynamic_cast<const RecordDecl*>(unit->get_tag("Box"));
    EXPECT_NE(decl, nullptr);
    EXPECT_EQ(decl->get_name(), "Box");
    EXPECT_EQ(decl->num_decls(), 2);

    auto f1 = dynamic_cast<const FieldDecl*>(decl->get_field(0));
    EXPECT_NE(f1, nullptr);
    EXPECT_EQ(f1->get_name(), "x");
    EXPECT_EQ(f1->get_type().to_string(), "long long");

    auto f2 = dynamic_cast<const FieldDecl*>(decl->get_field(1));
    EXPECT_NE(f2, nullptr);
    EXPECT_EQ(f2->get_name(), "y");
    EXPECT_EQ(f2->get_type().to_string(), "const int");
}

TEST_F(ParserTests, ParseEnumDecl) {
    Parser parser("test", "enum Colors { Orange, Yellow = 5, Red, Blue = -12, Green };");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 0);
    EXPECT_EQ(unit->num_tags(), 1);

    auto decl = dynamic_cast<const EnumDecl*>(unit->get_tag("Colors"));
    EXPECT_NE(decl, nullptr);
    EXPECT_EQ(decl->get_name(), "Colors");
    EXPECT_EQ(decl->num_variants(), 5);

    auto v1 = dynamic_cast<const EnumVariantDecl*>(decl->get_variant(0));
    EXPECT_NE(v1, nullptr);
    EXPECT_EQ(v1->get_name(), "Orange");
    EXPECT_EQ(v1->get_type().to_string(), "enum Colors");
    EXPECT_EQ(v1->get_value(), 0);

    auto v2 = dynamic_cast<const EnumVariantDecl*>(decl->get_variant(1));
    EXPECT_NE(v2, nullptr);
    EXPECT_EQ(v2->get_name(), "Yellow");
    EXPECT_EQ(v2->get_type().to_string(), "enum Colors");
    EXPECT_EQ(v2->get_value(), 5);

    auto v3 = dynamic_cast<const EnumVariantDecl*>(decl->get_variant(2));
    EXPECT_NE(v3, nullptr);
    EXPECT_EQ(v3->get_name(), "Red");
    EXPECT_EQ(v3->get_type().to_string(), "enum Colors");
    EXPECT_EQ(v3->get_value(), 6);

    auto v4 = dynamic_cast<const EnumVariantDecl*>(decl->get_variant(3));
    EXPECT_NE(v4, nullptr);
    EXPECT_EQ(v4->get_name(), "Blue");
    EXPECT_EQ(v4->get_type().to_string(), "enum Colors");
    EXPECT_EQ(v4->get_value(), -12);

    auto v5 = dynamic_cast<const EnumVariantDecl*>(decl->get_variant(4));
    EXPECT_NE(v5, nullptr);
    EXPECT_EQ(v5->get_name(), "Green");
    EXPECT_EQ(v5->get_type().to_string(), "enum Colors");
    EXPECT_EQ(v5->get_value(), -11);
}

TEST_F(ParserTests, ParseUnnamedEnumDecl) {
    Parser parser("test", "enum { Orange, Yellow = 5, Red, Blue = -12, Green };");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 0);
    EXPECT_EQ(unit->num_tags(), 1);

    auto decl = dynamic_cast<const EnumDecl*>(unit->get_tags().front());
    EXPECT_NE(decl, nullptr);
    EXPECT_EQ(decl->num_variants(), 5);

    auto v1 = dynamic_cast<const EnumVariantDecl*>(decl->get_variant(0));
    EXPECT_NE(v1, nullptr);
    EXPECT_EQ(v1->get_name(), "Orange");
    EXPECT_EQ(v1->get_type().to_string(), "int");
    EXPECT_EQ(v1->get_value(), 0);

    auto v2 = dynamic_cast<const EnumVariantDecl*>(decl->get_variant(1));
    EXPECT_NE(v2, nullptr);
    EXPECT_EQ(v2->get_name(), "Yellow");
    EXPECT_EQ(v2->get_type().to_string(), "int");
    EXPECT_EQ(v2->get_value(), 5);

    auto v3 = dynamic_cast<const EnumVariantDecl*>(decl->get_variant(2));
    EXPECT_NE(v3, nullptr);
    EXPECT_EQ(v3->get_name(), "Red");
    EXPECT_EQ(v3->get_type().to_string(), "int");
    EXPECT_EQ(v3->get_value(), 6);

    auto v4 = dynamic_cast<const EnumVariantDecl*>(decl->get_variant(3));
    EXPECT_NE(v4, nullptr);
    EXPECT_EQ(v4->get_name(), "Blue");
    EXPECT_EQ(v4->get_type().to_string(), "int");
    EXPECT_EQ(v4->get_value(), -12);

    auto v5 = dynamic_cast<const EnumVariantDecl*>(decl->get_variant(4));
    EXPECT_NE(v5, nullptr);
    EXPECT_EQ(v5->get_name(), "Green");
    EXPECT_EQ(v5->get_type().to_string(), "int");
    EXPECT_EQ(v5->get_value(), -11);
}

/*
TEST_F(ParserTests, ParseMemberBasic) {
    Parser parser("test", "struct A { int a; }; int main() { struct A x; x.a; }");
    EXPECT_NO_FATAL_FAILURE(unit = parser.parse());

    EXPECT_EQ(unit->num_decls(), 2);

    auto fn = dynamic_cast<const FunctionDecl*>(unit->get_decl("main"));
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

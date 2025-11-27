//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include <cstdint>

int32_t main(int32_t argc, char** argv) {
    return 1;
}

//
// Lexical Analysis
// Syntax Analysis
// Symbol Analysis
// Semantic Analysis
// Code Generation
//

//
// Type
// - BuiltinType (kind)
// - ArrayType (element, size)
// - PointerType (pointee)
// - FunctionType (ret, args)
// - AliasType (decl, underlying)
// - StructType (decl)
// - EnumType (decl)

//
// Decl
// - NamedDecl (name)
// - TranslationUnitDecl (scope)
//
// NamedDecl
// - TypeDecl (type)
// - ValueDecl (type)
//
// TypeDecl
// - AliasDecl (type)
// - StructDecl (fields)
// - EnumDecl (variants)
//
// ValueDecl
// - VariableDecl (?init)
// - ParameterDecl
// - FunctionDecl (params, ?body)
// - EnumVariantDecl (value)
//

//
// Stmt
// - ValueStmt (type)
// - BlockStmt (stmts)
// - IfStmt (cond, then, ?else)
// - WhileStmt (cond, ?body)
// - UntilStmt (cond, ?body)
// - LoopStmt (?init, ?stop, ?step, ?body)
// - MatchStmt (expr, cases, ?default)
// - CaseStmt (expr, body)
//
// ValueStmt
// - Expr
//
// Expr
// - IntegerLiteral (value)
// - FPLiteral (value)
// - CharLiteral (value)
// - StringLiteral (value)
// - NullLiteral
// - BinaryExpr (op, lhs, rhs)
// - UnaryExpr (op, expr)
// - ParenExpr (expr)
// - CastExpr (expr)
// - SizeofExpr (type)
// - DeclRefExpr (decl)
// - MemberExpr (base, decl)
// - SubscriptExpr (base, index)
// - CallExpr (callee, args)

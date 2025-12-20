//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/tree/Decl.hpp"
#include "stmc/tree/Expr.hpp"
#include "stmc/tree/Type.hpp"

#include <cassert>

using namespace stm;

BoolLiteral* BoolLiteral::create(Context &ctx, SourceSpan span, bool value) {
    return new BoolLiteral(
        span, BuiltinType::get(ctx, BuiltinType::Bool), value);
}

IntegerLiteral* IntegerLiteral::create(
        Context &ctx, SourceSpan span, const TypeUse &type, int64_t value) {
    return new IntegerLiteral(span, type, value);
}

FPLiteral* FPLiteral::create(
        Context &ctx, SourceSpan span, const TypeUse &type, double value) {
    return new FPLiteral(span, type, value);
}

CharLiteral* CharLiteral::create(Context &ctx, SourceSpan span, char value) {
    return new CharLiteral(
        span, BuiltinType::get(ctx, BuiltinType::Char), value);
}

StringLiteral* StringLiteral::create(
        Context &ctx, SourceSpan span, const string &value) {
    return new StringLiteral(
        span, 
        PointerType::get(ctx, BuiltinType::get(ctx, BuiltinType::Char)),
        value);
}

NullLiteral* NullLiteral::create(
        Context &ctx, SourceSpan span, const TypeUse &type) {
    return new NullLiteral(span, type);
}

BinaryOp::~BinaryOp() {
    delete m_lhs;
    delete m_rhs;

    m_lhs = nullptr;
    m_rhs = nullptr;
}

bool BinaryOp::is_assignment(Operator op) {
    switch (op) {
    case Assign:
    case AddAssign:
    case SubAssign:
    case MulAssign:
    case DivAssign:
    case ModAssign:
    case AndAssign:
    case OrAssign:
    case XorAssign:
    case LeftShiftAssign:
    case RightShiftAssign:
        return true;
    default:
        return false;
    }
}

BinaryOp* BinaryOp::create(
        Context &ctx, SourceSpan span, Operator oper, Expr *lhs, Expr *rhs) {
    assert(oper != Unknown && "invalid binary operand!");
    assert(lhs && "invalid binary lhs expression!");
    assert(rhs && "invalid binary rhs expression!");

    return new BinaryOp(span, lhs->get_type(), oper, lhs, rhs);
}

UnaryOp::~UnaryOp() {
    delete m_expr;
    m_expr = nullptr;
}

UnaryOp* UnaryOp::create(
        Context &ctx, SourceSpan span, Operator oper, bool prefix, 
        Expr *expr) {
    assert(oper != Unknown && "invalid unary operand!");
    assert(expr && "invalid unary expression!");

    return new UnaryOp(span, expr->get_type(), oper, prefix, expr);
}

CastExpr::~CastExpr() {
    delete m_expr;
    m_expr = nullptr;
}

CastExpr* CastExpr::create(
        Context &ctx, SourceSpan span, const TypeUse &type, Expr *expr) {
    assert(expr && "invalid cast expression!");

    return new CastExpr(span, type, expr);
}

ParenExpr::~ParenExpr() {
    delete m_expr;
    m_expr = nullptr;
}

ParenExpr* ParenExpr::create(Context &context, SourceSpan span, Expr *expr) {
    assert(expr && "invalid paren expression!");

    return new ParenExpr(span, expr->get_type(), expr);
}

SizeofExpr* SizeofExpr::create(
        Context &ctx, SourceSpan span, const TypeUse &target) {
    return new SizeofExpr(
        span, BuiltinType::get(ctx, BuiltinType::UInt64), target);
}

AccessExpr::~AccessExpr() {
    delete m_base;
    m_base = nullptr;
}

AccessExpr* AccessExpr::create(
        Context &ctx, SourceSpan span, Expr *base, const string& name) {
    assert(base && "invalid access base expression!");
    
    return new AccessExpr(
        span, 
        {},
        base, 
        name, 
        nullptr);
}

SubscriptExpr::~SubscriptExpr() {
    delete m_base;
    delete m_index;

    m_base = nullptr;
    m_index = nullptr;
}

SubscriptExpr* SubscriptExpr::create(
        Context &ctx, SourceSpan span, Expr *base, Expr *index) {
    assert(base && "invalid subscript base expression!");
    assert(index && "invalid subscript index expression!");

    return new SubscriptExpr(span, base->get_type(), base, index);
}

DeclRefExpr* DeclRefExpr::create(
        Context &ctx, SourceSpan span, const string &name, 
        const ValueDecl *decl) {
    return new DeclRefExpr(
        span, decl ? decl->get_type() : nullptr, name, decl);
}

CallExpr::~CallExpr() {
    delete m_callee;
    m_callee = nullptr;
    
    for (Expr* arg : m_args)
        delete arg;

    m_args.clear();
}

CallExpr* CallExpr::create(
        Context &ctx, SourceSpan span, Expr *callee, 
        const vector<Expr*> &args) {
    assert(callee && "invalid callee expression!");

    return new CallExpr(
        span, callee ? callee->get_type() : nullptr, callee, args);
}

//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/tree/Context.hpp"
#include "stmc/tree/Decl.hpp"
#include "stmc/tree/Expr.hpp"
#include "stmc/tree/Scope.hpp"
#include "stmc/tree/Stmt.hpp"

using namespace stm;

AsmStmt::~AsmStmt() {
    for (Expr* arg : m_args)
        delete arg;

    m_ins.clear();
    m_outs.clear();
    m_args.clear();
    m_clobbers.clear();
}

AsmStmt* AsmStmt::create(
        Context &ctx, SourceSpan span, const string& iasm, 
        const vector<string> &outs, const vector<string> &ins, 
        const vector<Expr*> &args, const vector<string> &clobbers) {
    return new AsmStmt(span, iasm, outs, ins, args, clobbers);
}

BlockStmt::~BlockStmt() {
    delete m_scope;
    m_scope = nullptr;

    for (Stmt* stmt : m_stmts)
        delete stmt;

    m_stmts.clear();
}

BlockStmt* BlockStmt::create(
        Context &ctx, SourceSpan span, Scope *scope, 
        const vector<Stmt*> &stmts) {
    return new BlockStmt(span, scope, stmts);
}

DeclStmt::~DeclStmt() {
    delete m_decl;
    m_decl = nullptr;
}

DeclStmt* DeclStmt::create(Context &ctx, Decl *decl) {
    return new DeclStmt(decl->get_span(), decl);
}

RetStmt::~RetStmt() {
    if (has_expr()) {
        delete m_expr;
        m_expr = nullptr;
    }
}

RetStmt* RetStmt::create(Context &ctx, SourceSpan span, Expr *expr) {
    return new RetStmt(span, expr);
}

IfStmt::~IfStmt() {
    delete m_cond;
    m_cond = nullptr;

    delete m_then;
    m_then = nullptr;

    if (has_else()) {
        delete m_else;
        m_else = nullptr;
    }
}

IfStmt* IfStmt::create(
        Context &ctx, SourceSpan span, Expr *cond, Stmt *then, Stmt *els) {
    return new IfStmt(span, cond, then, els);
}

WhileStmt::~WhileStmt() {
    delete m_cond;
    m_cond = nullptr;

    if (has_body()) {
        delete m_body;
        m_body = nullptr;
    }
}

WhileStmt* WhileStmt::create(
        Context &ctx, SourceSpan span, Expr *cond, Stmt *body) {
    return new WhileStmt(span, cond, body);
}

BreakStmt* BreakStmt::create(Context &ctx, SourceSpan span) {
    return new BreakStmt(span);
}

ContinueStmt* ContinueStmt::create(Context &ctx, SourceSpan span) {
    return new ContinueStmt(span);
}

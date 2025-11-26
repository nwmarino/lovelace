//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "scc/ast/Decl.hpp"
#include "scc/ast/Expr.hpp"
#include "scc/ast/Stmt.hpp"

using namespace scc;

CompoundStmt::~CompoundStmt() {
    for (auto stmt : m_stmts)
        delete stmt;

    m_stmts.clear();
}

ExprStmt::~ExprStmt() {
    delete m_expr;
    m_expr = nullptr;
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

ReturnStmt::~ReturnStmt() {
    if (has_expr()) {
        delete m_expr;
        m_expr = nullptr;
    }
}

WhileStmt::~WhileStmt() {
    delete m_cond;
    m_cond = nullptr;

    if (has_body()) {
        delete m_body;
        m_body = nullptr;
    }
}

ForStmt::~ForStmt() {
    if (has_init()) {
        delete m_init;
        m_init = nullptr;
    }

    if (has_cond()) {
        delete m_cond;
        m_cond = nullptr;
    }

    if (has_step()) {
        delete m_step;
        m_step = nullptr;
    }

    if (has_body()) {
        delete m_body;
        m_body = nullptr;
    }
}

CaseStmt::~CaseStmt() {
    delete m_match;
    m_match = nullptr;

    if (has_body()) {
        delete m_body;
        m_body = nullptr;
    }
}

SwitchStmt::~SwitchStmt() {
    delete m_match;
    m_match = nullptr;

    for (auto c : m_cases)
        delete c;

    m_cases.clear();

    if (has_default()) {
        delete m_default;
        m_default = nullptr;
    }
}

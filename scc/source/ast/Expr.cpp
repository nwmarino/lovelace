//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "scc/ast/Expr.hpp"

using namespace scc;

BinaryExpr::~BinaryExpr() {
    delete m_left;
    delete m_right;
    m_left = m_right = nullptr;
}

bool BinaryExpr::is_assignment(Op op) {
    switch (op) {
    case Assign:
    case AddAssign:
    case SubAssign:
    case MulAssign:
    case DivAssign:
    case ModAssign:
    case AndAssign:
    case OrAssign:
    case LeftShiftAssign:
    case RightShiftAssign:
        return true;
    default:
        return false;
    }
}

bool BinaryExpr::is_comparison(Op op) {
    switch (op) {
    case Equals:
    case NotEquals:
    case LessThan:
    case LessThanEquals:
    case GreaterThan:
    case GreaterThanEquals:
    case LogicAnd:
    case LogicOr:
        return true;
    default:
        return false;
    }
}

const char* BinaryExpr::to_string(Op op) {
    switch (op) {
    case Unknown:           return "";
    case Assign:            return "=";
    case Add:               return "+";
    case AddAssign:         return "+=";
    case Sub:               return "-";
    case SubAssign:         return "-=";
    case Mul:               return "*";
    case MulAssign:         return "*=";
    case Div:               return "/";
    case DivAssign:         return "/=";
    case Mod:               return "%";
    case ModAssign:         return "%=";
    case And:               return "&";
    case AndAssign:         return "&=";
    case Or:                return "|";
    case OrAssign:          return "|=";
    case Xor:               return "^";
    case XorAssign:         return "^=";
    case LeftShift:         return "<<";
    case LeftShiftAssign:   return "<<=";
    case RightShift:        return ">>";
    case RightShiftAssign:  return ">>=";
    case Equals:            return "==";
    case NotEquals:         return "!=";
    case LessThan:          return "<";
    case LessThanEquals:    return "<=";
    case GreaterThan:       return ">";
    case GreaterThanEquals: return ">=";
    case LogicAnd:          return "&&";
    case LogicOr:           return "||";
    }
}

UnaryExpr::~UnaryExpr() {
    delete m_expr;
    m_expr = nullptr;
}

const char* UnaryExpr::to_string(Op op) {
    switch (op) {
    case Unknown:       return "";
    case Not:           return "~";
    case LogicNot:      return "!";
    case Negate:        return "-";
    case AddressOf:     return "&";
    case Dereference:   return "*";
    case Increment:     return "++";
    case Decrement:     return "--";
    }
}

ParenExpr::~ParenExpr() {
    delete m_expr;
    m_expr = nullptr;
}

CallExpr::~CallExpr() {
    delete m_callee;
    m_callee = nullptr;

    for (auto arg : m_args)
        delete arg;

    m_args.clear();
}

CastExpr::~CastExpr() {
    delete m_expr;
    m_expr = nullptr;
}

SubscriptExpr::~SubscriptExpr() {
    delete m_base;
    delete m_index;
    m_base = m_index = nullptr;
}

MemberExpr::~MemberExpr() {
    delete m_base;
    m_base = nullptr;
}

TernaryExpr::~TernaryExpr() {
    delete m_cond;
    delete m_tval;
    delete m_fval;
    m_cond = m_tval = m_fval = nullptr;
}

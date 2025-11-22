//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "ast/Expr.hpp"

using namespace scc;

std::string BinaryExpr::to_string(Op op) {
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

std::string UnaryExpr::to_string(Op op) {
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

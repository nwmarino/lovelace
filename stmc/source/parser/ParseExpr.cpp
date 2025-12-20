//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/parser/Parser.hpp"
#include "stmc/tree/Expr.hpp"
#include "stmc/tree/Type.hpp"
#include "stmc/types/SourceLocation.hpp"

#include <cassert>
#include <string>

using namespace stm;

BinaryOp::Operator Parser::get_binary_oper(Token::Kind kind) const {
    switch (kind) {
    case Token::Eq:             return BinaryOp::Assign;
    case Token::EqEq:           return BinaryOp::Equals;
    case Token::BangEq:         return BinaryOp::NotEquals;
    case Token::Plus:           return BinaryOp::Add;
    case Token::PlusEq:         return BinaryOp::AddAssign;
    case Token::Minus:          return BinaryOp::Sub;
    case Token::MinusEq:        return BinaryOp::SubAssign;
    case Token::Star:           return BinaryOp::Mul;
    case Token::StarEq:         return BinaryOp::MulAssign;
    case Token::Slash:          return BinaryOp::Div;
    case Token::SlashEq:        return BinaryOp::DivAssign;
    case Token::Percent:        return BinaryOp::Mod;
    case Token::PercentEq:      return BinaryOp::ModAssign;
    case Token::Left:           return BinaryOp::LessThan;
    case Token::LeftLeft:       return BinaryOp::LeftShift;
    case Token::LeftEq:         return BinaryOp::LessThanEquals;
    case Token::LeftLeftEq:     return BinaryOp::LeftShiftAssign;
    case Token::Right:          return BinaryOp::GreaterThan;
    case Token::RightRight:     return BinaryOp::RightShift;
    case Token::RightEq:        return BinaryOp::GreaterThanEquals;
    case Token::RightRightEq:   return BinaryOp::RightShiftAssign;
    case Token::And:            return BinaryOp::And;
    case Token::AndAnd:         return BinaryOp::LogicAnd;
    case Token::AndEq:          return BinaryOp::AndAssign;
    case Token::Or:             return BinaryOp::Or;
    case Token::OrOr:           return BinaryOp::LogicOr;
    case Token::OrEq:           return BinaryOp::OrAssign;
    case Token::Xor:            return BinaryOp::Xor;
    case Token::XorEq:          return BinaryOp::XorAssign;
    default:                    return BinaryOp::Unknown;
    }
}

int32_t Parser::get_binary_oper_precedence(Token::Kind kind) const {
    switch (get_binary_oper(kind)) {
    case BinaryOp::Mul:
    case BinaryOp::Div:
    case BinaryOp::Mod:
        return 11;
    case BinaryOp::Add:
    case BinaryOp::Sub:
        return 10;
    case BinaryOp::LeftShift:
    case BinaryOp::RightShift:
        return 9;
    case BinaryOp::LessThan:
    case BinaryOp::LessThanEquals:
    case BinaryOp::GreaterThan:
    case BinaryOp::GreaterThanEquals:
        return 8;
    case BinaryOp::Equals:
    case BinaryOp::NotEquals:
        return 7;
    case BinaryOp::And:
        return 6;
    case BinaryOp::Or:
        return 5;
    case BinaryOp::Xor:
        return 4;
    case BinaryOp::LogicAnd:
        return 3;
    case BinaryOp::LogicOr:
        return 2;
    case BinaryOp::Assign:
    case BinaryOp::AddAssign:
    case BinaryOp::SubAssign:
    case BinaryOp::MulAssign:
    case BinaryOp::DivAssign:
    case BinaryOp::ModAssign:
    case BinaryOp::AndAssign:
    case BinaryOp::OrAssign:
    case BinaryOp::XorAssign:
    case BinaryOp::LeftShiftAssign:
    case BinaryOp::RightShiftAssign:
        return 1;
    case BinaryOp::Unknown:
        return -1;
    }
}

UnaryOp::Operator Parser::get_unary_oper(Token::Kind kind) const {
    switch (kind) {
    case Token::Bang:       return UnaryOp::LogicNot;
    case Token::PlusPlus:   return UnaryOp::Increment;
    case Token::Minus:      return UnaryOp::Negate;
    case Token::MinusMinus: return UnaryOp::Decrement;
    case Token::Star:       return UnaryOp::Dereference;
    case Token::And:        return UnaryOp::AddressOf;
    case Token::Tilde:      return UnaryOp::Not;
    default:                return UnaryOp::Unknown;
    }
}

Expr* Parser::parse_initial_expression() {
    Expr* base = parse_prefix_unary_operator();
    assert(base && "unable to parse base expression!");

    return parse_binary_operator(base, 0);
}

Expr* Parser::parse_primary_expression() {
    if (match(Token::Identifier)) {
        return parse_identifier_expression();
    } else if (match(Token::SetParen)) {
        return parse_parentheses();
    } else if (match(Token::Integer)) {
        return parse_integer_literal();
    } else if (match(Token::Float)) {
        return parse_floating_point_literal();
    } else if (match(Token::Character)) {
        return parse_character_literal();
    } else if (match(Token::String)) {
        return parse_string_literal();
    }

    return nullptr;
}

Expr* Parser::parse_identifier_expression() {
    if (match("cast")) {
        return parse_type_cast();
    } else if (match("null")) {
        return parse_null_pointer_literal();
    } else if (match("true") || match("false")) {
        return parse_boolean_literal();
    } else if (match("sizeof")) {
        return parse_sizeof_operator();
    } else {
        return parse_named_reference();
    }
}

Expr* Parser::parse_prefix_unary_operator() {
    UnaryOp::Operator op = get_unary_oper(last().kind);
    if (UnaryOp::is_prefix(op)) {
        SourceLocation start = loc();
        next();

        Expr* base = parse_prefix_unary_operator();
        assert(base && "unable to parse unary base expression!");

        return UnaryOp::create(*m_context, since(start), op, true, base);
    } else {
        return parse_postfix_unary_operator();
    }
}

Expr* Parser::parse_postfix_unary_operator() {
    Expr* expr = parse_primary_expression();
    assert(expr && "unable to parse postfix operator base expression!");

    do {
        SourceLocation start = loc();
        UnaryOp::Operator op = get_unary_oper(last().kind);

        if (UnaryOp::is_postfix(op)) {
            // Ordinary postfix operator -> UnaryOp recurse.
            next();
            expr = UnaryOp::create(*m_context, since(start), op, false, expr);
        } else if (match(Token::SetBrack)) {
            // '[]' operator -> SubscriptExpr.
            next(); // '['

            Expr* index = parse_initial_expression();
            assert(index && "unable to parse subscript index expression!");

            if (!expect(Token::EndBrack))
                m_diags.fatal("expected ']'", loc());

            expr = SubscriptExpr::create(*m_context, since(start), expr, index);
        } else if (match(Token::SetParen)) {
            // '(...' operator -> CallExpr.
            next(); // '('

            vector<Expr*> args = {};
            args.reserve(2);

            while (!expect(Token::EndParen)) {
                Expr* arg = parse_initial_expression();
                assert(arg && "unable to parse call argument expression!");

                args.push_back(arg);

                if (expect(Token::EndParen))
                    break;

                if (!expect(Token::Comma))
                    m_diags.fatal("expected ','", loc());
            }

            args.shrink_to_fit();

            expr = CallExpr::create(*m_context, since(start), expr, args);
        } else if (match(Token::Dot)) {
            // '.' operator -> AccessExpr.
            next(); // '.'

            if (!match(Token::Identifier))
                m_diags.fatal("expected identifier", loc());
        
            string field = last().value;
            next();

            expr = AccessExpr::create(*m_context, since(start), expr, field);
        } else {
            break;
        }
    } while (true);

    return expr;
}

Expr* Parser::parse_binary_operator(Expr* base, int32_t precedence) {
    do {
        int32_t curr = get_binary_oper_precedence(last().kind);
        if (curr < precedence)
            break;

        BinaryOp::Operator op = get_binary_oper(last().kind);
        if (op == BinaryOp::Unknown)
            break;

        next();
        Expr* right = parse_prefix_unary_operator();
        assert(right && "unable to parse rhs expression!");

        int32_t next = get_binary_oper_precedence(last().kind);
        if (curr < next) {
            right = parse_binary_operator(right, precedence + 1);
            assert(right && "unable to parse nested rhs expression!");
        }

        base = BinaryOp::create(
            *m_context, since(base->get_span().start), op, base, right);
    } while (true);

    return base;
}

Expr* Parser::parse_boolean_literal() {
    const Token lit = last();
    next();

    return BoolLiteral::create(*m_context, lit.loc, lit.value == "true");
}

Expr* Parser::parse_integer_literal() {
    const Token lit = last();
    next();

    // @Todo: support suffix type inference.

    return IntegerLiteral::create(
        *m_context, 
        lit.loc, 
        BuiltinType::get(*m_context, BuiltinType::Int64), 
        std::stoll(lit.value));
}

Expr* Parser::parse_floating_point_literal() {
    const Token lit = last();
    next();

    // @Todo: support suffix type inference.

    return FPLiteral::create(
        *m_context, 
        lit.loc, 
        BuiltinType::get(*m_context, BuiltinType::Float64), 
        std::stod(lit.value));
}

Expr* Parser::parse_character_literal() {
    const Token lit = last();
    next();

    return CharLiteral::create(*m_context, lit.loc, lit.value[0]);
}

Expr* Parser::parse_string_literal() {
    const Token lit = last();
    next();

    return StringLiteral::create(*m_context, lit.loc, lit.value);
}

Expr* Parser::parse_null_pointer_literal() {
    const Token lit = last();
    next();

    const Type* p_void = PointerType::get(
        *m_context, BuiltinType::get(*m_context, BuiltinType::Void));

    return NullLiteral::create(*m_context, lit.loc, p_void);
}

Expr* Parser::parse_type_cast() {
    SourceLocation start = loc();
    next(); // 'cast'


    if (!expect(Token::Left))
        m_diags.fatal("expected '<'", loc());

    TypeUse type;
    parse_type_specifier(type);

    if (!expect(Token::Right))
        m_diags.fatal("expected '>'", loc());

    if (!expect(Token::SetParen))
        m_diags.fatal("expected '('", loc());

    Expr* expr = parse_initial_expression();
    assert(expr && "unable to parse cast expression!");

    if (!match(Token::EndParen))
        m_diags.fatal("expected ')'", loc());

    SourceLocation end = loc();
    next(); // ')'

    return CastExpr::create(*m_context, SourceSpan(start, end), type, expr);
}

Expr* Parser::parse_parentheses() {
    SourceLocation start = loc();
    next(); // '('
    
    Expr* expr = parse_initial_expression();
    assert(expr && "unable to parse initial expression!");

    if (!match(Token::EndParen))
        m_diags.fatal("expected ')'", loc());

    SourceLocation end = loc();
    next(); // ')'

    return ParenExpr::create(*m_context, SourceSpan(start, end), expr);
}

Expr* Parser::parse_sizeof_operator() {
    SourceLocation start = loc();
    next(); // 'sizeof'

    if (!expect(Token::SetParen))
        m_diags.fatal("expected '('", loc());

    TypeUse type;
    parse_type_specifier(type);

    if (!match(Token::EndParen))
        m_diags.fatal("expected ')'", loc());

    SourceLocation end = loc();
    next(); // ')'

    return SizeofExpr::create(*m_context, SourceSpan(start, end), type);
}

Expr* Parser::parse_named_reference() {
    const Token ident = last();
    next();

    return DeclRefExpr::create(*m_context, since(ident.loc), ident.value, nullptr);
}

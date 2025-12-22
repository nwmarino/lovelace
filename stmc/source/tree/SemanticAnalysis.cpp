//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/tree/SemanticAnalysis.hpp"
#include "stmc/tree/Decl.hpp"
#include "stmc/tree/Expr.hpp"
#include "stmc/tree/Stmt.hpp"
#include "stmc/tree/Type.hpp"
#include "stmc/types/SourceSpan.hpp"

#include <cassert>

using namespace stm;

/// Test if \p type can be evaluated to a boolean (either trivially or through 
/// a comparison).
static inline bool is_boolean_evaluable(const Type* type) {
    return type->is_integer() || type->is_floating_point() || type->is_pointer();
}

SemanticAnalysis::TypeCheckResult SemanticAnalysis::type_check(
        const TypeUse& actual, const TypeUse& expected, 
        SemanticAnalysis::TypeCheckMode mode) const {
    if (actual.compare(expected))
        return Match;

    switch (mode) {
    case Explicit:
        return Mismatch;

    case AllowImplicit:
        if (actual.can_cast(expected, true))
            return Cast;

        return Mismatch;

    case Loose:
        if (actual.can_cast(expected, true))
            return Cast;

        if ((actual->is_integer() && expected->is_pointer()) 
          || (actual->is_pointer() && expected->is_integer()))
            return Match;

        return Mismatch;
    }
}

SemanticAnalysis::SemanticAnalysis(Diagnostics& diags, Options& options)
    : m_diags(diags), m_options(options) {}

void SemanticAnalysis::visit(TranslationUnitDecl& node) {
    m_diags.set_path(node.get_file());
    m_context = &node.get_context();

    for (uint32_t i = 0, e = node.num_decls(); i < e; ++i)
        node.get_decl(i)->accept(*this);
}

void SemanticAnalysis::visit(VariableDecl& node) {
    if (node.has_init()) {
        Expr* init = node.get_init();;
        init->accept(*this);

        const SourceSpan span = node.get_span();
        if (node.is_global() && !init->is_constant())
            m_diags.fatal("globals cannot be initialized with non-constants", span);

        init->get_type().as_mut();

        const TypeUse& actual = init->get_type();
        const TypeUse& expected = node.get_type();

        TypeCheckResult TC = type_check(actual, expected);
        if (TC == TypeCheckResult::Mismatch) {
            m_diags.fatal("initializer type mismatch; got '" + 
                actual.to_string() + "'", span);
        } else if (TC == TypeCheckResult::Cast) {
            node.m_init = CastExpr::create(
                *m_context, init->get_span(), expected, init);
        }
    }
}

void SemanticAnalysis::visit(FunctionDecl& node) {
    m_function = &node;

    if (node.is_main()) {
        const TypeUse& ret_type = node.get_return_type();
        if (!ret_type.compare(BuiltinType::get(*m_context, BuiltinType::Int64)))
            m_diags.fatal("'main' must return 's64'", node.get_span());
    }
    
    if (node.has_body())
        node.get_body()->accept(*this);

    m_function = nullptr;
}

void SemanticAnalysis::visit(AsmStmt& node) {
    const SourceSpan span = node.get_span();

    for (uint32_t i = 0, e = node.num_args(); i < e; ++i) {
        Expr* arg = node.get_arg(i);
        arg->accept(*this);

        if (i < node.num_output_constraints()) {
            if (!arg->get_type().is_mut())
                m_diags.fatal("immutable value cannot be used as 'asm' output", span);
        }
    }

    uint32_t arg_refs = 0;
    for (char c : node.get_assembly_string())
        if (c == '#')
            ++arg_refs;

    if (arg_refs > node.num_args())
        m_diags.fatal("'asm' references more arguments than provided", span);

    // Output constraints can be:
    //
    // (|r) write to register
    // (|m) write to memory
    // (&r) read-write to/from register
    // (&m) read-write to/from memory
    auto is_valid_output_constraint = [](const string& constraint) {
        return constraint == "|r" || constraint == "|m" 
            || constraint == "&r" || constraint == "&m";
    };

    // Input constraints can be:
    //
    // (r) read from register
    // (m) read from memory
    auto is_valid_input_constraint = [](const string& constraint) {
        return constraint == "r" || constraint == "m" || constraint == "...";
    };

    for (uint32_t i = 0, e = node.num_output_constraints(); i < e; ++i) {
        const string& constraint = node.get_output_constraint(i);
        if (!is_valid_output_constraint(constraint))
            m_diags.fatal("invalid output constraint: '" + constraint + "'", span);
    }

    for (uint32_t i = 0, e = node.num_input_constraints(); i < e; ++i) {
        const string& constraint = node.get_input_constraint(i);
        if (!is_valid_input_constraint(constraint))
            m_diags.fatal("invalid input constraint: '" + constraint + "'", span);
    }
}

void SemanticAnalysis::visit(BlockStmt& node) {
    for (uint32_t i = 0, e = node.num_stmts(); i < e; ++i)
        node.get_stmt(i)->accept(*this);
}

void SemanticAnalysis::visit(DeclStmt& node) {
    node.get_decl()->accept(*this);
}

void SemanticAnalysis::visit(RetStmt& node) {
    const SourceSpan span = node.get_span();
    if (!m_function)
        m_diags.fatal("'ret' outside of function", span);
 
    if (!node.has_expr()) {
        if (!m_function->returns_void())
            m_diags.fatal("function does not return 'void'", span);

        return;
    }

    Expr* expr = node.get_expr();
    expr->accept(*this);

    const TypeUse& val_type = expr->get_type();
    const TypeUse& ret_type = m_function->get_return_type();
    TypeCheckResult TC = type_check(val_type, ret_type);
    if (TC == TypeCheckResult::Mismatch) {
        m_diags.fatal("return type mismatch; got '" + 
            val_type.to_string() + "'", span);
    } else if (TC == TypeCheckResult::Cast) {
        node.m_expr = CastExpr::create(
            *m_context, expr->get_span(), ret_type, expr);
    }
}

void SemanticAnalysis::visit(IfStmt& node) {
    Expr* cond = node.get_cond();
    cond->accept(*this);

    // Check that the if condition can be evaluated to a boolean.
    const SourceSpan cond_span = cond->get_span();
    if (!is_boolean_evaluable(cond->get_type()))
        m_diags.fatal("'if' condition must be a boolean", cond_span);

    Stmt* then_body = node.get_then();
    then_body->accept(*this);

    // Check that the body of an if statement is not a declaration.
    const SourceSpan then_span = then_body->get_span();
    if (dynamic_cast<DeclStmt*>(then_body))
        m_diags.fatal("'if' body cannot be declarative", then_span);

    if (node.has_else()) {
        Stmt* else_body = node.get_else();
        else_body->accept(*this);

        // Check that the body of an if-else statement is not a declaration.
        const SourceSpan else_span = else_body->get_span();
        if (dynamic_cast<DeclStmt*>(else_body))
            m_diags.fatal("'else' body cannot be declarative", else_span);
    }
}

void SemanticAnalysis::visit(WhileStmt& node) {
    Expr* cond = node.get_cond();
    const SourceSpan cond_span = cond->get_span();
    cond->accept(*this);

    // Check that the while condition can be evaluated to a boolean.
    if (!is_boolean_evaluable(cond->get_type()))
        m_diags.fatal("'while' condition must be a boolean", cond_span);

    if (node.has_body()) {
        Loop prev_loop = m_loop;
        m_loop = While;

        Stmt* body = node.get_body();
        body->accept(*this);

        // Check that the body of a while statement is not a declaration.
        const SourceSpan body_span = body->get_span();
        if (dynamic_cast<DeclStmt*>(body))
            m_diags.fatal("'while' body cannot be declarative", body_span);

        m_loop = prev_loop;
    }
}

void SemanticAnalysis::visit(BreakStmt& node) {
    // Check that continue statements are inside loop bodies.
    const SourceSpan span = node.get_span();
    if (m_loop == None)
        m_diags.fatal("'break' outside of loop", span);
}

void SemanticAnalysis::visit(ContinueStmt& node) {
    // Check that continue statements are inside loop bodies.
    const SourceSpan span = node.get_span();
    if (m_loop == None)
        m_diags.fatal("'continue' outside of loop", span);
}

void SemanticAnalysis::visit(BinaryOp& node) {
    Expr* lhs = node.get_lhs();
    Expr* rhs = node.get_rhs();

    lhs->accept(*this);
    rhs->accept(*this);

    const SourceSpan span = node.get_span();
    const TypeUse& lhs_type = lhs->get_type();
    const TypeUse& rhs_type = rhs->get_type();

    TypeCheckResult TC = type_check(rhs_type, lhs_type);
    if (TC == TypeCheckResult::Mismatch) {
        m_diags.fatal("operand type mismatch; got '" + 
            rhs_type.to_string() + "'", span);
    } else if (TC == TypeCheckResult::Cast) {
        node.m_rhs = CastExpr::create(
            *m_context, rhs->get_span(), lhs_type, rhs);
    }

    // Set the resulting type of the operator to a 'bool' if the operator is
    // a boolean comparison.
    BinaryOp::Operator op = node.get_operator();
    if (BinaryOp::is_comparison(op)) {
        node.set_type(BuiltinType::get(*m_context, BuiltinType::Bool));
        return;
    } else {
        // Default the type of the operator to the LHS type.
        node.set_type(lhs_type);
    }

    if (BinaryOp::is_assignment(op)) {
        // Check that left hand operands of assignments are lvalues.
        if (!lhs->is_lvalue())
            m_diags.fatal("left hand operand must be an lvalue", span);
    
        // Check that left hand operands of assignments are mutable.
        if (!lhs_type.is_mut())
            m_diags.fatal("left hand operand must be mutable", span);
    }
}

void SemanticAnalysis::visit(UnaryOp& node) {
    Expr* expr = node.get_expr();
    expr->accept(*this);

    const SourceSpan span = node.get_span();
    const TypeUse& type = expr->get_type();

    switch (node.get_operator()) {
    case UnaryOp::Increment: {
        // Check operator type compatibility (numerics and pointers only).
        if (!(type->is_integer() || type->is_floating_point() || type->is_pointer()))
            m_diags.fatal("'++' operator incompatible with '" + 
                type.to_string() + "'", span);
        
        if (!expr->is_lvalue())
            m_diags.fatal("'++' base must be an lvalue", span);

        if (!type.is_mut())
            m_diags.fatal("'++' base must be mutable", span);

        node.set_type(type);
        break;
    }

    case UnaryOp::Decrement: {
        // Check operator type compatibility (numerics and pointers only).
        if (!(type->is_integer() || type->is_floating_point() || type->is_pointer()))
            m_diags.fatal("'--' operator incompatible with '" + 
                type.to_string() + "'", span);
        
        if (!expr->is_lvalue())
            m_diags.fatal("'--' base must be an lvalue", span);

        if (!type.is_mut())
            m_diags.fatal("'++' base must be mutable", span);

        node.set_type(type);
        break;
    }

    case UnaryOp::Negate: {
        // Check operator type compatibility (numerics only).
        if (!(type->is_integer() || type->is_floating_point()))
            m_diags.fatal("'-' operator incompatible with '" + 
                type.to_string() + "'", span);

        node.set_type(type);
        break;
    }

    case UnaryOp::Not: {
         // Check operator type compatibility (integers only).
        if (!type->is_integer())
            m_diags.fatal("'~' operator incompatible with '" + 
                type.to_string() + "'", span);

        node.set_type(type);
        break;
    }

    case UnaryOp::LogicNot: {
        // @Todo: force scalar types arguments.
        node.set_type(type);
        break;
    }

    case UnaryOp::AddressOf: {
        if (!expr->is_lvalue())
            m_diags.fatal("'&' base must be an lvalue", span);

        node.set_type(PointerType::get(*m_context, type));
        break;
    }

    case UnaryOp::Dereference: {
        if (!type->is_pointer())
            m_diags.fatal("'*' operator incompatible with '" + 
                type.to_string() + "'", span);

        node.set_type(static_cast<const PointerType*>(
            type.get_type())->get_pointee());
        break;
    }

    case UnaryOp::Unknown:
        m_diags.fatal("unknown unary operator", span);
    }
}

void SemanticAnalysis::visit(CastExpr& node) {
    Expr* expr = node.get_expr();
    expr->accept(*this);

    // @Todo: check that the cast is valid.
}

void SemanticAnalysis::visit(ParenExpr& node) {
    Expr* expr = node.get_expr();
    expr->accept(*this);
    
    node.set_type(expr->get_type());
}

void SemanticAnalysis::visit(AccessExpr& node) {
    const FieldDecl* field = node.get_field();
    assert(field && "field access left unresolved!");

    node.set_type(field->get_type());
}

void SemanticAnalysis::visit(SubscriptExpr& node) {
    Expr* base = node.get_base();
    base->accept(*this);

    Expr* index = node.get_index();
    index->accept(*this);

    const Type* base_type = base->get_type();
    if (auto AT = dynamic_cast<const ArrayType*>(base_type)) {
        node.set_type(AT->get_element_type());
    } else if (auto PT = dynamic_cast<const PointerType*>(base_type)) {
        node.set_type(PT->get_pointee());
    } else {
        const SourceSpan span = node.get_span();
        const TypeUse& qual_type = base->get_type();

        m_diags.fatal("invalid argument type to '[]' operator: '" 
            + qual_type.to_string() + "'", span);
    }
}

void SemanticAnalysis::visit(DeclRefExpr& node) {
    const ValueDecl* decl = node.get_decl();
    assert(decl && "named reference left unresolved!");

    node.set_type(decl->get_type());
}

void SemanticAnalysis::visit(CallExpr& node) {
    Expr* callee = node.get_callee();
    callee->accept(*this);

    const SourceSpan span = node.get_span();
    const TypeUse& callee_type = callee->get_type();
    const FunctionType* FT = dynamic_cast<const FunctionType*>(
        callee_type.get_type());

    if (!FT)
        m_diags.fatal("function call target is not a function", span);

    if (node.num_args() != FT->num_params())
        m_diags.fatal("argument count mismatch, expected " + 
            std::to_string(FT->num_params()), span);

    // Pass over each argument and compare its type to the functions expected
    // parameter type.
    for (uint32_t i = 0, e = node.num_args(); i < e; ++i) {
        Expr* arg = node.get_arg(i);
        arg->accept(*this);

        const TypeUse& actual = arg->get_type();
        const TypeUse& expected = FT->get_param(i);

        TypeCheckResult TC = type_check(actual, expected);
        if (TC == TypeCheckResult::Mismatch) {
            m_diags.fatal("argument type mismatch; got '" + 
                actual.to_string() + "'", span);
        } else if (TC == TypeCheckResult::Cast) {
            node.m_args[i] = CastExpr::create(
                *m_context, arg->get_span(), expected, arg);
        }
    }
}

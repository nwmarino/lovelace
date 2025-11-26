//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "scc/ast/Decl.hpp"
#include "scc/ast/Expr.hpp"
#include "scc/ast/Sema.hpp"
#include "scc/ast/QualType.hpp"
#include "scc/ast/Stmt.hpp"
#include "scc/ast/Type.hpp"
#include "scc/core/Logger.hpp"

using namespace scc;

static bool can_cast(const QualType& from, const QualType& to);
static bool can_implicit_cast(const QualType& from, const QualType& to);

/// Different modes for type-checking. Certain type equalities i.e. ptr == int
/// are normally disallowed, but under special operations like pointer
/// arithmetic may support implicit casting.
enum TypeCheckMode : uint32_t {
    Exact,
    Loose,
    AllowImplicit,
};

/// Different results from a type check.
enum TypeCheckResult : uint32_t {
    Match,      ///< The types are loosely equivelant; based on check mode.
    Mismatch,   ///< The type mismatch and a cast cannot be used.
    Cast,       ///< If an implicit cast can be safely used.
};

static bool can_cast(const BuiltinType* from, const Type* to) {
    if (to->get_kind() == Type::Builtin)
        return from->is_void() == to->is_void();

    // Allow explicit int -> ptr casts.
    if (to->get_kind() == Type::Pointer)
        return from->is_integer();

    return false;
}

static bool can_cast(const ArrayType* from, const Type* to) {
    // Can only cast T[] -> T*.
    if (to->get_kind() != Type::Pointer)
        return false;

    return *from->get_element() == 
        *static_cast<const PointerType*>(to)->get_pointee(); 
}

static bool can_cast(const PointerType* from, const Type* to) {
    // Can only cast ptr -> int or ptr -> ptr.
    return to->is_integer() || to->get_kind() == Type::Pointer;
}

static bool can_cast(const FunctionType* from, const Type* to) {
    return false;
}

static bool can_cast(const TypedefType* from, const Type* to) {
    return can_cast(from->get_underlying(), to);
}

static bool can_cast(const RecordType* from, const Type* to) {
    return false;
}

static bool can_cast(const EnumType* from, const Type* to) {
    return to->is_integer();
}

static bool can_cast(const QualType& from, const QualType& to) {
    // Cannot cast away const: const T -> T
    if (from.is_const() && !to.is_const())
        return false;

    // Cannot cast away volatile: volatile T -> T
    if (from.is_volatile() && !to.is_volatile())
        return false;

    switch (from->get_kind()) {
    case Type::Builtin:
        return can_cast(
            static_cast<const BuiltinType*>(from.get_type()), to.get_type());
    case Type::Array:
        return can_cast(
            static_cast<const ArrayType*>(from.get_type()), to.get_type());
    case Type::Pointer:
        return can_cast(
            static_cast<const PointerType*>(from.get_type()), to.get_type());
    case Type::Function:
        return can_cast(
            static_cast<const FunctionType*>(from.get_type()), to.get_type());
    case Type::Typedef:
        return can_cast(
            static_cast<const TypedefType*>(from.get_type()), to.get_type());
    case Type::Record:
        return can_cast(
            static_cast<const RecordType*>(from.get_type()), to.get_type());
    case Type::Enum:
        return can_cast(
            static_cast<const EnumType*>(from.get_type()), to.get_type());
    }
}

static bool can_implicit_cast(const BuiltinType* from, const Type* to) {
    // Cannot implicitly cast to non-builtins.
    if (to->get_kind() != Type::Builtin)
        return false;

    // Cannot implicitly cast floating point -> integer.
    if (from->is_floating_point() && to->is_integer())
        return false;

    // All other rules apply except if one is void and the other is not.
    return from->is_void() == to->is_void();
}

static bool can_implicit_cast(const ArrayType* from, const Type* to) {
    // Can only cast T[] -> T*.
    if (to->get_kind() != Type::Pointer)
        return false;

    return *from->get_element() == 
        *static_cast<const PointerType*>(to)->get_pointee(); 
}

static bool can_implicit_cast(const PointerType* from, const Type* to) {
    // Can only implicitly cast from void* -> T* or T* -> void*.
    if (from->get_pointee()->is_void())
        return true;

    if (to->get_kind() != Type::Pointer)
        return false;

    return static_cast<const PointerType*>(to)->get_pointee()->is_void();
}

static bool can_implicit_cast(const FunctionType* from, const Type* to) {
    return false;
}

static bool can_implicit_cast(const TypedefType* from, const Type* to) {
    return can_implicit_cast(from->get_underlying(), to);
}

static bool can_implicit_cast(const RecordType* from, const Type* to) {
    return false;
}

static bool can_implicit_cast(const EnumType* from, const Type* to) {
    return to->is_integer();
}

static bool can_implicit_cast(const QualType& from, const QualType& to) {
    // Cannot cast away const: const T -> T
    if (from.is_const() && !to.is_const())
        return false;

    // Cannot cast away volatile: volatile T -> T
    if (from.is_volatile() && !to.is_volatile())
        return false;

    switch (from->get_kind()) {
    case Type::Builtin:
        return can_implicit_cast(
            static_cast<const BuiltinType*>(from.get_type()), to.get_type());
    case Type::Array:
        return can_implicit_cast(
            static_cast<const ArrayType*>(from.get_type()), to.get_type());
    case Type::Pointer:
        return can_implicit_cast(
            static_cast<const PointerType*>(from.get_type()), to.get_type());
    case Type::Function:
        return can_implicit_cast(
            static_cast<const FunctionType*>(from.get_type()), to.get_type());
    case Type::Typedef:
        return can_implicit_cast(
            static_cast<const TypedefType*>(from.get_type()), to.get_type());
    case Type::Record:
        return can_implicit_cast(
            static_cast<const RecordType*>(from.get_type()), to.get_type());
    case Type::Enum:
        return can_implicit_cast(
            static_cast<const EnumType*>(from.get_type()), to.get_type());
    }
}

/// Perform a type check between \p actual and \p expected with type checking
/// mode (or looseness) \p mode. Returns a result based on next steps to take
/// as per the result of the type check.
static TypeCheckResult type_check(
        const QualType& actual, const QualType& expected, TypeCheckMode mode) {
    if (actual == expected)
        return Match;

    switch (mode) {
    case Loose:
        if (can_implicit_cast(actual.get_type(), expected.get_type()))
            return Cast;

        // TODO: Implement logic for loose pointer arithmetic casting.
        break;

    case AllowImplicit:
        // If can implicit cast to the expected tyhpe, then use that result.
        if (can_implicit_cast(actual.get_type(), expected.get_type()))
            return Cast;

        break;

    default:
        break;
    }

    return Mismatch;
}

/// Returns true if the type \p type is boolean evaluable, that is, can be
/// interpreted as a 'true' or 'false' result.
static bool is_bool_evaluable(const QualType& type) {
    return type->is_integer() || type->is_floating_point() 
        || type->get_kind() == Type::Pointer;
}

void Sema::visit(TranslationUnitDecl &node) {
    assert(m_unit == &node && 
        "unit is not the same as the one this sema is initialized with!");

    for (TagTypeDecl* tag : node.m_tags)
        tag->accept(*this);

    for (Decl* decl : node.m_decls)
        decl->accept(*this);
}

void Sema::visit(VariableDecl &node) {
    if (node.has_init()) {
        node.get_init()->accept(*this);

        TypeCheckResult tc = type_check(
            node.get_init()->get_type(), 
            node.get_type(), 
            AllowImplicit);

        if (tc == Cast) {
            node.m_init = new CastExpr(
                node.get_init()->get_span(),
                node.get_type(),
                node.get_init());
        } else if (tc != Match) {
            Logger::error("cannot initialize a variable of type '" 
                + node.get_type().to_string() + "' with initializer of type '" 
                + node.get_init()->get_type().to_string() + "'", 
                node.get_span());
        }
    }
}

void Sema::visit(ParameterDecl &node) {

}

void Sema::visit(FunctionDecl &node) {
    m_fn = &node;

    if (node.is_main()) {
        // Check that the signature of the 'main' function is one of the 
        // following:
        //
        // int ()
        // int (int)
        // int (int, char**)
        // int (int, char**, char**)

        const QualType& type = node.get_type();
        const FunctionType* f_type = static_cast<const FunctionType*>(
            type.get_type());

        // TODO: For the following type checks, check edge cases surround
        // const qualifiers on any of the signature types.

        // Check that 'main' returns 'int'.
        const QualType& r_type = f_type->get_return_type();
        if (r_type != BuiltinType::get_int_type(*m_tctx))
            Logger::error("'main' must return 'int'", node.get_span());
        
        // Check that main has no more than 3 parameters.
        if (node.num_params() > 3)
            Logger::error("'main' must have at most 3 parameters", node.get_span());

        // Check that the first parameter is of type 'int'.
        if (node.num_params() > 0) {
            const QualType& p_type = f_type->get_param_type(0);
            if (p_type != BuiltinType::get_int_type(*m_tctx)) {
                Logger::error("'main' first parameter must have type 'int'", 
                    node.get_span());
            }
        }

        // Check that the second parameter is of type 'char**'.
        if (node.num_params() > 1) {
            const QualType& p_type = f_type->get_param_type(1);
            if (p_type != PointerType::get_char_pp(*m_tctx)) {
                Logger::error("'main' second parameter must have type 'char**'", 
                    node.get_span());
            }
        }

        // Check that the third parameter is of type 'char**'.
        if (node.num_params() > 2) {
            const QualType& p_type = f_type->get_param_type(2);
            if (p_type != PointerType::get_char_pp(*m_tctx)) {
                Logger::error("'main' third parameter must have type 'char**'", 
                    node.get_span());
            }
        }
    }

    m_dctx = &node;

    if (node.has_body())
        node.m_body->accept(*this);

    m_dctx = m_dctx->get_parent();
    m_fn = nullptr;
}

void Sema::visit(FieldDecl &node) {

}

void Sema::visit(TypedefDecl &node) {

}

void Sema::visit(RecordDecl &node) {

}

void Sema::visit(EnumVariantDecl &node) {

}

void Sema::visit(EnumDecl &node) {

}

void Sema::visit(CompoundStmt &node) {
    for (Stmt* stmt : node.m_stmts)
        stmt->accept(*this);
}

void Sema::visit(DeclStmt &node) {

}

void Sema::visit(ExprStmt &node) {

}

void Sema::visit(IfStmt &node) {
    node.get_cond()->accept(*this);

    if (!is_bool_evaluable(node.get_cond()->get_type())) {
        Logger::error("'if' condition must be evaluable to a boolean; got '" 
            + node.get_cond()->get_type().to_string() + "'", 
            node.get_cond()->get_span());
    }

    if (dynamic_cast<DeclStmt*>(node.m_then)) {
        Logger::warn("declaration outside concrete scope", 
            node.m_then->get_span());
    }

    node.m_then->accept(*this);

    if (node.has_else()) {
        if (dynamic_cast<DeclStmt*>(node.m_else)) {
            Logger::warn("declaration outside concrete scope",
                node.m_else->get_span());
        }

        node.m_else->accept(*this);
    }
}

void Sema::visit(ReturnStmt &node) {
    if (!m_fn)
        Logger::error("'return' outside of function", node.get_span());
    
    if (node.has_expr()) {
        node.m_expr->accept(*this);

        TypeCheckResult tc = type_check(
            node.get_expr()->get_type(), 
            m_fn->get_return_type(), 
            AllowImplicit);

        if (tc == Cast) {
            node.m_expr = new CastExpr(
                node.m_expr->get_span(),
                m_fn->get_return_type(),
                node.m_expr);
        } else if (tc != Match) {
            Logger::error("return type mismatch; got '" 
                + node.get_expr()->get_type().to_string() + "'", 
                node.get_span());
        }
    } else if (!m_fn->get_return_type()->is_void()) {
        Logger::error("function returns 'void'", node.get_span());
    }
}

void Sema::visit(BreakStmt &node) {
    if (m_loop == None)
        Logger::error("'break' outside of loop", node.get_span());
}

void Sema::visit(ContinueStmt &node) {
    if (m_loop == None)
        Logger::error("'continue' outside of loop", node.get_span());
}

void Sema::visit(WhileStmt &node) {
    node.get_cond()->accept(*this);

    if (!is_bool_evaluable(node.get_cond()->get_type())) {
        Logger::error("'while' condition must be evaluable to a boolean; got '" 
            + node.get_cond()->get_type().to_string() + "'", 
            node.get_cond()->get_span());
    }

    if (node.has_body()) {
        LoopKind prev_loop = m_loop;
        m_loop = While;

        if (dynamic_cast<const DeclStmt*>(node.get_body()))
            Logger::warn("declaration outside concrete scope", 
                node.get_body()->get_span());

        node.get_body()->accept(*this);
        m_loop = prev_loop;
    }
}

void Sema::visit(ForStmt &node) {
    if (node.has_init())
        node.get_init()->accept(*this);

    if (node.has_cond())
        node.get_cond()->accept(*this);

    if (node.has_step())
        node.get_step()->accept(*this);

    if (node.has_body()) {
        LoopKind prev_loop = m_loop;
        m_loop = For;

        node.get_body()->accept(*this);
        m_loop = prev_loop;
    }

    if (!is_bool_evaluable(node.get_cond()->get_type()))
        Logger::error("'?' operator condition must be evaluable to a boolean", 
            node.get_cond()->get_span());
}

void Sema::visit(CaseStmt &node) {

}

void Sema::visit(SwitchStmt &node) {

}

void Sema::visit(BinaryExpr &node) {
    node.m_left->accept(*this);
    node.m_right->accept(*this);

    const QualType& l_type = node.m_left->get_type();
    const QualType& r_type = node.m_right->get_type();

    TypeCheckMode mode = AllowImplicit;
    
    TypeCheckResult tc = type_check(r_type, l_type, mode);
    if (tc == Cast) {
        node.m_right = new CastExpr(
            node.m_right->get_span(),
            l_type,
            node.m_right);
    } else if (tc != Match) {
        Logger::error("type mismatch: '" + l_type.to_string() + "' and '" 
            + r_type.to_string() + "'", node.get_span());
    }

    if (BinaryExpr::is_comparison(node.get_operator())) {
        node.m_type = BuiltinType::get_int_type(*m_tctx);
        return;
    }

    node.m_type = l_type;
    if (!BinaryExpr::is_assignment(node.get_operator()))
        return;

    // Check if the rhs is an lvalue.

    // Perform 'const' checks on lvalues for assignment operators.
}

void Sema::visit(UnaryExpr &node) {
    node.get_expr()->accept(*this);
    node.m_type = node.get_expr()->get_type();

    switch (node.get_operator()) {
    case UnaryExpr::LogicNot:
        node.m_type = BuiltinType::get_int_type(*m_tctx);
        break;
    
    case UnaryExpr::AddressOf:
        // TODO: Check that the base expression is an lvalue.
        break;
    
    case UnaryExpr::Dereference: {
        const QualType& e_type = node.get_expr()->get_type();
        if (e_type->get_kind() != Type::Pointer) {
            Logger::error("'*' operator does not work on type '" 
                + e_type.to_string() + "'", node.get_span());
        }

        node.m_type = 
            static_cast<const PointerType*>(e_type.get_type())->get_pointee();
        break;
    }

    case UnaryExpr::Increment:
    case UnaryExpr::Decrement:
        // Perform 'const' checks on base values for mutating operators.

    default:
        break;
    }   
}

void Sema::visit(ParenExpr &node) {
    node.m_expr->accept(*this);
    node.m_type = node.get_expr()->get_type();
}

void Sema::visit(RefExpr &node) {
    
}

void Sema::visit(CallExpr &node) {
    for (Expr* arg : node.get_args())
        arg->accept(*this);
}

void Sema::visit(CastExpr &node) {
    node.m_expr->accept(*this);

    const QualType& e_type = node.get_expr()->get_type();
    const QualType& c_type = node.get_type();

    if (!can_cast(e_type, c_type)) {
        Logger::error("cannot cast '" + e_type.to_string() + "' to '" 
            + c_type.to_string() + "'", node.get_span());
    }
}

void Sema::visit(SizeofExpr &node) {

}

void Sema::visit(SubscriptExpr &node) {
    node.get_base()->accept(*this);
    node.get_index()->accept(*this);

    // Check that the index type is an integer.
    if (!node.get_index()->get_type()->is_integer())
        Logger::error("'[]' operator index must be an integer type", 
            node.get_span());

    const QualType& b_type = node.get_base()->get_type();
    if (b_type->get_kind() == Type::Array) {
        node.m_type = 
            static_cast<const ArrayType*>(b_type.get_type())->get_element();
    } else if (b_type->get_kind() == Type::Pointer) {
        node.m_type = 
            static_cast<const PointerType*>(b_type.get_type())->get_pointee();
    } else {
        Logger::error("invalid '[]' base operator type: '" + b_type.to_string() 
            + "'", node.get_span());
    }
}

void Sema::visit(MemberExpr &node) {
    node.get_base()->accept(*this);
}

void Sema::visit(TernaryExpr &node) {
    node.get_cond()->accept(*this);

    if (!is_bool_evaluable(node.get_cond()->get_type()))
        Logger::error("'?' operator condition must be evaluable to a boolean", 
            node.m_cond->get_span());

    node.get_true_value()->accept(*this);
    node.get_false_value()->accept(*this);

    const QualType& t_type = node.get_true_value()->get_type();
    const QualType& f_type = node.get_false_value()->get_type();

    TypeCheckResult tc = type_check(f_type, t_type, AllowImplicit);
    if (tc == Cast) {
        node.m_fval = new CastExpr(
            node.m_fval->get_span(),
            t_type,
            node.m_fval);
    } else if (tc != Match) {
        Logger::error("type mismatch: '" + t_type.to_string() + "' and '" + 
            f_type.to_string() + "'", node.get_span());
    }
}

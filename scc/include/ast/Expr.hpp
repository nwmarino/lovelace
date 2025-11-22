//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SCC_EXPR_H_
#define SCC_EXPR_H_

//
// This header file declares all the recogniezd C expression nodes in the
// abstract syntax tree.
//

#include "ast/Decl.hpp"
#include "ast/QualType.hpp"
#include "core/Span.hpp"

#include <cassert>
#include <memory>
#include <vector>

namespace scc {

/// Base class for all expression nodes in the abstract syntax tree.
class Expr {
public:
    enum Kind : uint32_t {
        IntegerLiteral,
        FPLiteral,
        CharLiteral,
        StringLiteral,
        Binary,
        Unary,
        Ref,
        Call,
        Cast,
        Sizeof,
        /*
        Member,
        Subscript,
        Ternary,
        */
    };

protected:
    /// The kind of expression this is.
    const Kind m_kind;

    /// The span of source code that this expression covers.
    const Span m_span;

    /// The type of this expression.
    QualType m_type;

public:
    Expr(Kind kind, const Span& span, const QualType& ty)
        : m_kind(kind), m_span(span), m_type(ty) {}

    Expr(const Expr&) = delete;
    Expr& operator = (const Expr&) = delete;

    virtual ~Expr() = default;

    /// Returns the kind of expression this is.
    Kind kind() const { return m_kind; }

    /// Returns the span of source code this expression covers.
    const Span& span() const { return m_span; }

    /// Returns the type of this expression.
    const QualType& get_type() const { return m_type; }
    QualType& get_type() { return m_type; }
};

/// Represents integer literal expressions, e.g. '0' and '1'.
class IntegerLiteral final : public Expr {
    /// The integer value of this literal.
    int64_t m_value;

public:
    IntegerLiteral(const Span& span, const QualType& type, int64_t value)
        : Expr(Kind::IntegerLiteral, span, type), m_value(value) {}

    IntegerLiteral(const IntegerLiteral&) = delete;
    IntegerLiteral& operator = (const IntegerLiteral&) = delete;

    /// Returns the value of this literal as an integer.
    int64_t get_value() const { return m_value; }
};

/// Represents floating point literal expressions, e.g. '0.1' and '3.14'.
class FPLiteral final : public Expr {
    /// The floating point value of this literal.
    double m_value;

public:
    FPLiteral(const Span& span, const QualType& ty, double value)
        : Expr(Kind::FPLiteral, span, ty), m_value(value) {}

    FPLiteral(const FPLiteral&) = delete;
    FPLiteral& operator = (const FPLiteral&) = delete;

    /// Returns the value of this literal as a floating point.
    double get_value() const { return m_value; }
};

/// Represents floating point literal expressions, e.g. 'a' and 'b'.
class CharLiteral final : public Expr {
    /// The character value of this literal.
    char m_value;

public:
    CharLiteral(const Span& span, const QualType& ty, char value)
        : Expr(Kind::CharLiteral, span, ty), m_value(value) {}

    CharLiteral(const CharLiteral&) = delete;
    CharLiteral& operator = (const CharLiteral&) = delete;

    /// Returns the value of this literal as a character.
    char get_value() const { return m_value; }
};

/// Represents floating point literal expressions, e.g. "Hello" and "World!".
class StringLiteral final : public Expr {
    /// The string value of this literal.
    std::string m_value;

public:
    StringLiteral(const Span& span, const QualType& ty, 
                  const std::string& value)
        : Expr(Kind::StringLiteral, span, ty), m_value(value) {}

    StringLiteral(const StringLiteral&) = delete;
    StringLiteral& operator = (const StringLiteral&) = delete;

    /// Returns the value of this literal as a string.
    const std::string& get_value() const { return m_value; }
};

/// Represents binary operations between two nested expressions.
class BinaryExpr final : public Expr {
public:
    /// Possible kinds of binary operations.
    enum Op : uint32_t {
        Assign,
        Add,
        AddAssign,
        Sub,
        SubAssign,
        Mul,
        MulAssign,
        Div,
        DivAssign,
        Mod,
        ModAssign,
        And,
        AndAssign,
        Or,
        OrAssign,
        Xor,
        XorAssign,
        LeftShift,
        LeftShiftAssign,
        RightShift,
        RightShiftAssign,
        Equals,
        NotEquals,
        LessThan,
        LessThanEquals,
        GreaterThan,
        GreaterThanEquals,
        LogicAnd,
        LogicOr,
    };

private:
    /// The operator of this binary expression.
    Op m_operator;

    /// The left hand side expression of this operation.
    std::unique_ptr<Expr> m_left;

    /// The right hand side expression of this operation.
    std::unique_ptr<Expr> m_right;

public:
    BinaryExpr(const Span& span, const QualType& ty, Op op, 
               std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
        : Expr(Kind::Binary, span, ty), m_operator(op), m_left(std::move(left)), 
          m_right(std::move(right)) {}

    BinaryExpr(const BinaryExpr&) = delete;
    BinaryExpr& operator = (const BinaryExpr&) = delete;

    /// Returns the operator of this binary expression.
    Op get_operator() const { return m_operator; }

    /// Returns the left hand side expression of this operator.
    const Expr* get_lhs() const { return m_left.get(); }
    Expr* get_lhs() { return m_left.get(); }

    /// Returns the right hand side expression of this operator.
    const Expr* get_rhs() const { return m_right.get(); }
    Expr* get_rhs() { return m_right.get(); }
};

/// Represents unary operations over a nested expression.
class UnaryExpr final : public Expr {
public:
    /// Possible kinds of unary operators.
    enum Op : uint32_t {
        Not,
        LogicNot,
        Negate,
        AddressOf,
        Dereference,
        Increment,
        Decrement,
    };

private:
    /// The operator of this unary expression.
    Op m_operator;

    /// If true, then this expression uses a postfix operator instead of a
    /// prefix one. If false, then it uses a prefix.
    bool m_postfix;

    /// The nested expression this operates on.
    std::unique_ptr<Expr> m_expr;

public:
    UnaryExpr(const Span& span, const QualType& ty, Op op, bool postfix, 
              std::unique_ptr<Expr> expr)
        : Expr(Kind::Unary, span, ty), m_operator(op), m_postfix(postfix),
          m_expr(std::move(expr)) {}

    UnaryExpr(const UnaryExpr&) = delete;
    UnaryExpr& operator = (const UnaryExpr&) = delete;

    /// Returns the operator of this unary expression.
    Op get_operator() const { return m_operator; }

    /// Returns true if this is a prefix unary operation.
    bool is_prefix() const { return !m_postfix; }

    /// Returns true if this is a postfix unary operation.
    bool is_postfix() const { return m_postfix; }

    /// Returns the expression that this unary operation works on.
    const Expr* get_expr() const { return m_expr.get(); }
    Expr* get_expr() { return m_expr.get(); }
};

/// Represents a valued reference to some declaration.
class RefExpr final : public Expr {
    /// The declaration that this expression references.
    const Decl* m_decl;

public:
    RefExpr(const Span& span, const QualType& ty, const Decl* decl)
        : Expr(Kind::Ref, span, ty), m_decl(decl) {}

    RefExpr(const RefExpr&) = delete;
    RefExpr& operator = (const RefExpr&) = delete;

    /// Returns the declaration that this expression references.
    const Decl* get_decl() const { return m_decl; }

    /// Set the declaration this expression references to \p decl.
    void set_decl(const Decl* decl) { m_decl = decl; }

    /// Returns the name of the declaration that this expression references.
    const std::string& get_name() const { 
        assert(m_decl != nullptr && "declaration not set!");
        return m_decl->name(); 
    }
};

/// Represents a call to some function declaration.
class CallExpr final : public Expr {
    /// The base or callee expression of this function call.
    std::unique_ptr<Expr> m_callee;
    
    /// The argument expression to this function call.
    std::vector<std::unique_ptr<Expr>> m_args;

public:
    CallExpr(const Span& span, const QualType& ty, std::unique_ptr<Expr> callee, 
             std::vector<std::unique_ptr<Expr>>& args)
        : Expr(Kind::Call, span, ty), m_callee(std::move(callee)), 
          m_args(std::move(args)) {}

    CallExpr(const CallExpr&) = delete;
    CallExpr& operator = (const CallExpr&) = delete;

    /// Returns the base or callee expression of this function call.
    const Expr* get_callee() const { return m_callee.get(); }
    Expr* get_callee() { return m_callee.get(); }

    /// Returns the number of arguments in this function call.
    uint32_t num_args() const { return m_args.size(); }

    /// Returns true if this function call has any arguments.
    bool has_args() const { return !m_args.empty(); }

    /// Returns the argument expression at position \p i of this function call.
    const Expr* get_arg(uint32_t i) const {
        assert(i < num_args() && "index out of bounds!");
        return m_args[i].get();
    }

    Expr* get_arg(uint32_t i) {
        return const_cast<Expr*>(
            static_cast<const CallExpr*>(this)->get_arg(i)); 
    }
};

/// Represents a C-style type casting expression. This node can represent both
/// explicit casts defined by source code, and ones implicitly injected by the
/// compiler.
class CastExpr final : public Expr {
    /// The expression to type cast.
    std::unique_ptr<Expr> m_expr;

public:
    CastExpr(const Span& span, const QualType& ty, std::unique_ptr<Expr> expr)
        : Expr(Kind::Cast, span, ty), m_expr(std::move(expr)) {}

    CastExpr(const CastExpr&) = delete;
    CastExpr& operator = (const CastExpr&) = delete;

    /// Returns the expression that this type cast works on.
    const Expr* get_expr() const { return m_expr.get(); }
    Expr* get_expr() { return m_expr.get(); }
};

/// Represents a 'sizeof' compile-time expression over some type.
class SizeofExpr final : public Expr {
    /// The type to evaluate the size of.
    const Type* m_target;

public:
    SizeofExpr(const Span& span, const QualType& ty, const Type* target)
        : Expr(Kind::Sizeof, span, ty), m_target(target) {}

    SizeofExpr(const SizeofExpr&) = delete;
    SizeofExpr& operator = (const SizeofExpr&) = delete;

    /// Returns the type that this sizeof operator works on.
    const Type* get_target() const { return m_target; }
};

/*

class MemberExpr final : public Expr {
    std::unique_ptr<Expr> m_base;
    std::string m_member;
};

class SubscriptExpr final : public Expr {
    std::unique_ptr<Expr> m_base;
    std::unique_ptr<Expr> m_index;
};

class TernaryExpr final : public Expr {
    std::unique_ptr<Expr> m_condition;
    std::unique_ptr<Expr> m_true;
    std::unique_ptr<Expr> m_else;
};

*/

} // namespace scc

#endif // SCC_EXPR_H_

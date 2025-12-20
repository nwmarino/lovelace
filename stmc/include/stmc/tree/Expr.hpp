//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef STATIM_EXPR_H_
#define STATIM_EXPR_H_

#include "stmc/tree/Stmt.hpp"
#include "stmc/tree/TypeUse.hpp"
#include "stmc/tree/Visitor.hpp"
#include "stmc/types/SourceSpan.hpp"

#include <cassert>
#include <string>
#include <vector>

namespace stm {

using std::string;
using std::vector;

class Decl;
class FieldDecl;
class NamedDecl;
class ValueDecl;
class Type;

/// Base class for all expression nodes in the abstract syntax tree (AST).
///
/// An expression is considered a typed statement that produces a value.
class Expr : public Stmt {
protected:
    TypeUse m_type;

    Expr(SourceSpan span, const TypeUse& type) : Stmt(span), m_type(type) {}

public:
    virtual ~Expr() = default;

    Expr(const Expr&) = delete;
    void operator=(const Expr&) = delete;

    Expr(Expr&&) noexcept = delete;
    void operator=(Expr&&) noexcept = delete;

    virtual void accept(Visitor& visitor) = 0;

    /// Test if this expression is constant i.e. is known at compile-time.
    virtual bool is_constant() const { return false; }

    /// Test if this expression may be used as an lvalue.
    virtual bool is_lvalue() const { return false; }

    void set_type(const TypeUse& type) { m_type = type; }
    const TypeUse& get_type() const { return m_type; }
    TypeUse& get_type() { return m_type; }
};

/// Representation of boolean literals, e.g. 'true' or 'false'.
class BoolLiteral final : public Expr {
    bool m_value;

    BoolLiteral(SourceSpan span, const TypeUse& type, bool value)
      : Expr(span, type), m_value(value) {}

public:
    static BoolLiteral* create(Context& ctx, SourceSpan span, bool value);

    ~BoolLiteral() = default;

    BoolLiteral(const BoolLiteral&) = delete;
    void operator=(const BoolLiteral&) = delete;

    BoolLiteral(BoolLiteral&&) noexcept = delete;
    void operator=(BoolLiteral&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    bool is_constant() const override { return true; }

    bool get_value() const { return m_value; }
};

/// Representation of integer literals, e.g. '1'.
class IntegerLiteral final : public Expr {
    int64_t m_value;

    IntegerLiteral(SourceSpan span, const TypeUse& type, int64_t value)
      : Expr(span, type), m_value(value) {}

public:
    static IntegerLiteral* create(Context& ctx, SourceSpan span, 
                                  const TypeUse& type, int64_t value);

    ~IntegerLiteral() = default;
    
    IntegerLiteral(const IntegerLiteral&) = delete;
    void operator=(const IntegerLiteral&) = delete;

    IntegerLiteral(IntegerLiteral&&) noexcept = delete;
    void operator=(IntegerLiteral&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    bool is_constant() const override { return true; }

    int64_t get_value() const { return m_value; }
};

/// Representation of floating point literals, e.g. '3.14'.
class FPLiteral final : public Expr {
    double m_value;

    FPLiteral(SourceSpan span, const TypeUse& type, double value)
      : Expr(span, type), m_value(value) {}

public:
    static FPLiteral* create(Context& ctx, SourceSpan span, const TypeUse& type,
                             double value);

    ~FPLiteral() = default;

    FPLiteral(const FPLiteral&) = delete;
    void operator=(const FPLiteral&) = delete;

    FPLiteral(FPLiteral&&) noexcept = delete;
    void operator=(FPLiteral&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    bool is_constant() const override { return true; }

    double get_value() const { return m_value; }
};

/// Representation of character literals, e.g. 'a'.
class CharLiteral final : public Expr {
    char m_value;

    CharLiteral(SourceSpan span, const TypeUse& type, char value)
      : Expr(span, type), m_value(value) {}

public:
    static CharLiteral* create(Context& ctx, SourceSpan span, char value);

    ~CharLiteral() = default;
    
    CharLiteral(const CharLiteral&) = delete;
    void operator=(const CharLiteral&) = delete;

    CharLiteral(CharLiteral&&) noexcept = delete;
    void operator=(CharLiteral&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    bool is_constant() const override { return true; }

    char get_value() const { return m_value; }
};

/// Representation of string literals, e.g. "hello".
class StringLiteral final : public Expr {
    string m_value;

    StringLiteral(SourceSpan span, const TypeUse& type, const string& value)
      : Expr(span, type), m_value(value) {}

public:
    static StringLiteral* create(Context& ctx, SourceSpan span, 
                                 const string& value);

    ~StringLiteral() = default;

    StringLiteral(const StringLiteral&) = delete;
    void operator=(const StringLiteral&) = delete;

    StringLiteral(StringLiteral&&) noexcept = delete;
    void operator=(StringLiteral&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    bool is_constant() const override { return true; }

    const string& get_value() const { return m_value; }
};

/// Representation of null literals, e.g. 'null'.
class NullLiteral final : public Expr {
    NullLiteral(SourceSpan span, const TypeUse& type) : Expr(span, type) {}

public:
    static NullLiteral* create(Context& ctx, SourceSpan span, 
                               const TypeUse& type);

    ~NullLiteral() = default;

    NullLiteral(const NullLiteral&) = delete;
    void operator=(const NullLiteral&) = delete;

    NullLiteral(NullLiteral&&) noexcept = delete;
    void operator=(NullLiteral&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    bool is_constant() const override { return true; }
};

/// Represents a binary operation expression.
class BinaryOp final : public Expr {
    friend class SemanticAnalysis;

public:
    /// Possible kinds of binary operators.
    enum Operator : uint32_t {
        Unknown = 0,
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
        LogicAnd,
        LogicOr,
        Equals,
        NotEquals,
        LessThan,
        LessThanEquals,
        GreaterThan,
        GreaterThanEquals,
    };

    /// Test if the operator \p op is a basic assignment '='.
    static bool is_direct_assignment(Operator op) { return op == Assign; }

    /// Test if the operator \p op is a basic assignment or shorthand 
    /// assignment operator e.g. '+='.
    static bool is_assignment(Operator op);

    /// Test if the operator \p op performs any kind of comparison that would
    /// result in a boolean value.
    static bool is_comparison(Operator op) {
        return is_numerical_comparison(op) || is_logical_comparison(op);
    }

    /// Test if the operator \p op performs a numerical comparison.
    static bool is_numerical_comparison(Operator op) {
        return Equals <= op && op <= GreaterThanEquals;
    }

    /// Test if the operator \p op performs a bitwise comparison.
    static bool is_bitwise_comparison(Operator op) {
        return op == And || op == Or || op == Xor;
    }

    /// Test if the operator \p op performs a logical comparison.
    static bool is_logical_comparison(Operator op) {
        return op == LogicAnd || op == LogicOr;
    }

private:
    Operator m_oper;
    Expr* m_lhs;
    Expr* m_rhs;

    BinaryOp(SourceSpan span, const TypeUse& type, Operator oper, 
             Expr* lhs, Expr* rhs)
      : Expr(span, type), m_oper(oper), m_lhs(lhs), m_rhs(rhs) {}

public:
    static BinaryOp* create(Context& ctx, SourceSpan span, Operator oper, 
                            Expr* lhs, Expr* rhs);

    ~BinaryOp() override;
    
    BinaryOp(const BinaryOp&) = delete;
    void operator = (const BinaryOp&) = delete;

    BinaryOp(BinaryOp&&) noexcept = delete;
    void operator=(BinaryOp&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    bool is_constant() const override { 
        return m_lhs->is_constant() && m_rhs->is_constant(); 
    }

    Operator get_operator() const { return m_oper; }

    const Expr* get_lhs() const { return m_lhs; }
    Expr* get_lhs() { return m_lhs; }

    const Expr* get_rhs() const { return m_rhs; }
    Expr* get_rhs() { return m_rhs; }
};

/// Represents a unary operation expression.
class UnaryOp final : public Expr {
public:
    /// Possible kinds of unary operators.
    enum Operator : uint32_t {
        Unknown = 0,
        Increment,
        Decrement,
        Negate,
        Not,
        LogicNot,
        AddressOf,
        Dereference,
    };

    /// Test if the operator \p op can be used as a prefix operator.
    static bool is_prefix(Operator op) {
        return op != Unknown;
    }

    /// Test if the operator \p op can be used as a prefix operator.
    static bool is_postfix(Operator op) {
        return op == Increment || op == Decrement;
    }

private:
    Operator m_oper;
    bool m_prefix;
    Expr* m_expr;

    UnaryOp(SourceSpan span, const TypeUse& type, Operator oper, bool prefix, 
            Expr* expr)
      : Expr(span, type), m_oper(oper), m_prefix(prefix), m_expr(expr) {}

public:
    static UnaryOp* create(Context& ctx, SourceSpan span, Operator oper, 
                           bool prefix, Expr* expr);

    ~UnaryOp() override;

    UnaryOp(const UnaryOp&) = delete;
    void operator=(const UnaryOp&) = delete;

    UnaryOp(UnaryOp&&) noexcept = delete;
    void operator=(UnaryOp&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    bool is_constant() const override { 
        return m_expr->is_constant() || m_oper == AddressOf; 
    }

    bool is_lvalue() const override { return m_oper == Dereference; }

    Operator get_operator() const { return m_oper; }

    /// Test if this unary operation is interpreted as a prefix operator.
    bool is_prefix() const { return m_prefix; }

    /// Test if this unary operation is interpreted as a postfix operator.
    bool is_postfix() const { return !m_prefix; }

    const Expr* get_expr() const { return m_expr; }
    Expr* get_expr() { return m_expr; }
};

/// Represents a cast expression, i.e. 'cast<T>(...)'
class CastExpr final : public Expr {
    Expr* m_expr;

    CastExpr(SourceSpan span, const TypeUse& type, Expr* expr)
      : Expr(span, type), m_expr(expr) {}

public:
    static CastExpr* create(Context& ctx, SourceSpan span, const TypeUse& type, 
                            Expr* expr);

    ~CastExpr() override;
    
    CastExpr(const CastExpr&) = delete;
    void operator=(const CastExpr&) = delete;

    CastExpr(CastExpr&&) noexcept = delete;
    void operator=(CastExpr&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    bool is_constant() const override { return m_expr->is_constant(); }

    const Expr* get_expr() const { return m_expr; }
    Expr* get_expr() { return m_expr; }
};

/// Represents an expression within parentheses, i.e. '(...)'.
class ParenExpr final : public Expr {
    Expr* m_expr;

    ParenExpr(SourceSpan span, const TypeUse& type, Expr* expr)
      : Expr(span, type), m_expr(expr) {}

public:
    static ParenExpr* create(Context& ctx, SourceSpan span, Expr* expr);

    ~ParenExpr() override;

    ParenExpr(const ParenExpr&) = delete;
    void operator=(const ParenExpr&) = delete;

    ParenExpr(ParenExpr&&) noexcept = delete;
    void operator=(ParenExpr&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    bool is_constant() const override { return m_expr->is_constant(); }

    const Expr* get_expr() const { return m_expr; }
    Expr* get_expr() { return m_expr; }
};

/// Represents a 'sizeof(T)' expression.
class SizeofExpr final : public Expr {
    // The type to get the size of.
    TypeUse m_target;

    SizeofExpr(SourceSpan span, const TypeUse& type, const TypeUse& target)
      : Expr(span, type), m_target(target) {}

public:
    static SizeofExpr* create(Context& ctx, SourceSpan span, 
                              const TypeUse& target);

    ~SizeofExpr() = default;
    
    SizeofExpr(const SizeofExpr&) = delete;
    void operator = (const SizeofExpr&) = delete;

    SizeofExpr(SizeofExpr&&) noexcept = delete;
    void operator=(SizeofExpr&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    bool is_constant() const override { return true; }

    const TypeUse& get_target_type() const { return m_target; }
    TypeUse& get_target_type() { return m_target; }
};

/// Represents a field access '.' expression.
class AccessExpr final : public Expr {
    Expr* m_base;
    
    // The name of the structure field to access. Used for forwarding.
    string m_name;

    // The structure field to access.
    const FieldDecl* m_field;

    AccessExpr(SourceSpan span, const TypeUse& type, Expr* base, 
               const string& name, const FieldDecl* field)
      : Expr(span, type), m_base(base), m_name(name), m_field(field) {}

public:
    static AccessExpr* create(Context& ctx, SourceSpan span, Expr* base, 
                              const string& name);

    ~AccessExpr() override;

    AccessExpr(const AccessExpr&) = delete;
    void operator=(const AccessExpr&) = delete;

    AccessExpr(AccessExpr&&) = delete;
    void operator=(AccessExpr&&) = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    bool is_lvalue() const override { return true; }

    const Expr* get_base() const { return m_base; }
    Expr* get_base() { return m_base; }

    const string& get_name() { return m_name; }

    void set_field(const FieldDecl* field) { m_field = field; }
    const FieldDecl* get_field() const { return m_field; }
};

/// Represents a subscript '[]' operator expression.
class SubscriptExpr final : public Expr {
    Expr* m_base;
    Expr* m_index;

    SubscriptExpr(SourceSpan span, const TypeUse& type, Expr* base, Expr* index)
      : Expr(span, type), m_base(base), m_index(index) {}

public:
    static SubscriptExpr* create(Context& ctx, SourceSpan span, Expr* base, 
                                 Expr* index);

    ~SubscriptExpr() override;
    
    SubscriptExpr(const SubscriptExpr&) = delete;
    void operator=(const SubscriptExpr&) = delete;

    SubscriptExpr(SubscriptExpr&&) noexcept = delete;
    void operator=(SubscriptExpr&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    bool is_lvalue() const override { return true; }

    const Expr* get_base() const { return m_base; }
    Expr* get_base() { return m_base; }

    const Expr* get_index() const { return m_index; }
    Expr* get_index() { return m_index; }
};

/// Represents a named declaration reference expression.
class DeclRefExpr final : public Expr {
    string m_name;
    const ValueDecl* m_decl;

public:
    DeclRefExpr(SourceSpan span, const TypeUse& type, const string& name, 
                const ValueDecl* decl)
      : Expr(span, type), m_name(name), m_decl(decl) {}

public:
    static DeclRefExpr* create(Context& ctx, SourceSpan span, 
                               const string& name, const ValueDecl* decl);

    ~DeclRefExpr() = default;
    
    DeclRefExpr(const DeclRefExpr&) = delete;
    void operator=(const DeclRefExpr&) = delete;

    DeclRefExpr(DeclRefExpr&&) noexcept = delete;
    void operator=(DeclRefExpr&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    bool is_lvalue() const override { return true; }

    const string& get_name() const { return m_name; }
    string& get_name() { return m_name; }

    void set_decl(const ValueDecl* decl) { m_decl = decl; }
    const ValueDecl* get_decl() const { return m_decl; }
};

/// Represents a function call '()' expression.
class CallExpr final : public Expr {
    friend class SemanticAnalysis;

    Expr* m_callee;
    vector<Expr*> m_args;

    CallExpr(SourceSpan span, const TypeUse& type, Expr* callee, 
             const vector<Expr*>& args)
      : Expr(span, type), m_callee(callee), m_args(args) {}

public:
    static CallExpr* create(Context& ctx, SourceSpan span, Expr* callee, 
                            const vector<Expr*>& args);

    ~CallExpr() override;
    
    CallExpr(const CallExpr&) = delete;
    void operator=(const CallExpr&) = delete;

    CallExpr(CallExpr&&) noexcept = delete;
    void operator=(CallExpr&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    const Expr* get_callee() const { return m_callee; }
    Expr* get_callee() { return m_callee; }

    uint32_t num_args() const { return m_args.size(); }
    bool has_args() const { return !m_args.empty(); }

    const vector<Expr*>& get_args() const { return m_args; }
    vector<Expr*>& get_args() { return m_args; }

    const Expr* get_arg(uint32_t i) const { 
        assert(i < num_args() && "index out of bounds!");
        return m_args[i]; 
    }

    Expr* get_arg(uint32_t i) { 
        assert(i < num_args() && "index out of bounds!");
        return m_args[i]; 
    }
};

} // namespace stm

#endif // STATIM_EXPR_H_

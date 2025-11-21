#ifndef SCC_EXPR_H_
#define SCC_EXPR_H_

#include "Decl.hpp"
#include "../core/Span.hpp"

#include <memory>
#include <vector>

namespace scc {

class Type;

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
        Call,
        Member,
        Subscript,
        Cast,
        Ref,
        Ternary,
    };

protected:
    const Kind m_kind;
    const Span m_span;
    std::shared_ptr<Type> m_type;

public:
    Expr(Kind kind, const Span& span, std::shared_ptr<Type> type);

    Expr(const Expr&) = delete;
    Expr& operator = (const Expr&) = delete;

    virtual ~Expr() = 0; 

    /// Returns the kind of expression this is.
    Kind get_kind() const { return m_kind; }

    /// Returns the span of source code this expression covers.
    const Span& get_span() const { return m_span; }

    /// Returns the type of this expression.
    const Type* get_type() const { return m_type.get(); }
    Type* get_type() { return m_type.get(); }
};

/// Represents integer literal expressions, e.g. `0` and `1`.
class IntegerLiteral final : public Expr {
    int64_t m_value;

public:
    IntegerLiteral(const Span& span, std::shared_ptr<Type> type, int64_t value);

    IntegerLiteral(const IntegerLiteral&) = delete;
    IntegerLiteral& operator = (const IntegerLiteral&) = delete;

    ~IntegerLiteral() override = default;

    /// Returns the value of this literal as an integer.
    int64_t get_value() const { return m_value; }
};

/// Represents floating point literal expressions, e.g. `0.1` and `3.14`.
class FPLiteral final : public Expr {
    double m_value;

public:
    FPLiteral(const Span& span, std::shared_ptr<Type> type, double value);

    FPLiteral(const FPLiteral&) = delete;
    FPLiteral& operator = (const FPLiteral&) = delete;

    ~FPLiteral() override = default;

    /// Returns the value of this literal as a floating point.
    double get_value() const { return m_value; }
};

/// Represents floating point literal expressions, e.g. `a` and `b`.
class CharLiteral final : public Expr {
    char m_value;

public:
    CharLiteral(const Span& span, std::shared_ptr<Type> type, char value);

    CharLiteral(const CharLiteral&) = delete;
    CharLiteral& operator = (const CharLiteral&) = delete;

    ~CharLiteral() override = default;

    /// Returns the value of this literal as a character.
    char get_value() const { return m_value; }
};

/// Represents floating point literal expressions, e.g. `"Hello"` and `"World!"`.
class StringLiteral final : public Expr {
    std::string m_value;

public:
    StringLiteral(const Span& span, std::shared_ptr<Type> type, 
                  const std::string& value);

    StringLiteral(const StringLiteral&) = delete;
    StringLiteral& operator = (const StringLiteral&) = delete;

    ~StringLiteral() override = default;

    /// Returns the value of this literal as a string.
    const std::string& get_value() const { return m_value; }
};

class BinaryExpr final : public Expr {
public:
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
    Op m_operator;
    std::unique_ptr<Expr> m_left;
    std::unique_ptr<Expr> m_right;

public:
    BinaryExpr(const Span& span, std::shared_ptr<Type> type, Op op, 
               std::unique_ptr<Expr> left, std::unique_ptr<Expr> right);

    BinaryExpr(const BinaryExpr&) = delete;
    BinaryExpr& operator = (const BinaryExpr&) = delete;

    ~BinaryExpr() override = default;
};

class UnaryExpr final : public Expr {
public:
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
    Op m_operator;
    bool m_postfix;
    std::unique_ptr<Expr> m_expr;

public:
    UnaryExpr(const Span& span, std::shared_ptr<Type> type, Op op, 
              bool postfix, std::unique_ptr<Expr> expr);

    UnaryExpr(const UnaryExpr&) = delete;
    UnaryExpr& operator = (const UnaryExpr&) = delete;

    ~UnaryExpr() override = default;

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

class RefExpr final : public Expr {
    /// The declaration that this expression references.
    const Decl* m_decl;

public:
    RefExpr(const Span& span, std::shared_ptr<Type> type, const Decl* decl);

    RefExpr(const RefExpr&) = delete;
    RefExpr& operator = (const RefExpr&) = delete;

    ~RefExpr() override = default;

    /// Returns the declaration that this expression references.
    const Decl* get_decl() const { return m_decl; }

    /// Returns the name of the declaration that this expression references.
    const std::string& get_name() const { return m_decl->get_name(); }
};

class CallExpr final : public Expr {
    std::unique_ptr<Expr> m_callee;
    std::vector<std::unique_ptr<Expr>> m_args;
};

class MemberExpr final : public Expr {
    std::unique_ptr<Expr> m_base;
    std::string m_member;
};

class SubscriptExpr final : public Expr {
    std::unique_ptr<Expr> m_base;
    std::unique_ptr<Expr> m_index;
};

class CastExpr final : public Expr {
    std::unique_ptr<Expr> m_expr;
};

class TernaryExpr final : public Expr {
    std::unique_ptr<Expr> m_condition;
    std::unique_ptr<Expr> m_true;
    std::unique_ptr<Expr> m_else;
};

class SizeofExpr final : public Expr {
    std::shared_ptr<Type> m_type;
};

} // namespace scc

#endif // SCC_EXPR_H_

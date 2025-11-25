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
#include "ast/Visitor.hpp"
#include "core/SourceSpan.hpp"

#include <cassert>
#include <vector>

namespace scc {

using std::ostream;
using std::string;
using std::vector;

/// Base class for all expression nodes in the abstract syntax tree.
class Expr {
public:
    /// Possible kinds of C expressions.
    enum Kind : uint32_t {
        IntegerLiteral,
        FPLiteral,
        CharLiteral,
        StringLiteral,
        Binary,
        Unary,
        Paren,
        Ref,
        Call,
        Cast,
        Sizeof,
        Subscript,
        Member,
        Ternary,
    };

protected:
    /// The kind of expression this is.
    const Kind m_kind;

    /// The span of source code that this expression covers.
    SourceSpan m_span;

    /// The type of this expression.
    QualType m_type;

public:
    explicit Expr(Kind kind, const SourceSpan& span, const QualType& type)
        : m_kind(kind), m_span(span), m_type(type) {}

    Expr(const Expr&) = delete;
    Expr& operator = (const Expr&) = delete;

    virtual ~Expr() = default;

    /// Returns the kind of expression this is.
    Kind kind() const { return m_kind; }

    /// Returns the span of source code that this expression covers.
    const SourceSpan& get_span() const { return m_span; }
    SourceSpan& get_span() { return m_span; }
    
    /// Returns the location in source code that this expression starts at.
    const SourceLocation& get_starting_loc() const { return m_span.start; }
    SourceLocation& get_starting_loc() { return m_span.start; }

    /// Returns the location in source code that this expression ends at.
    const SourceLocation& get_ending_loc() const { return m_span.end; }
    SourceLocation& get_ending_loc() { return m_span.end; }

    /// Returns the type of this expression.
    const QualType& get_type() const { return m_type; }
    QualType& get_type() { return m_type; }

    /// Accept some visitor class \p visitor to access this node.
    virtual void accept(Visitor& visitor) = 0;

    /// Pretty-print this expression node to the output stream \p os.
    virtual void print(ostream& os) const = 0;
};

/// Represents integer literal expressions, e.g. '0' and '1'.
class IntegerLiteral final : public Expr {
    friend class Sema;
    friend class Codegen;

    /// The integer value of this literal.
    int64_t m_value;

public:
    IntegerLiteral(const SourceSpan& span, const QualType& type, int64_t value)
        : Expr(Kind::IntegerLiteral, span, type), m_value(value) {}

    IntegerLiteral(const IntegerLiteral&) = delete;
    IntegerLiteral& operator = (const IntegerLiteral&) = delete;

    /// Returns the value of this literal as an integer.
    int64_t get_value() const { return m_value; }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Represents floating point literal expressions, e.g. '0.1' and '3.14'.
class FPLiteral final : public Expr {
    friend class Sema;
    friend class Codegen;

    /// The floating point value of this literal.
    double m_value;

public:
    FPLiteral(const SourceSpan& span, const QualType& type, double value)
        : Expr(Kind::FPLiteral, span, type), m_value(value) {}

    FPLiteral(const FPLiteral&) = delete;
    FPLiteral& operator = (const FPLiteral&) = delete;

    /// Returns the value of this literal as a floating point.
    double get_value() const { return m_value; }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(std::ostream& os) const override;
};

/// Represents floating point literal expressions, e.g. 'a' and 'b'.
class CharLiteral final : public Expr {
    friend class Sema;
    friend class Codegen;

    /// The character value of this literal.
    char m_value;

public:
    CharLiteral(const SourceSpan& span, const QualType& type, char value)
        : Expr(Kind::CharLiteral, span, type), m_value(value) {}

    CharLiteral(const CharLiteral&) = delete;
    CharLiteral& operator = (const CharLiteral&) = delete;

    /// Returns the value of this literal as a character.
    char get_value() const { return m_value; }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Represents floating point literal expressions, e.g. "Hello" and "World!".
class StringLiteral final : public Expr {
    friend class Sema;
    friend class Codegen;

    /// The string value of this literal.
    string m_value;

public:
    StringLiteral(const SourceSpan& span, const QualType& type, const string& value)
        : Expr(Kind::StringLiteral, span, type), m_value(value) {}

    StringLiteral(const StringLiteral&) = delete;
    StringLiteral& operator = (const StringLiteral&) = delete;

    /// Returns the value of this literal as a string.
    const string& get_value() const { return m_value; }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Represents binary operations between two nested expressions.
class BinaryExpr final : public Expr {
    friend class Sema;
    friend class Codegen;

public:
    /// Possible kinds of binary operations.
    enum Op : uint32_t {
        Unknown = 0x0,
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

    /// Returns a stringified version of the operator \p op.
    static const char* to_string(Op op);

private:
    /// The operator of this binary expression.
    Op m_operator;

    /// The left hand side expression of this operation.
    Expr* m_left;

    /// The right hand side expression of this operation.
    Expr* m_right;

public:
    BinaryExpr(const SourceSpan& span, const QualType& type, Op op, Expr* left, 
               Expr* right)
        : Expr(Kind::Binary, span, type), m_operator(op), m_left(left), 
          m_right(right) {}

    BinaryExpr(const BinaryExpr&) = delete;
    BinaryExpr& operator = (const BinaryExpr&) = delete;

    ~BinaryExpr();

    /// Returns the operator of this binary expression.
    Op get_operator() const { return m_operator; }

    /// Returns the left hand side expression of this operator.
    const Expr* get_lhs() const { return m_left; }
    Expr* get_lhs() { return m_left; }

    /// Returns the right hand side expression of this operator.
    const Expr* get_rhs() const { return m_right; }
    Expr* get_rhs() { return m_right; }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Represents unary operations over a nested expression.
class UnaryExpr final : public Expr {
    friend class Sema;
    friend class Codegen;

public:
    /// Possible kinds of unary operators.
    enum Op : uint32_t {
        Unknown = 0x0,
        Not,
        LogicNot,
        Negate,
        AddressOf,
        Dereference,
        Increment,
        Decrement,
    };

    /// Returns true if the operator \p op can work as a prefix.
    static bool is_prefix_op(Op op) { return op != Unknown; }

    /// Returns true if the operator \p op can work as a postfix.
    static bool is_postfix_op(Op op) {
        return op == Increment || op == Decrement;
    }

    /// Returns a stringified version of the operator \p op.
    static const char* to_string(Op op);

private:
    /// The operator of this unary expression.
    Op m_operator;

    /// If true, then this expression uses a postfix operator instead of a
    /// prefix one. If false, then it uses a prefix.
    bool m_postfix;

    /// The nested expression this operates on.
    Expr* m_expr;

public:
    UnaryExpr(const SourceSpan& span, const QualType& type, Op op, bool postfix, 
              Expr* expr)
        : Expr(Kind::Unary, span, type), m_operator(op), m_postfix(postfix),
          m_expr(expr) {}

    UnaryExpr(const UnaryExpr&) = delete;
    UnaryExpr& operator = (const UnaryExpr&) = delete;
    
    ~UnaryExpr();

    /// Returns the operator of this unary expression.
    Op get_operator() const { return m_operator; }

    /// Returns true if this is a prefix unary operation.
    bool is_prefix() const { return !m_postfix; }

    /// Returns true if this is a postfix unary operation.
    bool is_postfix() const { return m_postfix; }

    /// Returns the expression that this unary operation works on.
    const Expr* get_expr() const { return m_expr; }
    Expr* get_expr() { return m_expr; }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Represents an expression enclosed with parentheses '(, )'.
class ParenExpr final : public Expr {
    friend class Sema;
    friend class Codegen;

    /// The nested expression.
    Expr* m_expr;

public:
    ParenExpr(const SourceSpan& span, const QualType& type, Expr* expr)
        : Expr(Kind::Paren, span, type), m_expr(expr) {}

    ParenExpr(const ParenExpr&) = delete;
    ParenExpr& operator = (const ParenExpr&) = delete;

    ~ParenExpr();

    /// Returns the nested expression.
    const Expr* get_expr() const { return m_expr; }
    Expr* get_expr() { return m_expr; }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Represents a valued reference to some declaration.
class RefExpr final : public Expr {
    friend class Sema;
    friend class Codegen;

    /// The declaration that this expression references.
    const ValueDecl* m_decl;

public:
    RefExpr(const SourceSpan& span, const QualType& type, const ValueDecl* decl)
        : Expr(Kind::Ref, span, type), m_decl(decl) {}

    RefExpr(const RefExpr&) = delete;
    RefExpr& operator = (const RefExpr&) = delete;

    /// Returns the declaration that this expression references.
    const ValueDecl* get_decl() const { return m_decl; }

    /// Set the declaration this expression references to \p decl.
    void set_decl(const ValueDecl* decl) { m_decl = decl; }

    /// Returns the name of the declaration that this expression references.
    const string& get_name() const { 
        assert(m_decl != nullptr && "declaration not set!");
        return m_decl->get_name(); 
    }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Represents a call to some function declaration.
class CallExpr final : public Expr {
    friend class Sema;
    friend class Codegen;

    /// The base or callee expression of this function call.
    Expr* m_callee;
    
    /// The argument expression to this function call.
    vector<Expr*> m_args;

public:
    CallExpr(const SourceSpan& span, const QualType& type, Expr* callee, 
             const vector<Expr*>& args)
        : Expr(Kind::Call, span, type), m_callee(callee), m_args(args) {}

    CallExpr(const CallExpr&) = delete;
    CallExpr& operator = (const CallExpr&) = delete;

    ~CallExpr();

    /// Returns the number of arguments in this function call.
    uint32_t num_args() const { return m_args.size(); }

    /// Returns true if this function call has any arguments.
    bool has_args() const { return !m_args.empty(); }

    /// Returns the base or callee expression of this function call.
    const Expr* get_callee() const { return m_callee; }
    Expr* get_callee() { return m_callee; }

    const vector<Expr*>& get_args() const { return m_args; }
    vector<Expr*>& get_args() { return m_args; }

    /// Returns the argument expression at position \p i of this function call.
    const Expr* get_arg(uint32_t i) const {
        assert(i < num_args() && "index out of bounds!");
        return m_args[i];
    }

    Expr* get_arg(uint32_t i) {
        return const_cast<Expr*>(
            static_cast<const CallExpr*>(this)->get_arg(i)); 
    }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Represents a C-style type casting expression. This node can represent both
/// explicit casts defined by source code, and ones implicitly injected by the
/// compiler.
class CastExpr final : public Expr {
    friend class Sema;
    friend class Codegen;

    /// The expression to type cast.
    Expr* m_expr;

public:
    CastExpr(const SourceSpan& span, const QualType& type, Expr* expr)
        : Expr(Kind::Cast, span, type), m_expr(expr) {}

    CastExpr(const CastExpr&) = delete;
    CastExpr& operator = (const CastExpr&) = delete;

    ~CastExpr();

    /// Returns the expression that this type cast works on.
    const Expr* get_expr() const { return m_expr; }
    Expr* get_expr() { return m_expr; }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Represents a 'sizeof' compile-time expression over some type.
class SizeofExpr final : public Expr {
    friend class Sema;
    friend class Codegen;

    /// The type to evaluate the size of.
    QualType m_target;

public:
    SizeofExpr(const SourceSpan& span, const QualType& type, const QualType& target)
        : Expr(Kind::Sizeof, span, type), m_target(target) {}

    SizeofExpr(const SizeofExpr&) = delete;
    SizeofExpr& operator = (const SizeofExpr&) = delete;

    /// Returns the type that this sizeof operator works on.
    const QualType& get_target() const { return m_target; }
    QualType& get_target() { return m_target; }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Represents a '[]' subscript expression.
class SubscriptExpr final : public Expr {
    friend class Sema;
    friend class Codegen;

    /// The base expression to access.
    Expr* m_base;

    /// The index to access the base expression at, i.e. the expression
    /// enclosed by braces '[, ]'.
    Expr* m_index;

public:
    SubscriptExpr(const SourceSpan& span, const QualType& type, Expr* base, 
                  Expr* index)
        : Expr(Kind::Subscript, span, type), m_base(base), m_index(index) {}

    SubscriptExpr(const SubscriptExpr&) = delete;
    SubscriptExpr& operator = (const SubscriptExpr&) = delete;

    ~SubscriptExpr();

    /// Returns the base expression of this subscript.
    const Expr* get_base() const { return m_base; }
    Expr* get_base() { return m_base; }

    /// Returns the index expression of this subscript.
    const Expr* get_index() const { return m_index; }
    Expr* get_index() { return m_index; }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(std::ostream& os) const override;
};

/// Represents a '.' or '->' member access expression.
class MemberExpr final : public Expr {
    friend class Sema;
    friend class Codegen;

    /// The base expression to access.
    Expr* m_base;

    /// The member declaration to access.
    const ValueDecl* m_member;

    /// If this is an arrow member access, i.e. one using the '->' operator 
    /// instead of the '.' operator.
    bool m_arrow;

public:
    MemberExpr(const SourceSpan& span, const QualType& type, Expr* base,
               const ValueDecl* member, bool arrow)
        : Expr(Kind::Member, span, type), m_base(std::move(base)),
          m_member(member), m_arrow(arrow) {}

    MemberExpr(const MemberExpr&) = delete;
    MemberExpr& operator = (const MemberExpr&) = delete;

    ~MemberExpr();

    /// Returns the base expression of this member access.
    const Expr* get_base() const { return m_base; }
    Expr* get_base() { return m_base; }

    /// Returns the declaration that this member access references.
    const ValueDecl* get_member() const { return m_member; }

    /// Returns true if this is an arrow '->' member access.
    bool is_arrow() const { return m_arrow; }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Represents a '?' ternary selection expression.
class TernaryExpr final : public Expr {
    friend class Sema;
    friend class Codegen;
    
    /// The condition expression of the ternary operator.
    Expr* m_cond;
    
    /// The expression to use if the condition is true.
    Expr* m_tval;

    /// The expression to use if the condition is false.
    Expr* m_fval;

public:
    TernaryExpr(const SourceSpan& span, const QualType& type, Expr* cond, 
                Expr* tval, Expr* fval)
        : Expr(Kind::Ternary, span, type), m_cond(cond), m_tval(tval), 
          m_fval(fval) {}

    TernaryExpr(const TernaryExpr&) = delete;
    TernaryExpr& operator = (const TernaryExpr&) = delete;

    ~TernaryExpr();

    /// Returns the condition expression of this operator.
    const Expr* get_cond() const { return m_cond; }
    Expr* get_cond() { return m_cond; }

    /// Returns the expression to be used if the condition of this operator
    /// is true.
    const Expr* get_true_value() const { return m_tval; }
    Expr* get_true_value() { return m_tval; }

    /// Returns the expression to be used if the condition of this operator
    /// is false.
    const Expr* get_false_value() const { return m_fval; }
    Expr* get_false_value() { return m_fval; }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

} // namespace scc

#endif // SCC_EXPR_H_

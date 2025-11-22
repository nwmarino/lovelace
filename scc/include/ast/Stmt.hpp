//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SCC_STMT_H_
#define SCC_STMT_H_

//
// This header file defines the statement nodes in the abstract syntax tree.
//

#include "ast/Expr.hpp"
#include "ast/Scope.hpp"
#include "core/Span.hpp"

#include <cassert>
#include <cstdint>
#include <memory>
#include <vector>

namespace scc {

/// Base class for all statement nodes in the abstract syntax tree.
class Stmt {
public:
    /// Possible kinds of C statements.
    enum Kind : uint32_t {
        Compound,
        Declaration,
        Expression,
        If,
        Return,
        Break,
        Continue,
        /*
        While,
        For,
        Case,
        Switch,
        */
    };

protected:
    /// The kind of statement this is.
    const Kind m_kind;

    /// The span of source code that this statement covers.
    const Span m_span;

public:
    Stmt(Kind kind, const Span& span) : m_kind(kind), m_span(span) {}

    Stmt(const Stmt&) = delete;
    Stmt& operator = (const Stmt&) = delete;

    virtual ~Stmt() = default;

    /// Returns the kind of statement this is.
    Kind kind() const { return m_kind; }

    /// Returns the span of source code this statement covers.
    const Span& span() const { return m_span; }

    /// Pretty-print this expression node to the output stream \p os.
    virtual void print(std::ostream& os) const = 0;
};

/// Represents a scoped list of statements enclosed by curly braces '{, }'.
class CompoundStmt final : public Stmt {
    /// The scope of this compound statement.
    std::unique_ptr<Scope> m_scope;

    /// The statements in this compound statement.
    std::vector<std::unique_ptr<Stmt>> m_stmts;

public:
    CompoundStmt(const Span& span, std::unique_ptr<Scope> scope, 
                 std::vector<std::unique_ptr<Stmt>>& stmts)
        : Stmt(Kind::Compound, span), m_scope(std::move(scope)), 
          m_stmts(std::move(stmts)) {}

    CompoundStmt(const CompoundStmt&) = delete;
    CompoundStmt& operator = (const CompoundStmt&) = delete;

    /// Returns the scope node of this compound statement.
    const Scope* get_scope() const { return m_scope.get(); }
    Scope* get_scope() { return m_scope.get(); }

    /// Returns the number of statements in this compound statement.
    uint32_t num_stmts() const { return m_stmts.size(); }

    /// Returns true if this compound has any statements in it.
    bool has_stmts() const { return !m_stmts.empty(); }

    /// Returns the statement at position \p i of this compound statement.
    const Stmt* get_stmt(uint32_t i) const {
        assert(i < num_stmts() && "index out of bounds!");
        return m_stmts[i].get();
    }

    Stmt* get_stmt(uint32_t i) {
        return const_cast<Stmt*>(
            static_cast<const CompoundStmt*>(this)->get_stmt(i));
    }

    void print(std::ostream& os) const override;
};

/// Represents an inline declaration as part of a statement. This is generally
/// only used for local variable definitions.
class DeclStmt final : public Stmt {
    /// The nested declaration in this statement.
    std::unique_ptr<Decl> m_decl;

public:
    DeclStmt(const Span& span, std::unique_ptr<Decl> decl)
        : Stmt(Kind::Declaration, span), m_decl(std::move(decl)) {}
        
    DeclStmt(const DeclStmt&) = delete;
    DeclStmt& operator = (const DeclStmt&) = delete;

    /// Returns the declaration that is part of this statement.
    const Decl* get_decl() const { return m_decl.get(); }
    Decl* get_decl() { return m_decl.get(); }

    void print(std::ostream& os) const override;
};

/// Represents an inline expression as part of a statement. This generally 
/// represents expressions that are standalone.
class ExprStmt final : public Stmt {
    /// The nested expression of this statement.
    std::unique_ptr<Expr> m_expr;
  
public:
    ExprStmt(const Span& span, std::unique_ptr<Expr> expr)
        : Stmt(Kind::Expression, span), m_expr(std::move(expr)) {}

    ExprStmt(const ExprStmt&) = delete;
    ExprStmt& operator = (const ExprStmt&) = delete;

    /// Returns the expression that is part of this statement.
    const Expr* get_expr() const { return m_expr.get(); }
    Expr* get_expr() { return m_expr.get(); }

    void print(std::ostream& os) const override;
};

/// Represents an 'if' statement.
class IfStmt final : public Stmt {
    /// The condition of this if statement.
    std::unique_ptr<Expr> m_cond;

    /// The then clause of this if statement.
    std::unique_ptr<Stmt> m_then;

    /// The else clause of this if statement, if there is one.
    std::unique_ptr<Stmt> m_else;

public:
    IfStmt(const Span& span, std::unique_ptr<Expr> cond, 
           std::unique_ptr<Stmt> then, std::unique_ptr<Stmt> els)
        : Stmt(Stmt::If, span), m_cond(std::move(cond)), m_then(std::move(then)),
          m_else(std::move(els)) {}

    IfStmt(const IfStmt&) = delete;
    IfStmt& operator = (const IfStmt&) = delete;

    /// Returns the condition expression of this if statement.
    const Expr* get_cond() const { return m_cond.get(); }
    Expr* get_cond() { return m_cond.get(); }

    /// Returns the then clause of this if statement.
    const Stmt* get_then() const { return m_then.get(); }
    Stmt* get_then() { return m_then.get(); }

    /// Returns true if this if statement contains an else clause.
    bool has_else() const { return m_else != nullptr; }

    /// Returns the else clause of this if statement, if there is one, and
    /// `nullptr` otherwise.
    const Stmt* get_else() const { return has_else() ? m_else.get() : nullptr; }
    Stmt* get_else() { return has_else() ? m_else.get() : nullptr; }

    void print(std::ostream& os) const override;
};

/// Represents a 'return' statement.
class ReturnStmt final : public Stmt {
    /// The expression that this statement returns, if there is one.
    std::unique_ptr<Expr> m_expr;

public:
    ReturnStmt(const Span& span, std::unique_ptr<Expr> expr)
        : Stmt(Kind::Return, span), m_expr(std::move(expr)) {}

    ReturnStmt(const ReturnStmt&) = delete;
    ReturnStmt& operator = (const ReturnStmt&) = delete;

    /// Returns true if this return statement returns a value.
    bool has_expr() const { return m_expr != nullptr; }

    /// Returns the expression that this statement returns, if there is one,
    /// and 'nullptr' otherwise.
    const Expr* get_expr() const { return has_expr() ? m_expr.get() : nullptr; }
    Expr* get_expr() { return has_expr() ? m_expr.get() : nullptr; }

    void print(std::ostream& os) const override;
};

/// Represents a 'break' statement.
class BreakStmt final : public Stmt {
public:
    BreakStmt(const Span& span) : Stmt(Kind::Break, span) {}

    BreakStmt(const BreakStmt&) = delete;
    BreakStmt& operator = (const BreakStmt&) = delete;

    void print(std::ostream& os) const override;
};

/// Represents a 'continue' statement.
class ContinueStmt final : public Stmt {
public:
    ContinueStmt(const Span& span) : Stmt(Kind::Continue, span) {}

    ContinueStmt(const ContinueStmt&) = delete;
    ContinueStmt& operator = (const ContinueStmt&) = delete;

    void print(std::ostream& os) const override;
};

/*

class WhileStmt final : public Stmt {
    std::unique_ptr<Expr> m_cond;
    std::unique_ptr<Expr> m_body;
};

class ForStmt final : public Stmt {
    std::unique_ptr<Expr> m_init;
    std::unique_ptr<Expr> m_cond;
    std::unique_ptr<Expr> m_step;
    std::unique_ptr<Stmt> m_body;
};

class CaseStmt final : public Stmt {
    std::unique_ptr<Expr> m_case;
    std::unique_ptr<Stmt> m_body;
};

class SwitchStmt final : public Stmt {
    std::unique_ptr<Expr> m_match;
    std::vector<std::unique_ptr<CaseStmt>> m_cases;
};

*/

} // namespace scc

#endif // SCC_STMT_H_

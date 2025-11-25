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
#include "core/SourceSpan.hpp"

#include <cassert>
#include <cstdint>
#include <vector>

namespace scc {

using std::ostream;
using std::vector;

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
        While,
        For,
        Case,
        Switch,
    };

protected:
    /// The kind of statement this is.
    const Kind m_kind;

    /// The span of source code that this statement covers.
    SourceSpan m_span;

public:
    Stmt(Kind kind, const SourceSpan& span) : m_kind(kind), m_span(span) {}

    virtual ~Stmt() = default;

    /// Returns the kind of statement this is.
    Kind get_kind() const { return m_kind; }

    /// Returns the span of source code that this statement covers.
    const SourceSpan& get_span() const { return m_span; }
    SourceSpan& get_span() { return m_span; }
    
    /// Returns the location in source code that this statement starts at.
    const SourceLocation& get_starting_loc() const { return m_span.start; }
    SourceLocation& get_starting_loc() { return m_span.start; }

    /// Returns the location in source code that this statement ends at.
    const SourceLocation& get_ending_loc() const { return m_span.end; }
    SourceLocation& get_ending_loc() { return m_span.end; }

    /// Accept some visitor class \p visitor to access this node.
    virtual void accept(Visitor& visitor) = 0;

    /// Pretty-print this expression node to the output stream \p os.
    virtual void print(ostream& os) const = 0;
};

/// Represents a list of statements enclosed by curly braces '{, }'.
class CompoundStmt final : public Stmt {
    friend class Sema;
    friend class Codegen;

    /// The statements in this compound statement.
    vector<Stmt*> m_stmts;

public:
    CompoundStmt(const SourceSpan& span, const vector<Stmt*>& stmts)
        : Stmt(Kind::Compound, span), m_stmts(stmts) {}

    CompoundStmt(const CompoundStmt&) = delete;
    CompoundStmt& operator = (const CompoundStmt&) = delete;

    ~CompoundStmt();

    /// Returns the number of statements in this compound statement.
    uint32_t num_stmts() const { return m_stmts.size(); }

    /// Returns true if this compound is empty, i.e. has no statements.
    bool empty() const { return m_stmts.empty(); }

    /// Returns the statements in this compound.
    const vector<Stmt*>& get_stmts() const { return m_stmts; }
    vector<Stmt*>& get_stmts() { return m_stmts; }

    /// Set the statement list of this compound to \p stmts.
    void set_stmts(const vector<Stmt*>& stmts) { m_stmts = stmts; }

    /// Returns the statement at position \p i of this compound statement.
    const Stmt* get_stmt(uint32_t i) const {
        assert(i < num_stmts() && "index out of bounds!");
        return m_stmts[i];
    }

    Stmt* get_stmt(uint32_t i) {
        return const_cast<Stmt*>(
            static_cast<const CompoundStmt*>(this)->get_stmt(i));
    }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Represents a series of inline declaration as part of a statement.
class DeclStmt final : public Stmt {
    friend class Sema;
    friend class Codegen;

    /// The nested declarations in this statement.
    vector<const Decl*> m_decls;

public:
    DeclStmt(const SourceSpan& span, const vector<const Decl*>& decls)
        : Stmt(Kind::Declaration, span), m_decls(decls) {}
        
    DeclStmt(const DeclStmt&) = delete;
    DeclStmt& operator = (const DeclStmt&) = delete;

    /// Returns the number of declarations in this statement.
    uint32_t num_decls() const { return m_decls.size(); }

    /// Returns the list of declarations in this statement.
    const vector<const Decl*>& get_decls() const { return m_decls; }
    vector<const Decl*>& get_decls() { return m_decls; }

    /// Returns the first declaration that is part of this statement.
    const Decl* get_decl() const { 
        return m_decls.empty() ? nullptr : m_decls.front();  
    }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Represents an inline expression as part of a statement. This generally 
/// represents expressions that are standalone.
class ExprStmt final : public Stmt {
    friend class Sema;
    friend class Codegen;

    /// The nested expression of this statement.
    Expr* m_expr;
  
public:
    ExprStmt(const SourceSpan& span, Expr* expr)
        : Stmt(Kind::Expression, span), m_expr(expr) {}

    ExprStmt(const ExprStmt&) = delete;
    ExprStmt& operator = (const ExprStmt&) = delete;

    ~ExprStmt();

    /// Returns the expression that is part of this statement.
    const Expr* get_expr() const { return m_expr; }
    Expr* get_expr() { return m_expr; }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Represents an 'if' statement.
class IfStmt final : public Stmt {
    friend class Sema;
    friend class Codegen;

    /// The condition of this if statement.
    Expr* m_cond;

    /// The then clause of this if statement.
    Stmt* m_then;

    /// The else clause of this if statement, if there is one.
    Stmt* m_else;

public:
    IfStmt(const SourceSpan& span, Expr* cond, Stmt* then, Stmt* els)
        : Stmt(Stmt::If, span), m_cond(cond), m_then(then), m_else(els) {}

    IfStmt(const IfStmt&) = delete;
    IfStmt& operator = (const IfStmt&) = delete;

    ~IfStmt();

    /// Returns the condition expression of this if statement.
    const Expr* get_cond() const { return m_cond; }
    Expr* get_cond() { return m_cond; }

    /// Returns the then clause of this if statement.
    const Stmt* get_then() const { return m_then; }
    Stmt* get_then() { return m_then; }

    /// Returns true if this if statement contains an else clause.
    bool has_else() const { return m_else != nullptr; }

    /// Returns the else clause of this if statement, if there is one, and
    /// `nullptr` otherwise.
    const Stmt* get_else() const { return m_else; }
    Stmt* get_else() { return m_else; }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Represents a 'return' statement.
class ReturnStmt final : public Stmt {
    friend class Sema;
    friend class Codegen;

    /// The expression that this statement returns, if there is one.
    Expr* m_expr;

public:
    ReturnStmt(const SourceSpan& span, Expr* expr)
        : Stmt(Kind::Return, span), m_expr(expr) {}

    ReturnStmt(const ReturnStmt&) = delete;
    ReturnStmt& operator = (const ReturnStmt&) = delete;

    ~ReturnStmt();

    /// Returns true if this return statement returns a value.
    bool has_expr() const { return m_expr != nullptr; }

    /// Returns the expression that this statement returns, if there is one,
    /// and 'nullptr' otherwise.
    const Expr* get_expr() const { return m_expr; }
    Expr* get_expr() { return  m_expr; }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Represents a 'break' statement.
class BreakStmt final : public Stmt {
    friend class Sema;
    friend class Codegen;

public:
    BreakStmt(const SourceSpan& span) : Stmt(Kind::Break, span) {}

    BreakStmt(const BreakStmt&) = delete;
    BreakStmt& operator = (const BreakStmt&) = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Represents a 'continue' statement.
class ContinueStmt final : public Stmt {
    friend class Sema;
    friend class Codegen;

public:
    ContinueStmt(const SourceSpan& span) : Stmt(Kind::Continue, span) {}

    ContinueStmt(const ContinueStmt&) = delete;
    ContinueStmt& operator = (const ContinueStmt&) = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Represents a 'while' statement.
class WhileStmt final : public Stmt {
    friend class Sema;
    friend class Codegen;

    /// The condition of the while loop.
    Expr* m_cond;

    /// The body of the while loop, if it has one.
    Stmt* m_body;

public:
    WhileStmt(const SourceSpan& span, Expr* cond, Stmt* body)
        : Stmt(Kind::While, span), m_cond(cond), m_body(body) {}

    WhileStmt(const WhileStmt&) = delete;
    WhileStmt& operator = (const WhileStmt&) = delete;

    ~WhileStmt();

    /// Returns the condition expression of this while statement.
    const Expr* get_cond() const { return m_cond; }
    Expr* get_cond() { return m_cond; }
    
    /// Returns true if this while statement has body.
    bool has_body() const { return m_body != nullptr; }

    /// Returns the body of this while statement if it has one, and 'nullptr'
    /// otherwise.
    const Stmt* get_body() const { return m_body; }
    Stmt* get_body() { return m_body; }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Represents a 'for' statement.
class ForStmt final : public Stmt {
    friend class Sema;
    friend class Codegen;

    /// The initializing statement of the for loop, if there is one.
    Stmt* m_init;

    /// The stopping condition of the for loop, if there is one.
    Expr* m_cond;

    /// The increment or step expression of the for loop, if there is one.
    Expr* m_step;
    
    /// The body of the for loop, if there is one.
    Stmt* m_body;

public:
    ForStmt(const SourceSpan& span, Stmt* init, Expr* cond, Expr* step, 
            Stmt* body)
        : Stmt(Kind::For, span), m_init(init), m_cond(cond), m_step(step), 
          m_body(body) {}

    ForStmt(const ForStmt&) = delete;
    ForStmt& operator = (const ForStmt&) = delete;

    ~ForStmt();

    /// Returns true if this for loop has an initializing statement.
    bool has_init() const { return m_init != nullptr; }

    /// Returns the initializing statement of this for loop if it has one, and
    /// 'nullptr' otherwise.
    const Stmt* get_init() const { return m_init; }
    Stmt* get_init() { return m_init; }

    /// Returns true if this for loop has a stopping condition.
    bool has_cond() const { return m_cond != nullptr; }

    /// Returns the stopping condition of this for loop if it has one, and
    /// 'nullptr' otherwise.
    const Expr* get_cond() const { return m_cond; }
    Expr* get_cond() { return m_cond; }

    /// Returns true if this for loop has a stepping expression.
    bool has_step() const { return m_step != nullptr; }

    /// Returns the stepping expression of this for loop if it has one, and
    /// 'nullptr' otherwise.
    const Expr* get_step() const { return m_step; }
    Expr* get_step() { return m_step; }

    /// Returns true if this for loop has a body.
    bool has_body() const { return m_body != nullptr; }

    /// Returns the body of this for loop if it has one, and 'nullptr' otherwise.
    const Stmt* get_body() const { return m_body; }
    Stmt* get_body() { return m_body; }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Represents a 'case' statement.
class CaseStmt final : public Stmt {
    friend class Sema;
    friend class Codegen;

    /// The expression to match for this case.
    Expr* m_match;

    /// The body of this case, if it has one.
    Stmt* m_body;

public:
    CaseStmt(const SourceSpan& span, Expr* match, Stmt* body)
        : Stmt(Kind::Case, span), m_match(match), m_body(body) {}

    CaseStmt(const CaseStmt&) = delete;
    CaseStmt& operator = (const CaseStmt&) = delete;

    ~CaseStmt();

    /// Returns the expression to match for this case statement.
    const Expr* get_match() const { return m_match; }
    Expr* get_match() { return m_match; }

    /// Returns true if this case has a body.
    bool has_body() const { return m_body != nullptr; }
    
    /// Returns the body of this case statement.
    const Stmt* get_body() const { return m_body; }
    Stmt* get_body() { return m_body; }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Represents a 'switch' statement.
class SwitchStmt final : public Stmt {
    friend class Sema;
    friend class Codegen;
    
    /// The expression to match for this switch statement.
    Expr* m_match;

    /// The list of cases in this switch statement.
    vector<CaseStmt*> m_cases;

    /// The default case for this switch statement, if there is one.
    Stmt* m_default;

public:
    SwitchStmt(const SourceSpan& span, Expr* match, 
               const vector<CaseStmt*>& cases, Stmt* def)
        : Stmt(Kind::Switch, span), m_match(match), m_cases(cases), 
          m_default(def) {}

    SwitchStmt(const SwitchStmt&) = delete;
    SwitchStmt& operator = (const SwitchStmt&) = delete;
    
    ~SwitchStmt();

    /// Returns the number of cases in this switch statement.
    uint32_t num_cases() const { return m_cases.size(); }

    /// Returns the case at position \p i of this switch statement.
    const CaseStmt* get_case(uint32_t i) const {
        assert(i < num_cases() && "index out of bounds!");
        return m_cases[i];
    }

    CaseStmt* get_case(uint32_t i) {
        return const_cast<CaseStmt*>(
            static_cast<const SwitchStmt*>(this)->get_case(i));
    }

    /// Returns true if this switch statement has a default case.
    bool has_default() const { return m_default != nullptr; }

    /// Returns the default statement of this switch statement if there is one,
    /// and 'nullptr' otherwise.
    const Stmt* get_default() const { return m_default; }
    Stmt* get_default() { return m_default; }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

} // namespace scc

#endif // SCC_STMT_H_

#ifndef SCC_STMT_H_
#define SCC_STMT_H_

#include "Expr.hpp"
#include "Scope.hpp"
#include "../core/Span.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace scc {

class Stmt {
public:
    enum Kind : uint32_t {
        Compound,
        Declaration,
        Expression,
        If,
        Break,
        Continue,
        While,
        For,
        Case,
        Switch,
        Return,
    };

protected:
    const Kind m_kind;
    const Span m_span;

public:
    Stmt(Kind kind, const Span& span) : m_kind(kind), m_span(span) {}

    Stmt(const Stmt&) = delete;
    Stmt& operator = (const Stmt&) = delete;

    virtual ~Stmt() = 0;

    /// Returns the kind of statement this is.
    Kind get_kind() const { return m_kind; }

    /// Returns the span of source code this declaration covers.
    const Span& get_span() const { return m_span; }
};

class CompoundStmt final : public Stmt {
    std::unique_ptr<Scope> m_scope;
    std::vector<std::unique_ptr<Stmt>> m_stmts;
};

class DeclStmt final : public Stmt {
    std::unique_ptr<Decl> m_decl;
};

class ExprStmt final : public Stmt {
    std::unique_ptr<Expr> m_expr;
};

class IfStmt final : public Stmt {
    std::unique_ptr<Expr> m_cond;
    std::unique_ptr<Stmt> m_then;
    std::unique_ptr<Stmt> m_else;
};

class BreakStmt final : public Stmt {

};

class ContinueStmt final : public Stmt {

};

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

class ReturnStmt final : public Stmt {
    std::unique_ptr<Expr> m_expr;
};

} // namespace scc

#endif // SCC_STMT_H_

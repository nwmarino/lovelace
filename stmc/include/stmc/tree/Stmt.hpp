//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef STATIM_STMT_H_
#define STATIM_STMT_H_

#include "stmc/tree/Context.hpp"
#include "stmc/tree/Visitor.hpp"
#include "stmc/types/SourceSpan.hpp"
#include <cassert>

namespace stm {

class Decl;
class Expr;
class Scope;

/// Base class for all statement nodes in the abstract syntax tree (AST).
class Stmt {
protected:
    // The span of source code which this statement covers.
    const SourceSpan m_span;

    Stmt(SourceSpan span) : m_span(span) {}

public:
    virtual ~Stmt() = default;

    Stmt(const Stmt&) = delete;
    void operator=(const Stmt&) = delete;
    
    Stmt(Stmt&&) noexcept = delete;
    void operator=(Stmt&&) noexcept = delete;

    virtual void accept(Visitor& visitor) = 0;

    SourceSpan get_span() const { return m_span; }
};

/// Represents an inline assembly 'asm' statement.
class AsmStmt final : public Stmt {
    // The inline assembly string.
    string m_asms;

    // The list of input constraints.
    vector<string> m_ins;

    // The list of output constraints.
    vector<string> m_outs;

    // The list of read/write expression arguments.
    vector<Expr*> m_args;

    // The list of register/memory clobbers.
    vector<string> m_clobbers;

    // If this inline assembly is volatile, i.e. should not be modified.
    bool m_volatile;

    AsmStmt(SourceSpan span, const string& asms, const vector<string>& ins, 
            const vector<string>& outs, const vector<Expr*>& args, 
            const vector<string>& clobbers, bool is_volatile)
      : Stmt(span), m_asms(asms), m_ins(ins), m_outs(outs), m_args(args), 
        m_clobbers(clobbers), m_volatile(is_volatile) {}

public:
    static AsmStmt* create(Context& ctx, SourceSpan span, const string& asms,
                           const vector<string>& ins, const vector<string>& outs,
                           const vector<Expr*>& args, 
                           const vector<string>& clobbers, bool is_volatile);

    ~AsmStmt() override;

    AsmStmt(const AsmStmt&) = delete;
    void operator=(const AsmStmt&) = delete;

    AsmStmt(AsmStmt&&) noexcept = delete;
    void operator=(AsmStmt&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    const string& get_assembly_string() const { return m_asms; }
    string& get_assembly_string() { return m_asms; }

    uint32_t num_input_constraints() const { return m_ins.size(); }
    bool has_input_constraints() const { return !m_ins.empty(); }

    const vector<string>& get_input_constraints() const { return m_ins; }
    vector<string>& get_input_constraints() { return m_ins; }

    uint32_t num_output_constraints() const { return m_outs.size(); }
    bool has_output_constraints() const { return !m_outs.empty(); }

    const vector<string>& get_output_constraints() const { return m_outs; }
    vector<string>& get_output_constraints() { return m_outs; }

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

    uint32_t num_clobbers() const { return m_clobbers.size(); }
    bool has_clobbers() const { return !m_clobbers.empty(); }

    const vector<string>& get_clobbers() const { return m_clobbers; }
    vector<string>& get_clobbers() { return m_clobbers; }

    void set_is_volatile(bool is_volatile = true) { m_volatile = is_volatile; }
    bool is_volatile() const { return m_volatile; }
};

/// Represents a list of statement enclosed by curly braces '{, }'.
class BlockStmt final : public Stmt {
    Scope* m_scope;
    vector<Stmt*> m_stmts;

    BlockStmt(SourceSpan span, Scope* scope, const vector<Stmt*>& stmts)
      : Stmt(span), m_scope(scope), m_stmts(stmts) {}

public:
    static BlockStmt* create(Context& ctx, SourceSpan span, Scope* scope,
                             const vector<Stmt*>& stmts);

    ~BlockStmt() override;

    BlockStmt(const BlockStmt&) = delete;
    void operator=(const BlockStmt&) = delete;

    BlockStmt(BlockStmt&&) noexcept = delete;
    void operator=(BlockStmt&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    const Scope* get_scope() const { return m_scope; }
    Scope* get_scope() { return m_scope; }

    uint32_t num_stmts() const { return m_stmts.size(); }
    bool has_stmts() const { return !m_stmts.empty(); }

    void set_stmts(const vector<Stmt*>& stmts) { m_stmts = stmts; }
    const vector<Stmt*>& get_stmts() const { return m_stmts; }
    vector<Stmt*>& get_stmts() { return m_stmts; }

    const Stmt* get_stmt(uint32_t i) const {
        assert(i < num_stmts() && "index out of bounds!");
        return m_stmts[i];
    }

    Stmt* get_stmt(uint32_t i) {
        assert(i < num_stmts() && "index out of bounds!");
        return m_stmts[i];
    }
};

/// Represents an inline, local declaration as a statement.
class DeclStmt final : public Stmt {
    Decl* m_decl;

    DeclStmt(SourceSpan span, Decl* decl) : Stmt(span), m_decl(decl) {}

public:
    static DeclStmt* create(Context& ctx, Decl* decl);

    ~DeclStmt() override;

    DeclStmt(const DeclStmt&) = delete;
    void operator=(const DeclStmt&) = delete;

    DeclStmt(DeclStmt&&) noexcept = delete;
    void operator=(DeclStmt&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    const Decl* get_decl() const { return m_decl; }
    Decl* get_decl() { return m_decl; }
};

/// Represents a 'ret' statement.
class RetStmt final : public Stmt {
    friend class SemanticAnalysis;

    Expr* m_expr;

    RetStmt(SourceSpan span, Expr* expr) : Stmt(span), m_expr(expr) {}

public:
    static RetStmt* create(Context& ctx, SourceSpan span, Expr* expr);

    ~RetStmt() override;

    RetStmt(const RetStmt&) = delete;
    void operator=(const RetStmt&) = delete;

    RetStmt(RetStmt&&) noexcept = delete;
    void operator=(RetStmt&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    bool has_expr() const { return m_expr != nullptr; }

    const Expr* get_expr() const { return m_expr; }
    Expr* get_expr() { return m_expr; }
};

/// Represents an 'if' statement.
class IfStmt final : public Stmt {
    Expr* m_cond;
    Stmt* m_then;
    Stmt* m_else;

    IfStmt(SourceSpan span, Expr* cond, Stmt* then, Stmt* els)
      : Stmt(span), m_cond(cond), m_then(then), m_else(els) {}

public:
    static IfStmt* create(Context& ctx, SourceSpan span, Expr* cond, 
                          Stmt* then, Stmt* els);

    ~IfStmt() override;

    IfStmt(const IfStmt&) = delete;
    void operator=(const IfStmt&) = delete;

    IfStmt(IfStmt&&) noexcept = delete;
    void operator=(IfStmt&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    const Expr* get_cond() const { return m_cond; }
    Expr* get_cond() { return m_cond; }

    const Stmt* get_then() const { return m_then; }
    Stmt* get_then() { return m_then; }

    bool has_else() const { return m_else != nullptr; }

    const Stmt* get_else() const { return m_else; }
    Stmt* get_else() { return m_else; }
};

/// Represents a 'while' statement.
class WhileStmt final : public Stmt {
    Expr* m_cond;
    Stmt* m_body;

    WhileStmt(SourceSpan span, Expr* cond, Stmt* body)
      : Stmt(span), m_cond(cond), m_body(body) {}

public:
    static WhileStmt* create(Context& ctx, SourceSpan span, Expr* cond, 
                             Stmt* body);

    ~WhileStmt() override;

    WhileStmt(const WhileStmt&) = delete;
    void operator=(const WhileStmt&) = delete;

    WhileStmt(WhileStmt&&) noexcept = delete;
    void operator=(WhileStmt&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    const Expr* get_cond() const { return m_cond; }
    Expr* get_cond() { return m_cond; }

    bool has_body() const { return m_body != nullptr; }

    const Stmt* get_body() const { return m_body; }
    Stmt* get_body() { return m_body; }
};

/// Represents a 'break' statement.
class BreakStmt final : public Stmt {
    BreakStmt(SourceSpan span) : Stmt(span) {}

public:
    static BreakStmt* create(Context& ctx, SourceSpan span);
    
    ~BreakStmt() = default;

    BreakStmt(const BreakStmt&) = delete;
    void operator=(const BreakStmt&) = delete;

    BreakStmt(BreakStmt&&) noexcept = delete;
    void operator=(BreakStmt&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }
};

/// Represents a 'continue' statement.
class ContinueStmt final : public Stmt {
    ContinueStmt(SourceSpan span) : Stmt(span) {}

public:
    static ContinueStmt* create(Context& ctx, SourceSpan span);

    ~ContinueStmt() = default;

    ContinueStmt(const ContinueStmt&) = delete;
    void operator=(const ContinueStmt&) = delete;

    ContinueStmt(ContinueStmt&&) noexcept = delete;
    void operator=(ContinueStmt&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }
};

} // namespace stm

#endif // STATIM_STMT_H_

//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef STATIM_SEMANTIC_ANALYSIS_H_
#define STATIM_SEMANTIC_ANALYSIS_H_

#include "stmc/core/Diagnostics.hpp"
#include "stmc/core/Options.hpp"
#include "stmc/tree/Decl.hpp"
#include "stmc/tree/Expr.hpp"
#include "stmc/tree/Visitor.hpp"

namespace stm {

/// A syntax tree analysis pass that performs semantics-related checks, i.e.
/// type checking, implicit cast injections, control flow constructs, language
/// rule enforcement, etc.
class SemanticAnalysis final : public Visitor {
    /// Different kinds of loops.
    enum Loop : uint32_t {
        None = 0,
        While,
    };

    /// Possible modes for a type check.
    enum TypeCheckMode : uint32_t {
        Explicit,
        Loose,
        AllowImplicit,
    };

    /// Possible results of a type check.
    enum TypeCheckResult : uint32_t {
        Match,
        Mismatch,
        Cast,
    };
    
    Diagnostics& m_diags;
    Options& m_options;
    
    Context* m_context = nullptr;
    FunctionDecl* m_function = nullptr;
    Loop m_loop = None;

    TypeCheckResult type_check(const TypeUse& actual, const TypeUse& expected, 
                               TypeCheckMode mode = AllowImplicit) const;

public:
    SemanticAnalysis(Diagnostics& diags, Options& options);

    void visit(TranslationUnitDecl& node) override;
    void visit(VariableDecl& node) override;
    void visit(FunctionDecl& node) override;
    
    void visit(AsmStmt& node) override;
    void visit(BlockStmt& node) override;
    void visit(DeclStmt& node) override;
    void visit(RetStmt& node) override;
    void visit(IfStmt& node) override;
    void visit(WhileStmt& node) override;
    void visit(BreakStmt& node) override;
    void visit(ContinueStmt& node) override;

    void visit(BinaryOp& node) override;
    void visit(UnaryOp& node) override;
    void visit(CastExpr& node) override;
    void visit(ParenExpr& node) override;
    void visit(AccessExpr& node) override;
    void visit(SubscriptExpr& node) override;
    void visit(DeclRefExpr& node) override;
    void visit(CallExpr& node) override;
};

} // namespace stm

#endif // STATIM_SEMANTIC_ANALYSIS_H_

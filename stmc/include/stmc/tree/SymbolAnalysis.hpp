//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef STATIM_SYMBOL_ANALYSIS_H_
#define STATIM_SYMBOL_ANALYSIS_H_

#include "stmc/core/Diagnostics.hpp"
#include "stmc/core/Options.hpp"
#include "stmc/tree/Scope.hpp"
#include "stmc/tree/Visitor.hpp"

namespace stm {

/// A syntax tree analysis pass that performs symbol-related checks, i.e.
/// name resolution, deferred symbol resolution, load resolves, etc.
class SymbolAnalysis final : public Visitor {
    Diagnostics& m_diags;
    Options& m_options;

    const Scope* m_scope = nullptr;

public:
    SymbolAnalysis(Diagnostics& diags, Options& options);

    void visit(TranslationUnitDecl& node) override;
    void visit(VariableDecl& node) override;
    void visit(FunctionDecl& node) override;

    void visit(AsmStmt& node) override;
    void visit(BlockStmt& node) override;
    void visit(DeclStmt& node) override;
    void visit(RetStmt& node) override;
    void visit(IfStmt& node) override;
    void visit(WhileStmt& node) override;
    
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

#endif // STATIM_SYMBOL_ANALYSIS_H_

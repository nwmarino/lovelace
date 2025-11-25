//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SCC_SEMA_H_
#define SCC_SEMA_H_

//
// This header file declares the semantic analysis (sema) class which uses the
// visitor design pattern to perform a traversal over an entire syntax tree.
//
// Semantic analysis performs operations like type checking, cast injection,
// scope tree construction, and other IR simplifying actions.
// 

#include "ast/Decl.hpp"
#include "ast/Visitor.hpp"

#include <cstdint>

namespace scc {

class Sema final : public Visitor {
    /// Different kinds of loops in C. Used for checking things like break or
    /// continue statement locations, invariants, etc.
    enum LoopKind : uint32_t {
        None,
        While,
        For,
        Do,
    };
    
    TranslationUnitDecl* m_unit = nullptr;
    FunctionDecl* m_fn = nullptr;
    TypeContext& m_tctx;
    DeclContext& m_dctx;
    LoopKind m_loop = None;

public:
    Sema(TranslationUnitDecl* unit) 
        : m_unit(unit), m_tctx(unit->get_context()), m_dctx(*unit) {}

    Sema(const Sema&) = delete;
    Sema& operator = (const Sema&) = delete;

    void visit(TranslationUnitDecl &node) override;
    void visit(VariableDecl &node) override;
    void visit(ParameterDecl &node) override;
    void visit(FunctionDecl &node) override;
    void visit(FieldDecl &node) override;
    void visit(TypedefDecl &node) override;
    void visit(RecordDecl &node) override;
    void visit(EnumVariantDecl &node) override;
    void visit(EnumDecl &node) override;

    void visit(CompoundStmt &node) override;
    void visit(DeclStmt &node) override;
    void visit(ExprStmt &node) override;
    void visit(IfStmt &node) override;
    void visit(ReturnStmt &node) override;
    void visit(BreakStmt &node) override;
    void visit(ContinueStmt &node) override;
    void visit(WhileStmt &node) override;
    void visit(ForStmt &node) override;
    void visit(CaseStmt &node) override;
    void visit(SwitchStmt &node) override;

    void visit(BinaryExpr &node) override;
    void visit(UnaryExpr &node) override;
    void visit(ParenExpr &node) override;
    void visit(RefExpr &node) override;
    void visit(CallExpr &node) override;
    void visit(CastExpr &node) override;
    void visit(SizeofExpr &node) override;
    void visit(SubscriptExpr &node) override;
    void visit(MemberExpr &node) override;
    void visit(TernaryExpr &node) override;
};

} // namespace scc

#endif // SCC_SEMA_H_

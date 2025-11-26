//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SCC_CODEGEN_H_
#define SCC_CODEGEN_H_

//
// This header file declares a class which implements the visitor design 
// pattern over the abstract syntax tree for generating SPBE-IR code.
//

#include "scc/ast/Type.hpp"
#include "scc/ast/Visitor.hpp"

#include "spbe/graph/BasicBlock.hpp"
#include "spbe/graph/CFG.hpp"
#include "spbe/graph/Function.hpp"
#include "spbe/graph/InstrBuilder.hpp"
#include "spbe/graph/Value.hpp"

namespace scc {

class Codegen final : public Visitor {
    enum ValueContext : uint32_t {
        LValue,
        RValue,
    };

    enum Phase : uint32_t {
        Declare,
        Define,
    };

    TranslationUnitDecl* m_unit;

    spbe::CFG& m_graph;
    spbe::InstrBuilder m_builder;
    spbe::Function* m_func = nullptr;
    spbe::Value* m_temp = nullptr;
    spbe::Value* m_place = nullptr;
    spbe::BasicBlock* m_condition = nullptr;
    spbe::BasicBlock* m_merge = nullptr;

    Phase m_phase = Declare;
    ValueContext m_vctx = RValue;

    /// Returns true if this is the declare phase.
    bool is_declare_phase() const { return m_phase == Declare; }

    /// Returns true if this is the define phase.
    bool is_define_phase() const { return m_phase == Define; }

    /// Lower the type \p type to an SPBE equivelant.
    const spbe::Type* lower_type(const QualType& type);

    /// Inject a boolean comparison onto \p value if it isn't already a boolean
    /// typed value.
    spbe::Value* inject_bool_comparison(spbe::Value* value);

public:
    Codegen(TranslationUnitDecl* unit, spbe::CFG& graph);

    Codegen(const Codegen&) = delete;
    Codegen& operator = (const Codegen&) = delete;

    void visit(TranslationUnitDecl &node) override;
    void visit(VariableDecl &node) override;
    void visit(FunctionDecl &node) override;
    void visit(RecordDecl &node) override;

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

    void visit(IntegerLiteral& node) override;
    void visit(FPLiteral& node) override;
    void visit(CharLiteral& node) override;
    void visit(StringLiteral& node) override;
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

#endif // SCC_CODEGEN_H_

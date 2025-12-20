//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef STATIM_SPBE_CODEGEN_H_
#define STATIM_SPBE_CODEGEN_H_

#include "stmc/core/Diagnostics.hpp"
#include "stmc/core/Options.hpp"
#include "stmc/tree/Decl.hpp"
#include "stmc/tree/Expr.hpp"
#include "stmc/tree/Type.hpp"
#include "stmc/tree/Visitor.hpp"

#include "spbe/graph/CFG.hpp"
#include "spbe/graph/BasicBlock.hpp"
#include "spbe/graph/Function.hpp"
#include "spbe/graph/InstrBuilder.hpp"

#include <cstdint>

namespace stm {

class SPBECodegen final : public Visitor {
    enum ValueContext : uint32_t {
        LValue,
        RValue,
    };

    enum Phase : uint32_t {
        Declare,
        Define,
    };

    Diagnostics& m_diags;
    Options& m_options;

    Phase m_phase = Declare;
    ValueContext m_vctx = RValue;

    spbe::CFG& m_graph;
    spbe::InstrBuilder m_builder;
    spbe::Function* m_function = nullptr;
    spbe::Value* m_temp = nullptr;
    spbe::Value* m_place = nullptr;
    spbe::BasicBlock* m_cond = nullptr;
    spbe::BasicBlock* m_merge = nullptr;

    spbe::Function* get_intrinsic(const string& name, const spbe::Type* ret,
                                  const vector<const spbe::Type*>& params = {});

    const spbe::Type* lower_type_to_spbe(const Type* type);

    spbe::Value* inject_bool_comparison(spbe::Value* value);

    void declare_spbe_global(VariableDecl& decl);
    void define_spbe_global(VariableDecl& decl);

    void declare_spbe_function(FunctionDecl& decl);
    void define_spbe_function(FunctionDecl& decl);

    void declare_spbe_structure(StructDecl& decl);
    void define_spbe_structure(StructDecl& decl);

    void gen_binary_assign(BinaryOp& op);
    void gen_binary_add(BinaryOp& op);
    void gen_binary_mul(BinaryOp& op);
    void gen_binary_mod(BinaryOp& op);
    void gen_binary_bitwise_arithmetic(BinaryOp& op);
    void gen_binary_numerical_cmp(BinaryOp& op);
    void gen_binary_bitwise_cmp(BinaryOp& op);
    void gen_binary_logical_and(BinaryOp& op);
    void gen_binary_logical_or(BinaryOp& op);
        
    void gen_unary_addition(UnaryOp& op);
    void gen_unary_memory(UnaryOp& op);
    void gen_unary_negation(UnaryOp& op);
    void gen_unary_bitwise_not(UnaryOp& op);
    void gen_unary_logical_not(UnaryOp& op);

public:
    SPBECodegen(Diagnostics& diags, Options& options, spbe::CFG& graph);

    ~SPBECodegen() = default;

    SPBECodegen(const SPBECodegen&) = delete;
    void operator=(const SPBECodegen&) = delete;

    SPBECodegen(SPBECodegen&&) noexcept = delete;
    void operator=(SPBECodegen&&) noexcept = delete;

    void visit(TranslationUnitDecl& node) override;
    void visit(VariableDecl& node) override;
    void visit(FunctionDecl& node) override;
    void visit(StructDecl& node) override;

    void visit(AsmStmt& node) override;
    void visit(BlockStmt& node) override;
    void visit(DeclStmt& node) override;
    void visit(RetStmt& node) override; 
    void visit(IfStmt& node) override;
    void visit(WhileStmt& node) override;
    void visit(BreakStmt& node) override;
    void visit(ContinueStmt& node) override;

    void visit(BoolLiteral& node) override;
    void visit(IntegerLiteral& node) override;
    void visit(FPLiteral& node) override;
    void visit(CharLiteral& node) override;
    void visit(StringLiteral& node) override;
    void visit(NullLiteral& node) override;
    void visit(BinaryOp& node) override;
    void visit(UnaryOp& node) override;
    void visit(CastExpr& node) override;
    void visit(ParenExpr& node) override;
    void visit(SizeofExpr& node) override;
    void visit(AccessExpr& node) override;
    void visit(SubscriptExpr& node) override;
    void visit(DeclRefExpr& node) override;
    void visit(CallExpr& node) override;
};

} // namespace stm

#endif // STATIM_SPBE_CODEGEN_H_

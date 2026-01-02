//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_CODEGEN_H_
#define LOVELACE_CODEGEN_H_

//
//  This header file declares the Codegen class, whom instances thereof 
//  generate SPBE-IR code from a valid abstract syntax tree.
//

#include "lace/core/Options.hpp"
#include "lace/tree/Defn.hpp"
#include "lace/tree/Expr.hpp"
#include "lace/tree/Type.hpp"
#include "lace/tree/Visitor.hpp"

#include "spbe/graph/CFG.hpp"
#include "spbe/graph/BasicBlock.hpp"
#include "spbe/graph/Function.hpp"
#include "spbe/graph/InstrBuilder.hpp"

#include <cstdint>

namespace lace {

class Codegen final : public Visitor {
    /// The different contexts for a value.
    enum ValueContext : uint8_t {
        LValue, RValue
    };

    /// The different phases of code generation.
    enum Phase : uint8_t {
        Declare, Define
    };

    /// The different kinds of addition operators.
    enum class AddOp : uint8_t {
        Add, Sub
    };

    /// The different kinds of multiplication operators.
    enum class MulOp : uint8_t {
        Mul, Div, Mod
    };

    /// The different kinds of bitwise arithmetic operators.
    enum class BitwiseOp : uint8_t {
        And, Or, Xor
    };

    /// The different kinds of bitwise shift operators.
    enum class ShiftOp : uint8_t {
        LShift, RShift
    };

    /// The different kinds of numerical comparison operators.
    enum class CmpOp : uint8_t {
        Eq, NEq, Lt, LtEq, Gt, GtEq
    };

    const Options& m_options;

    Phase m_phase = Declare;
    ValueContext m_vctx = RValue;

    spbe::CFG& m_graph;
    spbe::InstrBuilder m_builder;
    spbe::Function* m_function = nullptr;
    spbe::Value* m_temp = nullptr;
    spbe::Value* m_place = nullptr;
    spbe::BasicBlock* m_cond = nullptr;
    spbe::BasicBlock* m_merge = nullptr;

    spbe::Function* get_intrinsic(const std::string& name, 
                                  const spbe::Type* ret,
                                  const std::vector<const spbe::Type*>& params = {});

    const spbe::Type* lower_type(const Type* type);

    spbe::Value* inject_bool_comparison(spbe::Value* value);

    void declare_ir_global(VariableDefn& node);
    void define_ir_global(VariableDefn& node);

    void declare_ir_function(FunctionDefn& node);
    void define_ir_function(FunctionDefn& node);

    void declare_ir_structure(StructDefn& node);
    void define_ir_structure(StructDefn& node);

    void codegen_assignment(BinaryOp& node);
    void codegen_addition(BinaryOp& node, AddOp op);
    void codegen_multiplication(BinaryOp& node, MulOp op);
    void codegen_bitwise_arithmetic(BinaryOp& node, BitwiseOp op);
    void codegen_bitwise_shift(BinaryOp& node, ShiftOp op);
    void codegen_numerical_comparison(BinaryOp& node, CmpOp op);
    void codegen_logical_and(BinaryOp& node);
    void codegen_logical_or(BinaryOp& node);

    void codegen_cast_integer(spbe::Value* value, const spbe::Type* dst, 
                              bool is_signed);
    void codegen_cast_float(spbe::Value* value, const spbe::Type* dst);
    void codegen_cast_array(spbe::Value* value, const spbe::Type* dst);
    void codegen_cast_pointer(spbe::Value* value, const spbe::Type* dst);

public:
    Codegen(const Options& options, spbe::CFG& graph);

    ~Codegen() = default;

    Codegen(const Codegen&) = delete;
    void operator=(const Codegen&) = delete;

    Codegen(Codegen&&) noexcept = delete;
    void operator=(Codegen&&) noexcept = delete;

    void visit(AST& ast) override;

    void visit(VariableDefn& node) override;
    void visit(FunctionDefn& node) override;
    void visit(StructDefn& node) override;

    void visit(AdapterStmt& node) override;
    void visit(BlockStmt& node) override;
    void visit(IfStmt& node) override;
    void visit(RestartStmt& node) override;
    void visit(RetStmt& node) override;
    void visit(StopStmt& node) override;
    void visit(UntilStmt& node) override;

    void visit(BoolLiteral& node) override;
    void visit(CharLiteral& node) override;
    void visit(IntegerLiteral& node) override;
    void visit(FloatLiteral& node) override;
    void visit(NullLiteral& node) override;
    void visit(StringLiteral& node) override;

    void visit(BinaryOp& node) override;
    void visit(UnaryOp& node) override;

    void visit(AccessExpr& node) override;
    void visit(CallExpr& node) override;
    void visit(CastExpr& node) override;
    void visit(ParenExpr& node) override;
    void visit(RefExpr& node) override;
    void visit(SizeofExpr& node) override;
    void visit(SubscriptExpr& node) override;
};

} // namespace lace

#endif // LOVELACE_CODEGEN_H_

//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_CODEGEN_H_
#define LOVELACE_CODEGEN_H_

//
//  This header file declares the Codegen class, whom instances thereof 
//  generate LIR code from a valid abstract syntax tree.
//

#include "lace/core/Options.hpp"
#include "lace/tree/Defn.hpp"
#include "lace/tree/Expr.hpp"
#include "lace/tree/Type.hpp"
#include "lace/tree/Visitor.hpp"

#include "lir/graph/BasicBlock.hpp"
#include "lir/graph/Builder.hpp"
#include "lir/graph/CFG.hpp"
#include "lir/graph/Function.hpp"
#include "lir/machine/Machine.hpp"

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
    const lir::Machine& m_mach;

    Phase m_phase = Declare;
    ValueContext m_vctx = RValue;

    lir::CFG& m_cfg;
    lir::Builder m_builder;
    lir::Function* m_function = nullptr;
    lir::Value* m_temp = nullptr;
    lir::Value* m_place = nullptr;
    lir::BasicBlock* m_cnd = nullptr;
    lir::BasicBlock* m_mrg = nullptr;

    lir::Function* get_intrinsic(const std::string& name, lir::Type* ret, 
                                 const std::vector<lir::Type*>& params = {});

    /// Lower the given |type| to an LIR equivelant, where applicable.
    lir::Type* lower_type(const QualType& type);

    /// Attempt to inject a boolean comparison unto the given |value|, such
    /// that the result is some form of comparison of a boolean type.
    lir::Value* inject_bool_comparison(lir::Value* value);

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

    void codegen_negate(UnaryOp& node);
    void codegen_bitwise_not(UnaryOp& node);
    void codegen_logical_not(UnaryOp& node);
    void codegen_address_of(UnaryOp& node);
    void codegen_dereference(UnaryOp& node);

    void codegen_cast_integer(lir::Value* value, lir::Type* dest, bool is_signed);
    void codegen_cast_float(lir::Value* value, lir::Type* dest);
    void codegen_cast_array(lir::Value* value, lir::Type* dest);
    void codegen_cast_pointer(lir::Value* value, lir::Type* dest);

public:
    Codegen(const Options& options, lir::CFG& cfg);

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

//
//  Copyright (c) 2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_LLVM_CODEGEN_H_
#define LOVELACE_LLVM_CODEGEN_H_

#include "lace/core/Options.hpp"
#include "lace/tree/AST.hpp"
#include "lace/tree/Defn.hpp"
#include "lace/tree/Expr.hpp"
#include "lace/tree/Stmt.hpp"
#include "lace/tree/Type.hpp"

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"

namespace lace {

class LLVMCodegen final {
    using AllocaTable = std::unordered_map<std::string, llvm::AllocaInst*>;

    const Options& options;
    const AST* ast;

    llvm::Module* mod;
    llvm::IRBuilder<> builder;
    llvm::Function* func = nullptr;

    llvm::BasicBlock* parent_cnd = nullptr;
    llvm::BasicBlock* parent_mrg = nullptr;

    AllocaTable allocas = {};

    enum AdditiveOp : uint8_t {
        Add, Sub,
    };

    enum MultiplicativeOp : uint8_t {
        Mul, Div, Mod,
    };

    enum BitwiseOp : uint8_t {
        And, Or, Xor,
    };

    enum BitshiftOp : uint8_t {
        Left, Right,
    };

    enum ComparisonOp : uint8_t {
        Eq, NEq, Lt, LtEq, Gt, GtEq,
    };

public:
    LLVMCodegen(const Options& options, const AST* ast, llvm::Module* mod);

    /// Finalize the code generation process.
    void finalize();

private:
    llvm::Type* to_llvm_type(const QualType& type);

    llvm::Value* inject_comparison(llvm::Value* value);

    void emit_initial_definition(const Defn* defn);
    void fill_existing_definition(const Defn* defn);

    llvm::Function* emit_initial_function(const FunctionDefn* defn);
    llvm::Function* fill_existing_function(const FunctionDefn* defn);

    llvm::StructType* emit_structure_shell(const StructDefn* defn);
    llvm::StructType* fill_existing_structure(const StructDefn* defn);

    llvm::GlobalVariable* emit_initial_global(const VariableDefn* defn);
    llvm::GlobalVariable* fill_existing_global(const VariableDefn* defn);

    llvm::AllocaInst* emit_local(const VariableDefn* defn);

    llvm::Value* emit_valued_expression(const Expr* expr);
    llvm::Value* emit_addressed_expression(const Expr* expr);

    llvm::Value* emit_addressed_access(const AccessExpr* expr);
    llvm::Value* emit_valued_access(const AccessExpr* expr);

    llvm::Value* emit_addressed_ref(const RefExpr* expr);
    llvm::Value* emit_valued_ref(const RefExpr* expr);

    llvm::Value* emit_addressed_subscript(const SubscriptExpr* expr);
    llvm::Value* emit_valued_subscript(const SubscriptExpr* expr);

    llvm::Value* emit_addressed_deref(const UnaryOp* expr);
    llvm::Value* emit_valued_deref(const UnaryOp* expr);

    llvm::Value* emit_assignment(const BinaryOp* expr);
    llvm::Value* emit_additive(const BinaryOp* expr, AdditiveOp op);
    llvm::Value* emit_multiplicative(const BinaryOp* expr, MultiplicativeOp op);
    llvm::Value* emit_bitwise(const BinaryOp* expr, BitwiseOp op);
    llvm::Value* emit_bitshift(const BinaryOp* expr, BitshiftOp op);
    llvm::Value* emit_comparison(const BinaryOp* expr, ComparisonOp op);
    llvm::Value* emit_logical_and(const BinaryOp* expr);
    llvm::Value* emit_logical_or(const BinaryOp* expr);

    llvm::Value* emit_negation(const UnaryOp* expr);
    llvm::Value* emit_bitwise_not(const UnaryOp* expr);
    llvm::Value* emit_logical_not(const UnaryOp* expr);
    llvm::Value* emit_address_of(const UnaryOp* expr);

    llvm::Value* emit_boolean_literal(const BoolLiteral* lit);
    llvm::Value* emit_integer_literal(const IntegerLiteral* lit);
    llvm::Value* emit_float_literal(const FloatLiteral* lit);
    llvm::Value* emit_char_literal(const CharLiteral* lit);
    llvm::Value* emit_null_literal(const NullLiteral* lit);
    llvm::Value* emit_string_literal(const StringLiteral* lit);

    llvm::Value* emit_cast_expr(const CastExpr* expr);
    llvm::Value* emit_call_expr(const CallExpr* expr);
    llvm::Value* emit_paren_expr(const ParenExpr* expr);
    llvm::Value* emit_sizeof_expr(const SizeofExpr* expr);

    llvm::Value* emit_cast_integers(llvm::Value* value, llvm::Type* dest, bool signedness);
    llvm::Value* emit_cast_floats(llvm::Value* value, llvm::Type* dest);
    llvm::Value* emit_cast_arrays(llvm::Value* value, llvm::Type* dest);
    llvm::Value* emit_cast_pointers(llvm::Value* value, llvm::Type* dest);

    void emit_statement(const Stmt* stmt);
    void emit_adapter(const AdapterStmt* stmt);
    void emit_block(const BlockStmt* stmt);
    void emit_if(const IfStmt* stmt);
    void emit_restart(const RestartStmt* stmt);
    void emit_return(const RetStmt* stmt);
    void emit_stop(const StopStmt* stmt);
    void emit_until(const UntilStmt* stmt);
};

} // namespace lace

#endif // LOVELACE_LLVM_CODEGEN_H_

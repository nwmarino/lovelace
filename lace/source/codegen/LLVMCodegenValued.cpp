//
//  Copyright (c) 2026 Nick Marino
//  All rights reserved.
//

#include "lace/codegen/LLVMCodegen.hpp"
#include "lace/tree/Expr.hpp"

#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Support/Casting.h"
#include "llvm/IR/DerivedTypes.h"

using namespace lace;

llvm::Value* LLVMCodegen::emit_valued_expression(const Expr* expr) {
    switch (expr->get_kind()) {
        case Expr::Bool:
            return emit_boolean_literal(static_cast<const BoolLiteral*>(expr));

        case Expr::Char:
            return emit_char_literal(static_cast<const CharLiteral*>(expr));

        case Expr::Integer:
            return emit_integer_literal(static_cast<const IntegerLiteral*>(expr));

        case Expr::Float:
            return emit_float_literal(static_cast<const FloatLiteral*>(expr));

        case Expr::Null:
            return emit_null_literal(static_cast<const NullLiteral*>(expr));

        case Expr::String:
            return emit_string_literal(static_cast<const StringLiteral*>(expr));

        case Expr::Binary: {
            const BinaryOp* op = static_cast<const BinaryOp*>(expr);
            switch (op->get_operator()) {
                case BinaryOp::Assign:
                    return emit_assignment(op);
                case BinaryOp::Add:
                    return emit_additive(op, AdditiveOp::Add);
                case BinaryOp::Sub:
                    return emit_additive(op, AdditiveOp::Sub);
                case BinaryOp::Mul:
                    return emit_multiplicative(op, MultiplicativeOp::Mul);
                case BinaryOp::Div:
                    return emit_multiplicative(op, MultiplicativeOp::Div);
                case BinaryOp::Mod:
                    return emit_multiplicative(op, MultiplicativeOp::Mod);
                case BinaryOp::And:
                    return emit_bitwise(op, BitwiseOp::And);
                case BinaryOp::Or:
                    return emit_bitwise(op, BitwiseOp::Or);
                case BinaryOp::Xor:
                    return emit_bitwise(op, BitwiseOp::Xor);
                case BinaryOp::LShift:
                    return emit_bitshift(op, BitshiftOp::Left);
                case BinaryOp::RShift:
                    return emit_bitshift(op, BitshiftOp::Right);
                case BinaryOp::LogicAnd:
                    return emit_logical_and(op);
                case BinaryOp::LogicOr:
                    return emit_logical_or(op);
                case BinaryOp::Eq:
                    return emit_comparison(op, ComparisonOp::Eq);
                case BinaryOp::NEq:
                    return emit_comparison(op, ComparisonOp::NEq);
                case BinaryOp::Lt:
                    return emit_comparison(op, ComparisonOp::Lt);
                case BinaryOp::LtEq:
                    return emit_comparison(op, ComparisonOp::LtEq);
                case BinaryOp::Gt:
                    return emit_comparison(op, ComparisonOp::Gt);
                case BinaryOp::GtEq:
                    return emit_comparison(op, ComparisonOp::GtEq);
                default:
                    assert(false && "invalid rvalue binary operator!");
            }
        }

        case Expr::Unary: {
            const UnaryOp* op = static_cast<const UnaryOp*>(expr);
            switch (op->get_operator()) {
                case UnaryOp::Negate:
                    return emit_negation(op);
                case UnaryOp::Not:
                    return emit_bitwise_not(op);
                case UnaryOp::LogicNot:
                    return emit_logical_not(op);
                case UnaryOp::AddressOf:
                    return emit_address_of(op);
                case UnaryOp::Dereference:
                    return emit_valued_deref(op);
                default:
                    assert(false && "invalid rvalue unary operator!");
            }
        }

        case Expr::Access:
            return emit_valued_access(static_cast<const AccessExpr*>(expr));

        case Expr::Call:
            return emit_call_expr(static_cast<const CallExpr*>(expr));

        case Expr::Cast:
            return emit_cast_expr(static_cast<const CastExpr*>(expr));

        case Expr::Paren:
            return emit_paren_expr(static_cast<const ParenExpr*>(expr));

        case Expr::Ref:
            return emit_valued_ref(static_cast<const RefExpr*>(expr));
        
        case Expr::Sizeof:
            return emit_sizeof_expr(static_cast<const SizeofExpr*>(expr));
        
        case Expr::Subscript:
            return emit_valued_subscript(static_cast<const SubscriptExpr*>(expr));
    }

    __builtin_unreachable();
}

llvm::Value* LLVMCodegen::emit_boolean_literal(const BoolLiteral* lit) {
    return llvm::ConstantInt::get(
        to_llvm_type(lit->get_type()), lit->get_value()
    );
}

llvm::Value* LLVMCodegen::emit_integer_literal(const IntegerLiteral* lit) {
    return llvm::ConstantInt::get(
        to_llvm_type(lit->get_type()), lit->get_value()
    );
}

llvm::Value* LLVMCodegen::emit_float_literal(const FloatLiteral* lit) {
    return llvm::ConstantFP::get(
        to_llvm_type(lit->get_type()), lit->get_value()
    );
}

llvm::Value* LLVMCodegen::emit_char_literal(const CharLiteral* lit) {
    return llvm::ConstantInt::get(
        llvm::Type::getInt8Ty(mod->getContext()), lit->get_value()
    );
}

llvm::Value* LLVMCodegen::emit_null_literal(const NullLiteral* lit) {
    return llvm::ConstantPointerNull::get(
        llvm::dyn_cast<llvm::PointerType>(to_llvm_type(lit->get_type()))
    );
}

llvm::Value* LLVMCodegen::emit_string_literal(const StringLiteral* lit) {
    return builder.CreateGlobalString(lit->get_value(), "", 0, mod);
}

llvm::Value* LLVMCodegen::emit_cast_expr(const CastExpr* expr) {
    llvm::Value* value = emit_valued_expression(expr->get_expr());
    assert(value);

    llvm::Type* source = value->getType();
    llvm::Type* dest = to_llvm_type(expr->get_type());

    if (source->isIntegerTy()) {
        return emit_cast_integers(value, dest, expr->get_type()->is_signed_integer());
    } else if (source->isFloatingPointTy()) {
        return emit_cast_floats(value, dest);
    } else if (source->isArrayTy()) {
        return emit_cast_arrays(value, dest);
    } else if (source->isPointerTy()) {
        return emit_cast_pointers(value, dest);
    }

    assert(false && "unsupported type cast!");
}

llvm::Value* LLVMCodegen::emit_call_expr(const CallExpr* expr) {
    llvm::Value* callee = emit_addressed_expression(expr->get_callee());
    assert(callee);

    std::vector<llvm::Value*> args(expr->num_args(), nullptr);
    for (uint32_t i = 0; i < expr->num_args(); ++i) {
        llvm::Value* arg = emit_valued_expression(expr->get_arg(i));
        assert(arg);
        args[i] = arg;
    }

    return builder.CreateCall({ 
        llvm::dyn_cast<llvm::FunctionType>(to_llvm_type(expr->get_callee()->get_type())), 
        callee 
    }, args);
}

llvm::Value* LLVMCodegen::emit_paren_expr(const ParenExpr* expr) {
    return emit_valued_expression(expr->get_expr());
}

llvm::Value* LLVMCodegen::emit_sizeof_expr(const SizeofExpr* expr) {
    const llvm::DataLayout& layout = mod->getDataLayout();
    return llvm::ConstantInt::get(
        to_llvm_type(expr->get_type()), 
        layout.getTypeSizeInBits(to_llvm_type(expr->get_target_type())) / 8
    );
}

llvm::Value* LLVMCodegen::emit_cast_integers(
        llvm::Value* value, llvm::Type* dest, bool signedness) {
    if (dest->isIntegerTy()) {
        const llvm::DataLayout& layout = mod->getDataLayout();
        const uint32_t source_size = layout.getTypeSizeInBits(value->getType());
        const uint32_t dest_size = layout.getTypeSizeInBits(dest);

        if (source_size == dest_size) {
            return value;
        } else if (source_size > dest_size) {
            return builder.CreateTrunc(value, dest);
        } else {
            if (signedness) {
                return builder.CreateSExt(value, dest);
            } else {
                return builder.CreateZExt(value, dest);
            }
        }
    } else if (dest->isFloatingPointTy()) {
        if (signedness) {
            return builder.CreateSIToFP(value, dest);
        } else {
            return builder.CreateUIToFP(value, dest);
        }
    } else if (dest->isPointerTy()) {
        return builder.CreateIntToPtr(value, dest);
    }

    assert(false && "invalid integer type cast!");
}

llvm::Value* LLVMCodegen::emit_cast_floats(llvm::Value* value, llvm::Type* dest) {
    if (dest->isIntegerTy()) {
        return builder.CreateFPToSI(value, dest);
    } else if (dest->isFloatingPointTy()) {
        const llvm::DataLayout& layout = mod->getDataLayout();
        const uint32_t source_size = layout.getTypeSizeInBits(value->getType());
        const uint32_t dest_size = layout.getTypeSizeInBits(dest);

        if (source_size == dest_size) {
            return value;
        } else if (source_size > dest_size) {
            return builder.CreateFPTrunc(value, dest);
        } else {
            return builder.CreateFPExt(value, dest);
        }
    }

    assert(false && "invalid floating point type cast!");
}

llvm::Value* LLVMCodegen::emit_cast_arrays(llvm::Value* value, llvm::Type* dest) {
    if (dest->isPointerTy())
        return builder.CreateBitCast(value, dest);

    assert(false && "invalid array type cast!");
}

llvm::Value* LLVMCodegen::emit_cast_pointers(llvm::Value* value, llvm::Type* dest) {
    if (dest->isIntegerTy()) {
        return builder.CreatePtrToInt(value, dest);
    } else if (dest->isPointerTy()) {
        return builder.CreateBitCast(value, dest);
    }

    assert(false && "invalid pointer type cast!");
}

llvm::Value* LLVMCodegen::emit_valued_access(const AccessExpr* expr) {
    llvm::Value* ptr = emit_addressed_expression(expr);
    llvm::Type* type = to_llvm_type(expr->get_type());

    return builder.CreateLoad(type, ptr);
}

llvm::Value* LLVMCodegen::emit_valued_ref(const RefExpr* expr) {
    llvm::Type* type = to_llvm_type(expr->get_type());

    assert(expr->get_defn());
    switch (expr->get_defn()->get_kind()) 
    {
        case Defn::Parameter: {
            auto it = allocas.find(expr->get_name());
            if (it == allocas.end())
                return nullptr;

            return builder.CreateLoad(type, it->second);
        }

        case Defn::Variable: {
            const VariableDefn* var = 
                static_cast<const VariableDefn*>(expr->get_defn());

            if (var->is_global()) {
                llvm::GlobalVariable* global = 
                    mod->getGlobalVariable(var->get_name(), true);
                assert(global && "global does not exist!");
                return builder.CreateLoad(type, global);
            } else {
                auto it = allocas.find(expr->get_name());
                if (it == allocas.end())
                    return nullptr;

                return builder.CreateLoad(type, it->second);
            }
        }

        case Defn::Variant: {
            const VariantDefn* var =
                static_cast<const VariantDefn*>(expr->get_defn());

            return llvm::ConstantInt::get(type, var->get_value());
        }

        default:
            break;
    }

    assert(false && "invalid rvalue reference!");
}

llvm::Value* LLVMCodegen::emit_valued_subscript(const SubscriptExpr* expr) {
    llvm::Value* ptr = emit_addressed_expression(expr);
    llvm::Type* type = to_llvm_type(expr->get_type());

    return builder.CreateLoad(type, ptr);
}

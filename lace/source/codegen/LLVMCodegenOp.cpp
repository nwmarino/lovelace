//
//  Copyright (c) 2026 Nick Marino
//  All rights reserved.
//

#include "lace/codegen/LLVMCodegen.hpp"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"

using namespace lace;

llvm::Value* LLVMCodegen::emit_assignment(const BinaryOp* expr) {
    llvm::Value* lhs = emit_addressed_expression(expr->get_lhs());
    assert(lhs);

    llvm::Value* rhs = emit_valued_expression(expr->get_rhs());
    assert(rhs);

    builder.CreateStore(rhs, lhs);
    return rhs;
}

llvm::Value* LLVMCodegen::emit_additive(const BinaryOp* expr, AdditiveOp op) {
    llvm::Value* lhs = emit_valued_expression(expr->get_lhs());
    assert(lhs);

    llvm::Value* rhs = emit_valued_expression(expr->get_rhs());
    assert(rhs);

    llvm::Type* lhs_type = lhs->getType();
    llvm::Type* rhs_type = rhs->getType();

    if (lhs_type->isPointerTy() && rhs_type->isIntegerTy()) {
        if (op == AdditiveOp::Sub)
            rhs = builder.CreateNeg(rhs, "tmp.pa.ineg");
        
        return builder.CreateInBoundsGEP(lhs_type, lhs, { rhs });        
    } else if (lhs_type->isIntegerTy()) {
        if (op == AdditiveOp::Add) {
            return builder.CreateAdd(lhs, rhs); 
        } else {
            return builder.CreateSub(lhs, rhs);
        }
    } else if (lhs_type->isFloatingPointTy()) {
        if (op == AdditiveOp::Add) {
            return builder.CreateFAdd(lhs, rhs);
        } else {
            return builder.CreateFSub(lhs, rhs);
        }
    }

    assert(false && "invalid type operands to additive operator!");
}

llvm::Value* LLVMCodegen::emit_multiplicative(const BinaryOp* expr, MultiplicativeOp op) {
    llvm::Value* lhs = emit_valued_expression(expr->get_lhs());
    assert(lhs);

    llvm::Value* rhs = emit_valued_expression(expr->get_rhs());
    assert(rhs);

    llvm::Type* lhs_type = lhs->getType();
    if (lhs_type->isIntegerTy()) {
        if (op == MultiplicativeOp::Mul)
            return builder.CreateMul(lhs, rhs);

        if (expr->get_type()->is_signed_integer()) {
            if (op == MultiplicativeOp::Div) {
                return builder.CreateSDiv(lhs, rhs);
            } else if (op == MultiplicativeOp::Mod) {
                return builder.CreateSRem(lhs, rhs);
            }
        } else {
            if (op == MultiplicativeOp::Div) {
                return builder.CreateUDiv(lhs, rhs);
            } else if (op == MultiplicativeOp::Mod) {
                return builder.CreateURem(lhs, rhs);
            }
        }
    } else if (lhs_type->isFloatingPointTy()) {
        if (op == MultiplicativeOp::Mul) {
            return builder.CreateFMul(lhs, rhs);
        } else if (op == MultiplicativeOp::Div) {
            return builder.CreateFDiv(lhs, rhs);
        } else if (op == MultiplicativeOp::Mod) {
            return builder.CreateFRem(lhs, rhs);
        }
    }

    assert(false && "invalid type operands to multiplicative operator!");
}

llvm::Value* LLVMCodegen::emit_bitwise(const BinaryOp* expr, BitwiseOp op) {
    llvm::Value* lhs = emit_valued_expression(expr->get_lhs());
    assert(lhs);

    llvm::Value* rhs = emit_valued_expression(expr->get_rhs());
    assert(rhs);

    assert(lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy() &&
        "invalid type operands to bitwise operator!");

    if (op == BitwiseOp::And) {
        return builder.CreateAnd(lhs, rhs);
    } else if (op == BitwiseOp::Or) {
        return builder.CreateOr(lhs, rhs);
    } else if (op == BitwiseOp::Xor) {
        return builder.CreateXor(lhs, rhs);
    }

    __builtin_unreachable();
}

llvm::Value* LLVMCodegen::emit_bitshift(const BinaryOp* expr, BitshiftOp op) {
    llvm::Value* lhs = emit_valued_expression(expr->get_lhs());
    assert(lhs);

    llvm::Value* rhs = emit_valued_expression(expr->get_rhs());
    assert(rhs);

    assert(lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy() &&
        "invalid type operands to bitshift operator!");

    if (op == BitshiftOp::Left) {
        return builder.CreateShl(lhs, rhs);
    } else if (op == BitshiftOp::Right) {
        if (expr->get_type()->is_signed_integer()) {
            return builder.CreateAShr(lhs, rhs);
        } else {
            return builder.CreateLShr(lhs, rhs);
        }
    }

    __builtin_unreachable();
}

llvm::Value* LLVMCodegen::emit_comparison(const BinaryOp* expr, ComparisonOp op) {
    llvm::Value* lhs = emit_valued_expression(expr->get_lhs());
    assert(lhs);

    llvm::Value* rhs = emit_valued_expression(expr->get_rhs());
    assert(rhs);

    const QualType& type = expr->get_lhs()->get_type();
    switch (op) 
    {
        case ComparisonOp::Eq:
            if (type->is_integer() || type->is_pointer()) {
               return builder.CreateICmpEQ(lhs, rhs);
            } else if (type->is_floating_point()) {
               return builder.CreateFCmpOEQ(lhs, rhs);
            }

        case ComparisonOp::NEq:
            if (type->is_integer() || type->is_pointer()) {
               return builder.CreateICmpNE(lhs, rhs);
            } else if (type->is_floating_point()) {
               return builder.CreateFCmpONE(lhs, rhs);
            }

        case ComparisonOp::Lt:
            if (type->is_signed_integer() || type->is_pointer()) {
               return builder.CreateICmpSLT(lhs, rhs);
            } else if (type->is_unsigned_integer()) {
               return builder.CreateICmpULT(lhs, rhs);
            } else if (type->is_floating_point()) {
               return builder.CreateFCmpOLT(lhs, rhs);
            }

        case ComparisonOp::LtEq:
            if (type->is_signed_integer() || type->is_pointer()) {
               return builder.CreateICmpSLE(lhs, rhs);
            } else if (type->is_unsigned_integer()) {
               return builder.CreateICmpULE(lhs, rhs);
            } else if (type->is_floating_point()) {
               return builder.CreateFCmpOLE(lhs, rhs);
            }

        case ComparisonOp::Gt:
            if (type->is_signed_integer() || type->is_pointer()) {
               return builder.CreateICmpSGT(lhs, rhs);
            } else if (type->is_unsigned_integer()) {
               return builder.CreateICmpUGT(lhs, rhs);
            } else if (type->is_floating_point()) {
               return builder.CreateFCmpOGT(lhs, rhs);
            }

        case ComparisonOp::GtEq:
            if (type->is_signed_integer() || type->is_pointer()) {
               return builder.CreateICmpSGE(lhs, rhs);
            } else if (type->is_unsigned_integer()) {
               return builder.CreateICmpUGE(lhs, rhs);
            } else if (type->is_floating_point()) {
               return builder.CreateFCmpOGE(lhs, rhs);
            }
    }

    __builtin_unreachable();
}

llvm::Value* LLVMCodegen::emit_logical_and(const BinaryOp* expr) {
    llvm::BasicBlock* rgt = llvm::BasicBlock::Create(mod->getContext(), "land.rgt");
    llvm::BasicBlock* mrg = llvm::BasicBlock::Create(mod->getContext(), "land.mrg");

    llvm::Value* lhs = emit_valued_expression(expr->get_lhs());
    assert(lhs);

    llvm::BasicBlock* fls = builder.GetInsertBlock();
    builder.CreateCondBr(inject_comparison(lhs), rgt, mrg);

    rgt->insertInto(func);
    builder.SetInsertPoint(rgt);

    llvm::Value* rhs = emit_valued_expression(expr->get_rhs());
    assert(rhs);
    rhs = inject_comparison(rhs);

    builder.CreateBr(mrg);

    llvm::BasicBlock* ow = builder.GetInsertBlock();
    mrg->insertInto(func);
    builder.SetInsertPoint(mrg);

    llvm::PHINode* phi = builder.CreatePHI(llvm::Type::getInt1Ty(mod->getContext()), 2);
    phi->addIncoming(llvm::ConstantInt::getFalse(llvm::Type::getInt1Ty(mod->getContext())), fls);
    phi->addIncoming(rhs, ow);

    return phi;
}

llvm::Value* LLVMCodegen::emit_logical_or(const BinaryOp* expr) {
    llvm::BasicBlock* rgt = llvm::BasicBlock::Create(mod->getContext(), "lor.rgt");
    llvm::BasicBlock* mrg = llvm::BasicBlock::Create(mod->getContext(), "lor.mrg");

    llvm::Value* lhs = emit_valued_expression(expr->get_lhs());
    assert(lhs);

    llvm::BasicBlock* tru = builder.GetInsertBlock();
    builder.CreateCondBr(inject_comparison(lhs), mrg, rgt);

    rgt->insertInto(func);
    builder.SetInsertPoint(rgt);

    llvm::Value* rhs = emit_valued_expression(expr->get_rhs());
    assert(rhs);
    rhs = inject_comparison(rhs);

    builder.CreateBr(mrg);

    llvm::BasicBlock* ow = builder.GetInsertBlock();
    mrg->insertInto(func);
    builder.SetInsertPoint(mrg);

    llvm::PHINode* phi = builder.CreatePHI(llvm::Type::getInt1Ty(mod->getContext()), 2);
    phi->addIncoming(llvm::ConstantInt::getTrue(llvm::Type::getInt1Ty(mod->getContext())), tru);
    phi->addIncoming(rhs, ow);

    return phi;
}

llvm::Value* LLVMCodegen::emit_negation(const UnaryOp* expr) {
    llvm::Value* value = emit_valued_expression(expr->get_expr());
    llvm::Type* type = value->getType();

    if (type->isIntegerTy()) {
        return builder.CreateNeg(value);
    } else if (type->isFloatingPointTy()) {
        return builder.CreateFNeg(value);
    }

    assert(false && "invalid type operand to negation!");
}

llvm::Value* LLVMCodegen::emit_bitwise_not(const UnaryOp* expr) {
    llvm::Value* value = emit_valued_expression(expr->get_expr());
    llvm::Type* type = value->getType();

    if (type->isIntegerTy())
        return builder.CreateNot(value);

    assert(false && "invalid type operand to bitwise not!");
}

llvm::Value* LLVMCodegen::emit_logical_not(const UnaryOp* expr) {
    llvm::Value* value = emit_valued_expression(expr->get_expr());
    llvm::Type* type = value->getType();

    if (type->isIntegerTy()) {
        return builder.CreateICmpNE(value, llvm::ConstantInt::get(type, 0));
    } else if (type->isFloatingPointTy()) {
        return builder.CreateFCmpONE(value, llvm::ConstantFP::getZero(type));
    } else if (type->isPointerTy()) {
        return builder.CreateICmpNE(
            value, 
            llvm::ConstantPointerNull::get(dyn_cast<llvm::PointerType>(type)));
    }

    assert(false && "invalid type operand to logical not!");
}

llvm::Value* LLVMCodegen::emit_address_of(const UnaryOp* expr) {
    return emit_addressed_expression(expr->get_expr());
}

llvm::Value* LLVMCodegen::emit_valued_deref(const UnaryOp* expr) {
    llvm::Value* value = emit_valued_expression(expr->get_expr());
    return builder.CreateLoad(to_llvm_type(expr->get_type()), value);
}

//
//  Copyright (c) 2026 Nick Marino
//  All rights reserved.
//

#include "lace/codegen/LLVMCodegen.hpp"

#include "llvm/IR/Type.h"
#include <llvm/IR/GlobalVariable.h>

using namespace lace;

llvm::Value* LLVMCodegen::emit_addressed_expression(const Expr* expr) {
    switch (expr->get_kind()) {
        case Expr::Unary: {
            const UnaryOp* unary = static_cast<const UnaryOp*>(expr);
            assert(unary->get_operator() == UnaryOp::Dereference);
            return emit_addressed_deref(unary);
        }

        case Expr::Access:
            return emit_addressed_access(static_cast<const AccessExpr*>(expr));

        case Expr::Ref:
            return emit_addressed_ref(static_cast<const RefExpr*>(expr));

        case Expr::Subscript:
            return emit_addressed_subscript(static_cast<const SubscriptExpr*>(expr));

        default:
            break;
    }

    assert(false && "invalid lvalue!");
}

llvm::Value* LLVMCodegen::emit_addressed_access(const AccessExpr* expr) {
    llvm::Value* base = nullptr;
    if (expr->get_base()->get_type()->is_pointer()) {
        base = emit_valued_expression(expr->get_base());
    } else {
        base = emit_addressed_expression(expr->get_base());
    }

    const Type* base_type = expr->get_base()->get_type().get_type();
    if (base_type->is_pointer())
        base_type = static_cast<const PointerType*>(base_type)->get_pointee().get_type();
    
    assert(base_type->is_struct());

    const FieldDefn* field = expr->get_field();
    assert(field);

    llvm::Type* field_type = to_llvm_type(field->get_type());
    uint32_t field_index = field->get_index();

    return builder.CreateInBoundsGEP(
        to_llvm_type(base_type),
        base, 
        { 
            llvm::ConstantInt::get(llvm::Type::getInt32Ty(mod->getContext()), 0), 
            llvm::ConstantInt::get(llvm::Type::getInt32Ty(mod->getContext()), field_index)
        });
}

llvm::Value* LLVMCodegen::emit_addressed_ref(const RefExpr* expr) {
    assert(expr->get_defn());
    switch (expr->get_defn()->get_kind()) {
        case Defn::Function: {
            llvm::Function* function = mod->getFunction(expr->get_name());
            assert(function && "function does not exist!");
            return function;
        }

        case Defn::Parameter: {
            auto it = allocas.find(expr->get_name());
            if (it != allocas.end())
                return it->second;

            assert(false && "alloca does not exist for parameter!");
        }

        case Defn::Variable: {
            const VariableDefn* var = 
                static_cast<const VariableDefn*>(expr->get_defn());

            if (var->is_global()) {
                llvm::GlobalVariable* global = 
                    mod->getGlobalVariable(var->get_name(), true);
                assert(global && "global does not exist!");
                return global;
            } else {
                auto it = allocas.find(expr->get_name());
                if (it != allocas.end())
                    return it->second;

                assert(false && "alloca does not exist for local variable!");
            }
        }
    
    default:
        break;
    }

    assert(false && "invalid lvalue reference!");
}

llvm::Value* LLVMCodegen::emit_addressed_subscript(const SubscriptExpr* expr) {
    llvm::Value* base = nullptr;

    // When subscripting pointers, we need one less level of indirection to
    // get at the type of the pointee for pointer arithmetic.
    if (expr->get_base()->get_type()->is_pointer()) {
        base = emit_valued_expression(expr->get_base());
    } else {
        base = emit_addressed_expression(expr->get_base());
    }

    llvm::Value* index = emit_valued_expression(expr->get_index());
    assert(base);
    assert(index);

    return builder.CreateInBoundsGEP(
        to_llvm_type(expr->get_type()), base, { index });
}

llvm::Value* LLVMCodegen::emit_addressed_deref(const UnaryOp* expr) {
    assert(expr->get_operator() == UnaryOp::Dereference);
    return emit_valued_expression(expr->get_expr());
}

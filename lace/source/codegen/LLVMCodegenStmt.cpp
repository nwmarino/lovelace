//
//  Copyright (c) 2026 Nick Marino
//  All rights reserved.
//

#include "lace/codegen/LLVMCodegen.hpp"
#include "lace/tree/Stmt.hpp"

using namespace lace;

void LLVMCodegen::emit_statement(const Stmt* stmt) {
    switch (stmt->get_kind()) {
        case Stmt::Adapter:
            return emit_adapter(static_cast<const AdapterStmt*>(stmt));
        case Stmt::Block:
            return emit_block(static_cast<const BlockStmt*>(stmt));
        case Stmt::If:
            return emit_if(static_cast<const IfStmt*>(stmt));
        case Stmt::Restart:
            return emit_restart(static_cast<const RestartStmt*>(stmt));
        case Stmt::Ret:
            return emit_return(static_cast<const RetStmt*>(stmt));
        case Stmt::Stop:
            return emit_stop(static_cast<const StopStmt*>(stmt));
        case Stmt::Until:
            return emit_until(static_cast<const UntilStmt*>(stmt));
    }
}

void LLVMCodegen::emit_adapter(const AdapterStmt* stmt) {
    switch (stmt->get_flavor()) {
    case AdapterStmt::Definitive:
        emit_local(static_cast<const VariableDefn*>(stmt->get_defn()));
        break;

    case AdapterStmt::Expressive:
        emit_valued_expression(stmt->get_expr());
        break;
    }
}

void LLVMCodegen::emit_block(const BlockStmt* stmt) {
    for (uint32_t i = 0; i < stmt->num_stmts(); ++i)
        emit_statement(stmt->get_stmt(i));
}

void LLVMCodegen::emit_if(const IfStmt* stmt) {
    llvm::Value* condition = emit_valued_expression(stmt->get_cond());
    assert(condition);

    llvm::BasicBlock* thn = llvm::BasicBlock::Create(mod->getContext(), "if.thn");
    llvm::BasicBlock* mrg = llvm::BasicBlock::Create(mod->getContext(), "if.mrg");
    llvm::BasicBlock* els = nullptr;

    if (stmt->has_else()) {
        els = llvm::BasicBlock::Create(mod->getContext(), "if.els");
        builder.CreateCondBr(inject_comparison(condition), thn, els);
    } else {
        builder.CreateCondBr(inject_comparison(condition), thn, mrg);
    }

    thn->insertInto(func);
    builder.SetInsertPoint(thn);
    emit_statement(stmt->get_then());

    if (!builder.GetInsertBlock()->getTerminator())
        builder.CreateBr(mrg);

    if (stmt->has_else()) {
        els->insertInto(func);
        builder.SetInsertPoint(els);
        emit_statement(stmt->get_else());

        if (!builder.GetInsertBlock()->getTerminator())
            builder.CreateBr(mrg);
    }

    if (mrg->hasNPredecessorsOrMore(1)) {
        mrg->insertInto(func);
        builder.SetInsertPoint(mrg);
    }
}

void LLVMCodegen::emit_restart(const RestartStmt* stmt) {
    assert(!builder.GetInsertBlock()->getTerminator() && 
        "block terminates on its own!");

    builder.CreateBr(parent_cnd);
}

void LLVMCodegen::emit_return(const RetStmt* stmt) {
    if (stmt->has_expr()) {
        llvm::Value* value = emit_valued_expression(stmt->get_expr());
        assert(value);
        builder.CreateRet(value);
    } else {
        builder.CreateRetVoid();
    }
}

void LLVMCodegen::emit_stop(const StopStmt* stmt) {
    assert(!builder.GetInsertBlock()->getTerminator() && 
        "block terminates on its own!");

    builder.CreateBr(parent_mrg);
}

void LLVMCodegen::emit_until(const UntilStmt* stmt) {
    llvm::BasicBlock* cnd = llvm::BasicBlock::Create(mod->getContext(), "until.cnd", func);
    llvm::BasicBlock* bdy = nullptr;
    llvm::BasicBlock* mrg = llvm::BasicBlock::Create(mod->getContext(), "until.mrg");

    builder.CreateBr(cnd);
    
    builder.SetInsertPoint(cnd);
    llvm::Value* condition = emit_valued_expression(stmt->get_cond());
    assert(condition);

    if (stmt->has_body()) {
        bdy = llvm::BasicBlock::Create(mod->getContext(), "until.bdy", func);
        builder.CreateCondBr(inject_comparison(condition), mrg, bdy);

        builder.SetInsertPoint(bdy);

        llvm::BasicBlock* prev_parent_cnd = parent_cnd;
        llvm::BasicBlock* prev_parent_mrg = parent_mrg;

        parent_cnd = cnd;
        parent_mrg = mrg;

        emit_statement(stmt->get_body());

        if (!builder.GetInsertBlock()->getTerminator())
            builder.CreateBr(cnd);

        parent_cnd = prev_parent_cnd;
        parent_mrg = prev_parent_mrg;
    } else {
        builder.CreateCondBr(inject_comparison(condition), mrg, cnd);
    }

    mrg->insertInto(func);
    builder.SetInsertPoint(mrg);
}

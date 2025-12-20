//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/codegen/SPBECodegen.hpp"

#include "spbe/graph/BasicBlock.hpp"

using namespace stm;

void SPBECodegen::visit(AsmStmt& node) {

}

void SPBECodegen::visit(BlockStmt& node) {
    for (uint32_t i = 0, e = node.num_stmts(); i < e; ++i)
        node.get_stmt(i)->accept(*this);
}

void SPBECodegen::visit(DeclStmt& node) {
    node.get_decl()->accept(*this);
}

void SPBECodegen::visit(RetStmt& node) {
    if (node.has_expr()) {
        Expr* expr = node.get_expr();
        m_vctx = RValue;
        expr->accept(*this);

        m_builder.build_ret(m_temp);
    } else {
        m_builder.build_ret_void();
    }
}

void SPBECodegen::visit(IfStmt& node) {
    m_vctx = RValue;
    node.get_cond()->accept(*this);
    assert(m_temp && "if condition does not produce a value!");

    spbe::BasicBlock* then_bb = new spbe::BasicBlock(m_function);
    spbe::BasicBlock* else_bb = nullptr;
    spbe::BasicBlock* merge_bb = new spbe::BasicBlock();

    if (node.has_else()) {
        else_bb = new spbe::BasicBlock();
        m_builder.build_brif(inject_bool_comparison(m_temp), then_bb, else_bb);
    } else {
        m_builder.build_brif(inject_bool_comparison(m_temp), then_bb, merge_bb);
    }

    m_builder.set_insert(then_bb);
    node.get_then()->accept(*this);

    if (!m_builder.get_insert()->terminates())
        m_builder.build_jmp(merge_bb);

    if (node.has_else()) {
        m_function->push_back(else_bb);
        m_builder.set_insert(else_bb);
        node.get_else()->accept(*this);

        if (!m_builder.get_insert()->terminates())
            m_builder.build_jmp(merge_bb);
    }

    if (merge_bb->has_preds()) {
        m_function->push_back(merge_bb);
        m_builder.set_insert(merge_bb);
    }
}

void SPBECodegen::visit(WhileStmt& node) {
    spbe::BasicBlock* cond_bb = new spbe::BasicBlock(m_function);
    spbe::BasicBlock* body_bb = new spbe::BasicBlock();
    spbe::BasicBlock* merge_bb = new spbe::BasicBlock();

    m_builder.build_jmp(cond_bb);

    m_builder.set_insert(cond_bb);
    m_vctx = RValue;
    node.get_cond()->accept(*this);
    assert(m_temp && "while condition does not produce a value!");

    m_builder.build_brif(inject_bool_comparison(m_temp), body_bb, merge_bb);

    m_function->push_back(body_bb);
    m_builder.set_insert(body_bb);

    spbe::BasicBlock* prev_cond = m_cond;
    spbe::BasicBlock* prev_merge = m_merge;
    m_cond = cond_bb;
    m_merge = merge_bb;

    node.get_body()->accept(*this);

    if (!m_builder.get_insert()->terminates())
        m_builder.build_jmp(cond_bb);

    m_function->push_back(merge_bb);
    m_builder.set_insert(merge_bb);

    m_cond = prev_cond;
    m_merge = prev_merge;
}

void SPBECodegen::visit(BreakStmt& node) {
    if (m_builder.get_insert()->terminates()) {
        assert(m_merge && "no merge block to break to!");
        m_builder.build_jmp(m_merge);
    }
}

void SPBECodegen::visit(ContinueStmt& node) {
    if (m_builder.get_insert()->terminates()) {
        assert(m_cond && "no condition block to continue to!");
        m_builder.build_jmp(m_cond);
    }
}

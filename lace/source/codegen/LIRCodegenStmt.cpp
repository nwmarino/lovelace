//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lace/codegen/Codegen.hpp"
#include "lace/tree/AST.hpp"
#include "lace/tree/Defn.hpp"

using namespace lace;

void Codegen::visit(AdapterStmt& node)  {
    switch (node.get_flavor()) {
        case AdapterStmt::Definitive:
            return node.get_defn()->accept(*this);
        case AdapterStmt::Expressive:
            return node.get_expr()->accept(*this);
    }
}

void Codegen::visit(BlockStmt& node)  {
    for (Stmt* stmt : node.get_stmts())
        stmt->accept(*this);
}

void Codegen::visit(IfStmt& node)  {
    m_vctx = RValue;
    node.get_cond()->accept(*this);
    assert(m_temp && "condition does not produce a value!");

    lir::BasicBlock* thn = lir::BasicBlock::create({}, m_function);
    lir::BasicBlock* els = nullptr;
    lir::BasicBlock* mrg = lir::BasicBlock::create();

    if (node.has_else()) {
        els = lir::BasicBlock::create();
        m_builder.build_jif(inject_bool_comparison(m_temp), thn, {}, els, {});
    } else {
        m_builder.build_jif(inject_bool_comparison(m_temp), thn, {}, mrg, {});
    }

    m_builder.set_insert(thn);
    node.get_then()->accept(*this);

    if (!m_builder.get_insert()->terminates())
        m_builder.build_jmp(mrg);

    if (node.has_else()) {
        m_function->append(els);
        m_builder.set_insert(els);
        node.get_else()->accept(*this);

        if (!m_builder.get_insert()->terminates())
            m_builder.build_jmp(mrg);
    }

    if (mrg->has_preds()) {
        m_function->append(mrg);
        m_builder.set_insert(mrg);
    }
}

void Codegen::visit(RestartStmt& node)  {
    if (m_builder.get_insert()->terminates()) {
        assert(m_cnd && "no condition block to restart to!");
        m_builder.build_jmp(m_cnd);
    }
}

void Codegen::visit(RetStmt& node)  {
    if (node.has_expr()) {
        m_vctx = RValue;
        node.get_expr()->accept(*this);
        assert(m_temp && "return does not produce a value!");
        m_builder.build_ret(m_temp);
    } else {
        m_builder.build_ret_void();
    }
}

void Codegen::visit(StopStmt& node)  {
    if (m_builder.get_insert()->terminates()) {
        assert(m_mrg && "no merge block to stop to!");
        m_builder.build_jmp(m_mrg);
    }
}

void Codegen::visit(UntilStmt& node)  {
    lir::BasicBlock* cnd = lir::BasicBlock::create({}, m_function);
    lir::BasicBlock* bdy = nullptr;
    lir::BasicBlock* mrg = lir::BasicBlock::create();

    m_builder.build_jmp(cnd);

    m_builder.set_insert(cnd);
    m_vctx = RValue;
    node.get_cond()->accept(*this);
    assert(m_temp && "condition does not produce a value!");

    if (node.has_body()) {
        bdy = lir::BasicBlock::create({}, m_function);
        m_builder.build_jif(inject_bool_comparison(m_temp), mrg, {}, bdy, {});

        m_function->append(bdy);
        m_builder.set_insert(bdy);

        lir::BasicBlock* prev_cnd = m_cnd;
        lir::BasicBlock* prev_mrg = m_mrg;
        m_cnd = cnd;
        m_mrg = mrg;

        if (node.has_body())
            node.get_body()->accept(*this);

        if (!m_builder.get_insert()->terminates())
            m_builder.build_jmp(cnd);

        m_cnd = prev_cnd;
        m_mrg = prev_mrg;
    } else {
        m_builder.build_jif(inject_bool_comparison(m_temp), mrg, {}, cnd, {});
    }

    m_function->append(mrg);
    m_builder.set_insert(mrg);
}

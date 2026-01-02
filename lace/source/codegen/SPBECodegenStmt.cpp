//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/codegen/SPBECodegen.hpp"

#include "spbe/graph/BasicBlock.hpp"
#include "spbe/graph/InlineAsm.hpp"
#include "spbe/graph/Type.hpp"

#include <vector>

using namespace stm;

void SPBECodegen::visit(AsmStmt& node) {
    const vector<string>& outs = node.get_output_constraints();
    const vector<string>& ins = node.get_input_constraints();
    
    vector<string> constraints = {};
    constraints.reserve(outs.size() + ins.size());

    constraints.insert(constraints.end(), outs.begin(), outs.end());
    constraints.insert(constraints.end(), ins.begin(), ins.end());

    for (uint32_t i = 0, e = node.num_clobbers(); i < e; ++i)
        constraints.push_back('~' + node.get_clobber(i));

    vector<spbe::Value*> args(node.num_args(), nullptr);
    vector<const spbe::Type*> arg_types(node.num_args(), nullptr);

    for (uint32_t i = 0, e = node.num_args(); i < e; ++i) {
        // Load outputs as lvalues.
        m_vctx = i < outs.size() ? LValue : RValue;
        node.get_arg(i)->accept(*this);
        args[i] = m_temp;
        arg_types[i] = m_temp->get_type();
    }

    const spbe::FunctionType* type = spbe::FunctionType::get(
        m_graph, arg_types, nullptr);

    m_temp = m_builder.build_call(
        type, 
        new spbe::InlineAsm(type, node.get_assembly_string(), constraints), 
        args);
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

    if (node.has_body())
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

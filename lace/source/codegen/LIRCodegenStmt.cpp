//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lace/codegen/LIRCodegen.hpp"
#include "lace/tree/Defn.hpp"
#include "lace/tree/Stmt.hpp"
#include "lir/graph/Function.hpp"
#include "lir/graph/Type.hpp"

using namespace lace;

void LIRCodegen::codegen_statement(const Stmt* stmt) {
    switch (stmt->get_kind()) {
        case Stmt::Kind::Adapter:
            return codegen_adapter(static_cast<const AdapterStmt*>(stmt));
        case Stmt::Kind::Block:
            return codegen_block(static_cast<const BlockStmt*>(stmt));
        case Stmt::Kind::If:
            return codegen_if(static_cast<const IfStmt*>(stmt));
        case Stmt::Kind::Restart:
            return codegen_restart(static_cast<const RestartStmt*>(stmt));
        case Stmt::Kind::Ret:
            return codegen_return(static_cast<const RetStmt*>(stmt));
        case Stmt::Kind::Stop:
            return codegen_stop(static_cast<const StopStmt*>(stmt));
        case Stmt::Kind::Until:
            return codegen_until(static_cast<const UntilStmt*>(stmt));
        case Stmt::Kind::Rune:
            return codegen_rune_statement(static_cast<const RuneStmt*>(stmt));
    }
}

void LIRCodegen::codegen_adapter(const AdapterStmt* stmt)  {
    switch (stmt->get_flavor()) {
        case AdapterStmt::Definitive: {
            auto var = dynamic_cast<const VariableDefn*>(stmt->get_defn());
            assert(var && "cannot generate code for a non-variable adapter!");
            codegen_local_variable(var);    
            break;
        }

        case AdapterStmt::Expressive:
            codegen_valued_expression(stmt->get_expr());
            break;
    }
}

void LIRCodegen::codegen_block(const BlockStmt* stmt)  {
    for (Stmt* stmt : stmt->get_stmts())
        codegen_statement(stmt);
}

void LIRCodegen::codegen_if(const IfStmt* stmt) {
    lir::Value* condition = codegen_valued_expression(stmt->get_cond());
    assert(condition);
    condition = inject_comparison(condition);

    lir::BasicBlock* then_bb = lir::BasicBlock::create({}, m_func);
    lir::BasicBlock* else_bb = nullptr;
    lir::BasicBlock* merge_bb = lir::BasicBlock::create();

    if (stmt->has_else()) {
        else_bb = lir::BasicBlock::create();
        m_builder.build_jif(condition, then_bb, {}, else_bb, {});
    } else {
        m_builder.build_jif(condition, then_bb, {}, merge_bb, {});
    }

    m_builder.set_insert(then_bb);
    codegen_statement(stmt->get_then());

    if (!m_builder.get_insert()->terminates())
        m_builder.build_jmp(merge_bb);

    if (stmt->has_else()) {
        m_func->append(else_bb);
        m_builder.set_insert(else_bb);
        codegen_statement(stmt->get_else());

        if (!m_builder.get_insert()->terminates())
            m_builder.build_jmp(merge_bb);
    }

    if (merge_bb->has_preds()) {
        m_func->append(merge_bb);
        m_builder.set_insert(merge_bb);
    } else {
        delete merge_bb;
    }
}

void LIRCodegen::codegen_until(const UntilStmt* stmt)  {
    lir::BasicBlock* cond_bb = lir::BasicBlock::create({}, m_func);
    lir::BasicBlock* body_bb = nullptr;
    lir::BasicBlock* merge_bb = lir::BasicBlock::create();

    m_builder.build_jmp(cond_bb);

    m_builder.set_insert(cond_bb);
    lir::Value* condition = codegen_valued_expression(stmt->get_cond());
    assert(condition);
    condition = inject_comparison(condition);

    if (stmt->has_body()) {
        body_bb = lir::BasicBlock::create({}, m_func);
        m_builder.build_jif(condition, merge_bb, {}, body_bb, {});

        m_builder.set_insert(body_bb);

        lir::BasicBlock* prev_cnd = m_parent_cond;
        lir::BasicBlock* prev_mrg = m_parent_merge;
        m_parent_cond = cond_bb;
        m_parent_merge = merge_bb;

        codegen_statement(stmt->get_body());

        if (!m_builder.get_insert()->terminates())
            m_builder.build_jmp(cond_bb);

        m_parent_cond = prev_cnd;
        m_parent_merge = prev_mrg;
    } else {
        m_builder.build_jif(condition, merge_bb, {}, cond_bb, {});
    }

    m_func->append(merge_bb);
    m_builder.set_insert(merge_bb);
}

void LIRCodegen::codegen_restart(const RestartStmt* stmt)  {
    if (!m_builder.get_insert()->terminates()) {
        assert(m_parent_cond && "no condition block to restart to!");
        m_builder.build_jmp(m_parent_cond);
    }
}

void LIRCodegen::codegen_stop(const StopStmt* stmt)  {
    if (!m_builder.get_insert()->terminates()) {
        assert(m_parent_merge && "no merge block to stop to!");
        m_builder.build_jmp(m_parent_merge);
    }
}

void LIRCodegen::codegen_return(const RetStmt* stmt) {
    if (!stmt->has_expr()) {
        m_builder.build_ret();
        return;
    }

    lir::Type* type = to_lir_type(stmt->get_expr()->get_type());
    
    if (m_mach.is_scalar(type)) {
        lir::Value* value = codegen_valued_expression(stmt->get_expr());
        assert(value);

        m_builder.build_ret(value);
    } else {
        // Return type must be non-scalar/aggregate, so we write changes to the
        // aret argument.
        lir::Value* value = codegen_addressed_expression(stmt->get_expr());
        assert(value);

        lir::Function* copy = get_intrinsic(
            "__copy", 
            lir::VoidType::get(m_cfg), 
            { 
                lir::PointerType::get_void_pointer(m_cfg),
                lir::PointerType::get_void_pointer(m_cfg),
                lir::IntegerType::get_i64_type(m_cfg),
            }
        );

        lir::FunctionArgument* aret = m_func->get_aret();
        assert(aret);

        m_builder.build_call(copy->get_type(), copy, {
            aret,
            value,
            lir::Integer::get(m_cfg, lir::IntegerType::get_i64_type(m_cfg), m_mach.get_size(type))
        });

        m_builder.build_ret();
    }
}

void LIRCodegen::codegen_rune_statement(const RuneStmt* stmt) {
    switch (stmt->get_rune()->get_kind()) {
        case Rune::Abort:
            m_builder.build_abort();
            return;

        case Rune::Unreachable:
            m_builder.build_unreachable();
            return;

        default:
            assert(false && "invalid rune statement!");
    }
}

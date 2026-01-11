//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lace/core/Diagnostics.hpp"
#include "lace/codegen/LIRCodegen.hpp"
#include "lace/tree/AST.hpp"
#include "lace/tree/Defn.hpp"
#include "lir/graph/Type.hpp"

using namespace lace;

void Codegen::visit(BoolLiteral& node) {
    m_temp = lir::Integer::get(
        m_cfg, 
        lower_type(node.get_type()), 
        static_cast<int64_t>(node.get_value()));
}

void Codegen::visit(CharLiteral& node) {
    m_temp = lir::Integer::get(
        m_cfg,
        lir::Type::get_i8_type(m_cfg),
        static_cast<int64_t>(node.get_value()));
}

void Codegen::visit(IntegerLiteral& node) {
    m_temp = lir::Integer::get(
        m_cfg, 
        lower_type(node.get_type()), 
        node.get_value());
}

void Codegen::visit(FloatLiteral& node) {
    m_temp = lir::Float::get(
        m_cfg, 
        lower_type(node.get_type()), 
        node.get_value());
}

void Codegen::visit(NullLiteral& node) {
    m_temp = lir::Null::get(m_cfg, lower_type(node.get_type()));
}

void Codegen::visit(StringLiteral& node) {
    m_temp = m_builder.build_string(lir::String::get(m_cfg, node.get_value()));
}

void Codegen::visit(AccessExpr& node) {
    ValueContext ctx = m_vctx;

    m_vctx = LValue;
    node.get_base()->accept(*this);
    assert(m_temp && "base does not produce a value!");
    log::note("temp type: " + m_temp->get_type()->to_string(), 
        log::Span(m_cfg.get_filename(), node.get_span()));

    const FieldDefn* field = node.get_field();
    lir::Type* field_type = lower_type(field->get_type());

    lir::Integer* index = lir::Integer::get(
        m_cfg, lir::Type::get_i64_type(m_cfg), field->get_index());

    m_temp = m_builder.build_access(lir::PointerType::get(m_cfg, field_type), m_temp, index);
    if (ctx == RValue)
        m_temp = m_builder.build_load(field_type, m_temp, m_mach.get_align(field_type));
}

void Codegen::visit(ParenExpr& node) {
    node.get_expr()->accept(*this);
}

void Codegen::visit(RefExpr& node) {
    lir::Type* type = lower_type(node.get_type());

    switch (node.get_defn()->get_kind()) {
        case Defn::Function: {
            m_temp = m_cfg.get_function(node.get_name());
            assert(m_temp && "function does not exist!");
            assert(m_vctx != RValue && 
                "cannot produce function reference as an rvalue!");
                
            return;
        }

        case Defn::Parameter:
            m_temp = m_function->get_local(node.get_name());
            assert(m_temp && "parameter does not exist as a local!");

            if (m_vctx == RValue)
                m_temp = m_builder.build_load(type, m_temp, m_mach.get_align(type));

            return;

        case Defn::Variable: {
            const VariableDefn* var = 
                static_cast<const VariableDefn*>(node.get_defn());
            
            if (var->is_global()) {
                m_temp = m_cfg.get_global(node.get_name());
                assert(m_temp && "global does not exist for variable!");
            } else {
                m_temp = m_function->get_local(node.get_name());
                if (!m_temp)
                    log::fatal("unresolved variable: " + node.get_name(), 
                        log::Span(m_cfg.get_filename(), node.get_span()));
            }

            if (m_vctx == RValue)
                m_temp = m_builder.build_load(type, m_temp, m_mach.get_align(type));

            return;
        }

        case Defn::Variant:
            m_temp = lir::Integer::get(
                m_cfg, 
                type, 
                static_cast<const VariantDefn*>(node.get_defn())->get_value());
            return;

        default:
            assert(false && "invalid definition reference!");
    }
}

void Codegen::visit(SizeofExpr& node) {
    m_temp = lir::Integer::get(
        m_cfg,
        lower_type(node.get_type()),
        m_mach.get_size(lower_type(node.get_target_type())));
}

void Codegen::visit(SubscriptExpr& node) {
    ValueContext ctx = m_vctx;
    lir::Value* base = nullptr;
    lir::Value* index = nullptr;
    lir::Type* type = lower_type(node.get_type());

    m_vctx = LValue;
    if (node.get_base()->get_type()->is_pointer())
        m_vctx = RValue;

    node.get_base()->accept(*this);
    assert(m_temp && "base does not produce a value!");
    base = m_temp;

    node.get_index()->accept(*this);
    assert(m_temp && "index does not produce a value!");
    index = m_temp;

    m_temp = m_builder.build_ap(lir::PointerType::get(m_cfg, type), base, index);
    if (ctx == RValue)
        m_temp = m_builder.build_load(type, m_temp, m_mach.get_align(type));
}

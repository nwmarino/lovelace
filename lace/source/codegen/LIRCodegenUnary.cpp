//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lace/codegen/Codegen.hpp"
#include "lace/tree/Defn.hpp"
#include "lir/graph/Constant.hpp"
#include "lir/graph/Type.hpp"

using namespace lace;

void Codegen::codegen_negate(UnaryOp& node) {
    m_vctx = RValue;
    node.get_expr()->accept(*this);

    lir::Type* type = m_temp->get_type();
    if (type->is_integer_type()) {
        if (const lir::Integer* integer = dynamic_cast<lir::Integer*>(m_temp)) {
            m_temp = lir::Integer::get(m_cfg, type, -integer->get_value());
        } else {
            m_temp = m_builder.build_ineg(m_temp);
        }
    } else if (type->is_float_type()) {
        if (const lir::Float* fp = dynamic_cast<lir::Float*>(m_temp)) {
            m_temp = lir::Float::get(m_cfg, type, -fp->get_value());
        } else {
            m_temp = m_builder.build_fneg(m_temp);
        }
    } else {
        assert(false && "type incompatible with negate operator!");
    }
}

void Codegen::codegen_bitwise_not(UnaryOp& node) {
    m_vctx = RValue;
    node.get_expr()->accept(*this);

    lir::Type* type = m_temp->get_type();
    if (type->is_integer_type()) { 
        if (const lir::Integer* integer = dynamic_cast<lir::Integer*>(m_temp)) {
            m_temp = lir::Integer::get(m_cfg, type, ~integer->get_value());
        } else {
            m_temp = m_builder.build_not(m_temp);
        }
    } else {
        assert(false && "type incompatible with bitwise not operator!");
    }
}

void Codegen::codegen_logical_not(UnaryOp& node) {
    m_vctx = RValue;
    node.get_expr()->accept(*this);

    lir::Type* type = m_temp->get_type();
    if (type->is_integer_type()) {
        if (const lir::Integer* integer = dynamic_cast<lir::Integer*>(m_temp)) {
            m_temp = lir::Integer::get(
                m_cfg, 
                lir::Type::get_i1_type(m_cfg), 
                !integer->get_value());
        } else {
            m_temp = m_builder.build_cmp_ine(
                m_temp, 
                lir::Integer::get_zero(m_cfg, type));
        }
    } else if (type->is_float_type()) {
        if (const lir::Float* fp = dynamic_cast<lir::Float*>(m_temp)) {
            m_temp = lir::Integer::get(
                m_cfg, 
                lir::Type::get_i1_type(m_cfg), 
                !fp->get_value());
        } else {
            m_temp = m_builder.build_cmp_one(
                m_temp, 
                lir::Float::get_zero(m_cfg, type));
        }
    } else if (type->is_pointer_type()) {
        if (dynamic_cast<lir::Null*>(m_temp)) {
            m_temp = lir::Integer::get_true(m_cfg); 
        } else {
            m_temp = m_builder.build_cmp_ine(m_temp, lir::Null::get(m_cfg, type));
        }
    } else {
        assert(false && "type incompatible with logical not operator!");
    }
}

void Codegen::codegen_address_of(UnaryOp& node) {
    m_vctx = LValue;
    node.get_expr()->accept(*this);
}

void Codegen::codegen_dereference(UnaryOp& node) {
    ValueContext ctx = m_vctx;
    m_vctx = RValue;
    node.get_expr()->accept(*this);

    if (ctx == RValue) {
        lir::Type* type = lower_type(node.get_type());
        m_temp = m_builder.build_load(type, m_temp, m_mach.get_align(type));
    }
}

void Codegen::visit(UnaryOp& node) {
    switch (node.get_operator()) {
        case UnaryOp::Negate:
            return codegen_negate(node);
        case UnaryOp::Not:
            return codegen_bitwise_not(node);
        case UnaryOp::LogicNot:
            return codegen_logical_not(node);
        case UnaryOp::AddressOf:
            return codegen_address_of(node);
        case UnaryOp::Dereference:
            return codegen_dereference(node);
        default:
            assert(false && "unknown operator!");
    }

    __builtin_unreachable();
}

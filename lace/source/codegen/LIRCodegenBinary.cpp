//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lace/codegen/LIRCodegen.hpp"
#include "lace/tree/Defn.hpp"
#include "lace/tree/Type.hpp"
#include "lir/graph/Constant.hpp"
#include "lir/graph/Type.hpp"

using namespace lace;

void Codegen::codegen_assignment(BinaryOp& node) {
    lir::Value* loc = nullptr;
    lir::Value* rhs = nullptr;

    m_vctx = LValue;
    node.get_lhs()->accept(*this);
    loc = m_temp;

    m_vctx = RValue;
    node.get_rhs()->accept(*this);
    rhs = m_temp;

    m_builder.build_store(rhs, loc, m_mach.get_align(rhs->get_type()));
}

void Codegen::codegen_addition(BinaryOp& node, AddOp op) {
    lir::Value* lhs = nullptr;
    lir::Value* rhs = nullptr;

    m_vctx = RValue;
    node.get_lhs()->accept(*this);
    lhs = m_temp;

    m_vctx = RValue;
    node.get_rhs()->accept(*this);
    rhs = m_temp;

    lir::Type* lhs_type = lhs->get_type();
    lir::Type* rhs_type = rhs->get_type();

    if (lhs_type->is_pointer_type() && rhs_type->is_integer_type()) {
        if (op == AddOp::Sub) {
            if (lir::Integer* integer = dynamic_cast<lir::Integer*>(rhs)) {
                rhs = lir::Integer::get(m_cfg, rhs->get_type(), -integer->get_value());
            } else {
                rhs = m_builder.build_ineg(rhs);
            }
        }

        m_temp = m_builder.build_pwalk(lhs_type, lhs, { rhs });
    } else if (lhs_type->is_integer_type()) {
        if (op == AddOp::Add) {
            m_temp = m_builder.build_iadd(lhs, rhs);
        } else if (op == AddOp::Sub) {
            m_temp = m_builder.build_isub(lhs, rhs);
        }
    } else if (lhs_type->is_float_type()) {
        if (op == AddOp::Add) {
            m_temp = m_builder.build_fadd(lhs, rhs);
        } else if (op == AddOp::Sub) {
            m_temp = m_builder.build_fsub(lhs, rhs);
        }
    } else {
        assert(false && "type incompatible with addition operator!");
    }
}

void Codegen::codegen_multiplication(BinaryOp& node, MulOp op) {
    lir::Value* loc = nullptr;
    lir::Value* lhs = nullptr;
    lir::Value* rhs = nullptr;

    m_vctx = RValue;
    node.get_lhs()->accept(*this);
    lhs = m_temp;

    m_vctx = RValue;
    node.get_rhs()->accept(*this);
    rhs = m_temp;

    lir::Type* lhs_type = lhs->get_type();
    if (lhs_type->is_integer_type()) {
        if (node.get_type()->is_signed_integer()) {
            if (op == MulOp::Mul) {
                m_temp = m_builder.build_imul(lhs, rhs);
            } else if (op == MulOp::Div) {
                m_temp = m_builder.build_sdiv(lhs, rhs);
            } else if (op == MulOp::Mod) {
                m_temp = m_builder.build_smod(lhs, rhs);
            }
        } else {
            if (op == MulOp::Mul) {
                m_temp = m_builder.build_imul(lhs, rhs);
            } else if (op == MulOp::Div) {
                m_temp = m_builder.build_udiv(lhs, rhs);
            } else if (op == MulOp::Mod) {
                m_temp = m_builder.build_umod(lhs, rhs);
            }
        }
    } else if (lhs_type->is_float_type()) {
        if (op == MulOp::Mul) {
            m_temp = m_builder.build_fmul(lhs, rhs);
        } else if (op == MulOp::Div) {
            m_temp = m_builder.build_fdiv(lhs, rhs);
        }
    } else {
        assert(false && "type incompatible with multiplication operator!");
    }
}

void Codegen::codegen_bitwise_arithmetic(BinaryOp& node, BitwiseOp op) {
    lir::Value* lhs = nullptr;
    lir::Value* rhs = nullptr;

    m_vctx = RValue;
    node.get_lhs()->accept(*this);
    lhs = m_temp;

    m_vctx = RValue;
    node.get_rhs()->accept(*this);
    rhs = m_temp;

    assert(lhs->get_type()->is_integer_type() && 
        "type incompatible with bitwise operator!");

    if (op == BitwiseOp::And) {
        lir::Integer* int1 = dynamic_cast<lir::Integer*>(lhs);
        lir::Integer* int2 = dynamic_cast<lir::Integer*>(rhs);

        if (int1 && int2) {
            m_temp = lir::Integer::get(
                m_cfg, lhs->get_type(), int1->get_value() & int2->get_value());
        } else {
            m_temp = m_builder.build_and(lhs, rhs);
        }
    } else if (op == BitwiseOp::Or) {
        lir::Integer* int1 = dynamic_cast<lir::Integer*>(lhs);
        lir::Integer* int2 = dynamic_cast<lir::Integer*>(rhs);

        if (int1 && int2) {
            m_temp = lir::Integer::get(
                m_cfg, lhs->get_type(), int1->get_value() | int2->get_value());
        } else {
            m_temp = m_builder.build_or(lhs, rhs);
        }
    } else if (op == BitwiseOp::Xor) {
        lir::Integer* int1 = dynamic_cast<lir::Integer*>(lhs);
        lir::Integer* int2 = dynamic_cast<lir::Integer*>(rhs);

        if (int1 && int2) {
            m_temp = lir::Integer::get(
                m_cfg, lhs->get_type(), int1->get_value() ^ int2->get_value());
        } else {
            m_temp = m_builder.build_xor(lhs, rhs);
        }
    }
}

void Codegen::codegen_bitwise_shift(BinaryOp& node, ShiftOp op) {
    lir::Value* lhs = nullptr;
    lir::Value* rhs = nullptr;

    m_vctx = RValue;
    node.get_lhs()->accept(*this);
    lhs = m_temp;

    m_vctx = RValue;
    node.get_rhs()->accept(*this);
    rhs = m_temp;

    assert(lhs->get_type()->is_integer_type() && 
        rhs->get_type()->is_integer_type() && 
        "type incompatible with bitwise operator!");

    if (op == ShiftOp::LShift) {
        m_temp = m_builder.build_shl(lhs, rhs);
    } else if (op == ShiftOp::RShift) {
        if (node.get_lhs()->get_type()->is_signed_integer()) {
            m_temp = m_builder.build_sar(lhs, rhs);
        } else {
            m_temp = m_builder.build_shr(lhs, rhs);
        }
    }
}

void Codegen::codegen_numerical_comparison(BinaryOp& node, CmpOp op) {
    lir::Value* lhs = nullptr;
    lir::Value* rhs = nullptr;

    m_vctx = RValue;
    node.get_lhs()->accept(*this);
    lhs = m_temp;

    m_vctx = RValue;
    node.get_rhs()->accept(*this);
    rhs = m_temp;

    const QualType& type = node.get_lhs()->get_type();
    switch (op) {
        case CmpOp::Eq:
            if (type->is_integer() || type->is_pointer()) {
                m_temp = m_builder.build_cmp_ieq(lhs, rhs);
            } else if (type->is_floating_point()) {
                m_temp = m_builder.build_cmp_oeq(lhs, rhs);
            }

            break;

        case CmpOp::NEq:
            if (type->is_integer() || type->is_pointer()) {
                m_temp = m_builder.build_cmp_ine(lhs, rhs);
            } else if (type->is_floating_point()) {
                m_temp = m_builder.build_cmp_one(lhs, rhs);
            }

            break;

        case CmpOp::Lt:
            if (type->is_signed_integer() || type->is_pointer()) {
                m_temp = m_builder.build_cmp_slt(lhs, rhs);
            } else if (type->is_unsigned_integer()) {
                m_temp = m_builder.build_cmp_ult(lhs, rhs);
            } else if (type->is_floating_point()) {
                m_temp = m_builder.build_cmp_olt(lhs, rhs);
            }
            
            break;

        case CmpOp::LtEq:
            if (type->is_signed_integer() || type->is_pointer()) {
                m_temp = m_builder.build_cmp_sle(lhs, rhs);
            } else if (type->is_unsigned_integer()) {
                m_temp = m_builder.build_cmp_ule(lhs, rhs);
            } else if (type->is_floating_point()) {
                m_temp = m_builder.build_cmp_ole(lhs, rhs);
            }
            
            break;

        case CmpOp::Gt:
            if (type->is_signed_integer() || type->is_pointer()) {
                m_temp = m_builder.build_cmp_sgt(lhs, rhs);
            } else if (type->is_unsigned_integer()) {
                m_temp = m_builder.build_cmp_ugt(lhs, rhs);
            } else if (type->is_floating_point()) {
                m_temp = m_builder.build_cmp_ogt(lhs, rhs);
            }
            
            break;

        case CmpOp::GtEq:
            if (type->is_signed_integer() || type->is_pointer()) {
                m_temp = m_builder.build_cmp_sge(lhs, rhs);
            } else if (type->is_unsigned_integer()) {
                m_temp = m_builder.build_cmp_uge(lhs, rhs);
            } else if (type->is_floating_point()) {
                m_temp = m_builder.build_cmp_oge(lhs, rhs);
            }
            
            break;
    }
}

void Codegen::codegen_logical_and(BinaryOp& node) {
    lir::BasicBlock* rgt = lir::BasicBlock::create();
    lir::BasicBlock* mrg = lir::BasicBlock::create();
    lir::BasicBlock::Arg* and_res = lir::BasicBlock::Arg::create(
        lir::Type::get_i1_type(m_cfg), mrg);

    lir::Value* lhs = nullptr;
    lir::Value* rhs = nullptr;

    m_vctx = RValue;
    node.get_lhs()->accept(*this);
    lhs = inject_bool_comparison(m_temp);

    lir::BasicBlock* fls = m_builder.get_insert();
    m_builder.build_jif(lhs, rgt, {}, mrg, { lir::Integer::get_false(m_cfg) });

    m_function->append(rgt);
    m_builder.set_insert(rgt);

    m_vctx = RValue;
    node.get_rhs()->accept(*this);
    rhs = inject_bool_comparison(m_temp);

    m_builder.build_jmp(mrg, { rhs });

    lir::BasicBlock* ow = m_builder.get_insert();
    m_function->append(mrg);
    m_builder.set_insert(mrg);

    m_temp = and_res;
}

void Codegen::codegen_logical_or(BinaryOp& node) {
    lir::BasicBlock* rgt = lir::BasicBlock::create();
    lir::BasicBlock* mrg = lir::BasicBlock::create();
    lir::BasicBlock::Arg* or_res = lir::BasicBlock::Arg::create(
        lir::Type::get_i1_type(m_cfg), mrg);

    lir::Value* lhs = nullptr;
    lir::Value* rhs = nullptr;

    m_vctx = RValue;
    node.get_lhs()->accept(*this);
    lhs = inject_bool_comparison(m_temp);

    lir::BasicBlock* tru = m_builder.get_insert();
    m_builder.build_jif(lhs, mrg, { lir::Integer::get_true(m_cfg) }, rgt, {});

    m_function->append(rgt);
    m_builder.set_insert(rgt);

    m_vctx = RValue;
    node.get_rhs()->accept(*this);
    rhs = inject_bool_comparison(m_temp);

    m_builder.build_jmp(mrg, { rhs });

    lir::BasicBlock* ow = m_builder.get_insert();
    m_function->append(mrg);
    m_builder.set_insert(mrg);

    m_temp = or_res;
}

void Codegen::visit(BinaryOp& node) {
    switch (node.get_operator()) {
        case BinaryOp::Assign:
            return codegen_assignment(node);
        case BinaryOp::Add:
            return codegen_addition(node, AddOp::Add);
        case BinaryOp::Sub:
            return codegen_addition(node, AddOp::Sub);
        case BinaryOp::Mul:
            return codegen_multiplication(node, MulOp::Mul);
        case BinaryOp::Div:
            return codegen_multiplication(node, MulOp::Div);
        case BinaryOp::Mod:
            return codegen_multiplication(node, MulOp::Mod);
        case BinaryOp::And:
            return codegen_bitwise_arithmetic(node, BitwiseOp::And);
        case BinaryOp::Or:
            return codegen_bitwise_arithmetic(node, BitwiseOp::Or);
        case BinaryOp::Xor:
            return codegen_bitwise_arithmetic(node, BitwiseOp::Xor);
        case BinaryOp::LShift:
            return codegen_bitwise_shift(node, ShiftOp::LShift);
        case BinaryOp::RShift:
            return codegen_bitwise_shift(node, ShiftOp::RShift);
        case BinaryOp::LogicAnd:
            return codegen_logical_and(node);
        case BinaryOp::LogicOr:
            return codegen_logical_or(node);
        case BinaryOp::Eq:
            return codegen_numerical_comparison(node, CmpOp::Eq);
        case BinaryOp::NEq:
            return codegen_numerical_comparison(node, CmpOp::NEq);
        case BinaryOp::Lt:
            return codegen_numerical_comparison(node, CmpOp::Lt);
        case BinaryOp::LtEq:
            return codegen_numerical_comparison(node, CmpOp::LtEq);
        case BinaryOp::Gt:
            return codegen_numerical_comparison(node, CmpOp::Gt);
        case BinaryOp::GtEq:
            return codegen_numerical_comparison(node, CmpOp::GtEq);
        default:
            assert(false && "unknown operator!");
    }

    __builtin_unreachable();
}

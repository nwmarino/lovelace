//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/codegen/SPBECodegen.hpp"
#include "stmc/tree/Decl.hpp"
#include "stmc/tree/Visitor.hpp"

#include "spbe/graph/BasicBlock.hpp"
#include "spbe/graph/Constant.hpp"
#include "spbe/graph/Instruction.hpp"
#include "spbe/graph/Type.hpp"
#include "spbe/target/Target.hpp"

#include <cassert>

using namespace stm;

void SPBECodegen::visit(BoolLiteral& node) {
    m_temp = spbe::ConstantInt::get(
        m_graph, 
        lower_type(node.get_type()), 
        static_cast<int64_t>(node.get_value()));
}

void SPBECodegen::visit(IntegerLiteral& node) {
    m_temp = spbe::ConstantInt::get(
        m_graph, 
        lower_type(node.get_type()), 
        node.get_value());
}

void SPBECodegen::visit(FPLiteral& node) {
    m_temp = spbe::ConstantFP::get(
        m_graph, 
        lower_type(node.get_type()), 
        node.get_value());
}

void SPBECodegen::visit(CharLiteral& node) {
    m_temp = spbe::ConstantInt::get(
        m_graph, 
        spbe::Type::get_i8_type(m_graph), 
        static_cast<int64_t>(node.get_value()));
}

void SPBECodegen::visit(StringLiteral& node) {
    m_temp = m_builder.build_string(spbe::ConstantString::get(
        m_graph, 
        node.get_value()));
}

void SPBECodegen::visit(NullLiteral& node) {
    m_temp = spbe::ConstantNull::get(
        m_graph, 
        lower_type(node.get_type()));
}

void SPBECodegen::codegen_assignment(BinaryOp& op) {
    spbe::Value* loc = nullptr;
    spbe::Value* rhs = nullptr;

    m_vctx = LValue;
    op.get_lhs()->accept(*this);
    loc = m_temp;

    m_vctx = RValue;
    op.get_rhs()->accept(*this);
    rhs = m_temp;

    m_builder.build_store(rhs, loc);
}

void SPBECodegen::codegen_addition(BinaryOp& node, AddOp op, bool assign) {
    spbe::Value* loc = nullptr;
    spbe::Value* lhs = nullptr;
    spbe::Value* rhs = nullptr;

    if (assign) {
        m_vctx = LValue;
        node.get_lhs()->accept(*this);
        loc = m_temp;

        lhs = m_builder.build_load(static_cast<const spbe::PointerType*>(
            loc->get_type())->get_pointee(), loc);
    } else {
        m_vctx = RValue;
        node.get_lhs()->accept(*this);
        lhs = m_temp;
    }

    m_vctx = RValue;
    node.get_rhs()->accept(*this);
    rhs = m_temp;

    const spbe::Type* lhs_type = lhs->get_type();
    const spbe::Type* rhs_type = rhs->get_type();

    if (lhs_type->is_pointer_type() && rhs_type->is_integer_type()) {
        if (op == AddOp::Sub)
            rhs = m_builder.build_ineg(rhs);

        m_temp = m_builder.build_ap(lhs_type, lhs, rhs);
    } else if (lhs_type->is_integer_type()) {
        if (op == AddOp::Add) {
            m_temp = m_builder.build_iadd(lhs, rhs);
        } else if (op == AddOp::Sub) {
            m_temp = m_builder.build_isub(lhs, rhs);
        }
    } else if (lhs_type->is_floating_point_type()) {
        if (op == AddOp::Add) {
            m_temp = m_builder.build_fadd(lhs, rhs);
        } else if (op == AddOp::Sub) {
            m_temp = m_builder.build_fsub(lhs, rhs);
        }
    } else {
        assert(false && "type incompatible with addition operator!");
    }

    if (assign)
        m_builder.build_store(m_temp, loc);
}

void SPBECodegen::codegen_multiplication(
        BinaryOp& node, MulOp op, bool assign) {
    spbe::Value* loc = nullptr;
    spbe::Value* lhs = nullptr;
    spbe::Value* rhs = nullptr;

    if (assign) {
        m_vctx = LValue;
        node.get_lhs()->accept(*this);
        loc = m_temp;

        lhs = m_builder.build_load(static_cast<const spbe::PointerType*>(
            loc->get_type())->get_pointee(), loc);
    } else {
        m_vctx = RValue;
        node.get_lhs()->accept(*this);
        lhs = m_temp;
    }

    m_vctx = RValue;
    node.get_rhs()->accept(*this);
    rhs = m_temp;

    const spbe::Type* lhs_type = lhs->get_type();

    if (lhs_type->is_integer_type()) {
        if (node.get_type()->is_signed_integer()) {
            if (op == MulOp::Mul) {
                m_temp = m_builder.build_smul(lhs, rhs);
            } else if (op == MulOp::Div) {
                m_temp = m_builder.build_sdiv(lhs, rhs);
            } else if (op == MulOp::Mod) {
                m_temp = m_builder.build_srem(lhs, rhs);
            }
        } else {
            if (op == MulOp::Mul) {
                m_temp = m_builder.build_umul(lhs, rhs);
            } else if (op == MulOp::Div) {
                m_temp = m_builder.build_udiv(lhs, rhs);
            } else if (op == MulOp::Mod) {
                m_temp = m_builder.build_urem(lhs, rhs);
            }
        }
    } else if (lhs_type->is_floating_point_type()) {
        if (op == MulOp::Mul) {
            m_temp = m_builder.build_fmul(lhs, rhs);
        } else if (op == MulOp::Div) {
            m_temp = m_builder.build_fdiv(lhs, rhs);
        }
    } else {
        assert(false && "type incompatible with multiplication operator!");
    }

    if (assign)
        m_builder.build_store(m_temp, loc);
}

void SPBECodegen::codegen_bitwise_arithmetic(
        BinaryOp& node, BitwiseOp op, bool assign) {
    spbe::Value* loc = nullptr;
    spbe::Value* lhs = nullptr;
    spbe::Value* rhs = nullptr;

    if (assign) {
        m_vctx = LValue;
        node.get_lhs()->accept(*this);
        loc = m_temp;

        lhs = m_builder.build_load(static_cast<const spbe::PointerType*>(
            loc->get_type())->get_pointee(), loc);
    } else {
        m_vctx = RValue;
        node.get_lhs()->accept(*this);
        lhs = m_temp;
    }

    m_vctx = RValue;
    node.get_rhs()->accept(*this);
    rhs = m_temp;

    assert(lhs->get_type()->is_integer_type() && 
        "type incompatible with bitwise operator!");

    if (op == BitwiseOp::And) {
        m_temp = m_builder.build_and(lhs, rhs);
    } else if (op == BitwiseOp::Or) {
        m_temp = m_builder.build_or(lhs, rhs);
    } else if (op == BitwiseOp::Xor) {
        m_temp = m_builder.build_xor(lhs, rhs);
    }

    if (assign)
        m_builder.build_store(loc, m_temp);
}

void SPBECodegen::codegen_bitwise_shift(
        BinaryOp& node, ShiftOp op, bool assign) {
    spbe::Value* loc = nullptr;
    spbe::Value* lhs = nullptr;
    spbe::Value* rhs = nullptr;

    if (assign) {
        m_vctx = LValue;
        node.get_lhs()->accept(*this);
        loc = m_temp;

        lhs = m_builder.build_load(static_cast<const spbe::PointerType*>(
            loc->get_type())->get_pointee(), loc);
    } else {
        m_vctx = RValue;
        node.get_lhs()->accept(*this);
        lhs = m_temp;
    }

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

    if (assign)
        m_builder.build_store(loc, m_temp);
}

void SPBECodegen::codegen_comparison(BinaryOp& node, ComparisonOp op) {
    spbe::Value* lhs = nullptr;
    spbe::Value* rhs = nullptr;

    m_vctx = RValue;
    node.get_lhs()->accept(*this);
    lhs = m_temp;

    m_vctx = RValue;
    node.get_rhs()->accept(*this);
    rhs = m_temp;

    const TypeUse& type = node.get_lhs()->get_type();
    switch (op) {
    case ComparisonOp::Equals:
        if (type->is_integer() || type->is_pointer()) {
            m_temp = m_builder.build_cmp_ieq(lhs, rhs);
        } else if (type->is_floating_point()) {
            m_temp = m_builder.build_cmp_oeq(lhs, rhs);
        }

        break;

    case ComparisonOp::NEquals:
        if (type->is_integer() || type->is_pointer()) {
            m_temp = m_builder.build_cmp_ine(lhs, rhs);
        } else if (type->is_floating_point()) {
            m_temp = m_builder.build_cmp_one(lhs, rhs);
        }

        break;

    case ComparisonOp::Less:
        if (type->is_signed_integer() || type->is_pointer()) {
            m_temp = m_builder.build_cmp_slt(lhs, rhs);
        } else if (type->is_unsigned_integer()) {
            m_temp = m_builder.build_cmp_ult(lhs, rhs);
        } else if (type->is_floating_point()) {
            m_temp = m_builder.build_cmp_olt(lhs, rhs);
        }
        
        break;

    case ComparisonOp::LessEquals:
        if (type->is_signed_integer() || type->is_pointer()) {
            m_temp = m_builder.build_cmp_sle(lhs, rhs);
        } else if (type->is_unsigned_integer()) {
            m_temp = m_builder.build_cmp_ule(lhs, rhs);
        } else if (type->is_floating_point()) {
            m_temp = m_builder.build_cmp_ole(lhs, rhs);
        }
        
        break;

    case ComparisonOp::Greater:
        if (type->is_signed_integer() || type->is_pointer()) {
            m_temp = m_builder.build_cmp_sgt(lhs, rhs);
        } else if (type->is_unsigned_integer()) {
            m_temp = m_builder.build_cmp_ugt(lhs, rhs);
        } else if (type->is_floating_point()) {
            m_temp = m_builder.build_cmp_ogt(lhs, rhs);
        }
        
        break;

    case ComparisonOp::GreaterEquals:
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

void SPBECodegen::codegen_logical_and(BinaryOp& node) {
    spbe::BasicBlock* right_bb = new spbe::BasicBlock();
    spbe::BasicBlock* merge_bb = new spbe::BasicBlock();

    spbe::Value* lhs = nullptr;
    spbe::Value* rhs = nullptr;

    m_vctx = RValue;
    node.get_lhs()->accept(*this);
    lhs = inject_bool_comparison(m_temp);

    spbe::BasicBlock* false_bb = m_builder.get_insert();
    m_builder.build_brif(lhs, right_bb, merge_bb);

    m_function->push_back(right_bb);
    m_builder.set_insert(right_bb);

    m_vctx = RValue;
    node.get_rhs()->accept(*this);
    rhs = inject_bool_comparison(m_temp);

    m_builder.build_jmp(merge_bb);

    spbe::BasicBlock* otherwise = m_builder.get_insert();
    m_function->push_back(merge_bb);
    m_builder.set_insert(merge_bb);

    spbe::Instruction* phi = m_builder.build_phi(spbe::Type::get_i1_type(m_graph));
    phi->add_incoming(m_graph, spbe::ConstantInt::get_false(m_graph), false_bb);
    phi->add_incoming(m_graph, rhs, otherwise);

    m_temp = phi;
}

void SPBECodegen::codegen_logical_or(BinaryOp& node) {
    spbe::BasicBlock* right_bb = new spbe::BasicBlock();
    spbe::BasicBlock* merge_bb = new spbe::BasicBlock();

    spbe::Value* lhs = nullptr;
    spbe::Value* rhs = nullptr;

    m_vctx = RValue;
    node.get_lhs()->accept(*this);
    lhs = inject_bool_comparison(m_temp);

    spbe::BasicBlock* true_bb = m_builder.get_insert();
    m_builder.build_brif(lhs, merge_bb, right_bb);

    m_function->push_back(right_bb);
    m_builder.set_insert(right_bb);

    m_vctx = RValue;
    node.get_rhs()->accept(*this);
    rhs = inject_bool_comparison(m_temp);

    m_builder.build_jmp(merge_bb);

    spbe::BasicBlock* otherwise = m_builder.get_insert();
    m_function->push_back(merge_bb);
    m_builder.set_insert(merge_bb);

    spbe::Instruction* phi = m_builder.build_phi(spbe::Type::get_i1_type(m_graph));
    phi->add_incoming(m_graph, spbe::ConstantInt::get_true(m_graph), true_bb);
    phi->add_incoming(m_graph, rhs, otherwise);

    m_temp = phi;
}

void SPBECodegen::visit(BinaryOp& node) {
    switch (node.get_operator()) {
    case BinaryOp::Assign:
        return codegen_assignment(node);

    case BinaryOp::Add:
        return codegen_addition(node, AddOp::Add, false);

    case BinaryOp::AddAssign:
        return codegen_addition(node, AddOp::Add, true);

    case BinaryOp::Sub:
        return codegen_addition(node, AddOp::Sub, false);

    case BinaryOp::SubAssign:
        return codegen_addition(node, AddOp::Sub, true);

    case BinaryOp::Mul:
        return codegen_multiplication(node, MulOp::Mul, false);

    case BinaryOp::MulAssign:
        return codegen_multiplication(node, MulOp::Mul, true);

    case BinaryOp::Div:
        return codegen_multiplication(node, MulOp::Div, false);

    case BinaryOp::DivAssign:
        return codegen_multiplication(node, MulOp::Div, true);

    case BinaryOp::Mod:
        return codegen_multiplication(node, MulOp::Mod, false);

    case BinaryOp::ModAssign:
        return codegen_multiplication(node, MulOp::Mod, true);

    case BinaryOp::And:
        return codegen_bitwise_arithmetic(node, BitwiseOp::And, false);

    case BinaryOp::AndAssign:
        return codegen_bitwise_arithmetic(node, BitwiseOp::And, true);
    
    case BinaryOp::Or:
        return codegen_bitwise_arithmetic(node, BitwiseOp::Or, false);
    
    case BinaryOp::OrAssign:
        return codegen_bitwise_arithmetic(node, BitwiseOp::Or, true);

    case BinaryOp::Xor:
        return codegen_bitwise_arithmetic(node, BitwiseOp::Xor, false);

    case BinaryOp::XorAssign:
        return codegen_bitwise_arithmetic(node, BitwiseOp::Xor, true);

    case BinaryOp::LeftShift:
        return codegen_bitwise_shift(node, ShiftOp::LShift, false);

    case BinaryOp::LeftShiftAssign:
        return codegen_bitwise_shift(node, ShiftOp::LShift, true);

    case BinaryOp::RightShift:
        return codegen_bitwise_shift(node, ShiftOp::RShift, false);

    case BinaryOp::RightShiftAssign:
        return codegen_bitwise_shift(node, ShiftOp::RShift, true);

    case BinaryOp::LogicAnd:
        return codegen_logical_and(node);

    case BinaryOp::LogicOr:
        return codegen_logical_or(node);

    case BinaryOp::Equals:
        return codegen_comparison(node, ComparisonOp::Equals);

    case BinaryOp::NotEquals:
        return codegen_comparison(node, ComparisonOp::NEquals);

    case BinaryOp::LessThan:
        return codegen_comparison(node, ComparisonOp::Less);

    case BinaryOp::LessThanEquals:
        return codegen_comparison(node, ComparisonOp::LessEquals);

    case BinaryOp::GreaterThan:
        return codegen_comparison(node, ComparisonOp::Greater);
        
    case BinaryOp::GreaterThanEquals:
        return codegen_comparison(node, ComparisonOp::GreaterEquals);

    case BinaryOp::Unknown:
        m_diags.fatal("unknown binary operator", node.get_span());
    }
}

void SPBECodegen::visit(UnaryOp& node) {
    Expr* expr = node.get_expr();

    switch (node.get_operator()) {
    case UnaryOp::Increment: {
        ValueContext ctx = m_vctx;
        m_vctx = LValue;
        expr->accept(*this);

        const spbe::Type* type = lower_type(node.get_type());
        spbe::Value* lval = m_temp;
        spbe::Value* pre = m_builder.build_load(type, lval);
        spbe::Constant* one = nullptr;

        if (type->is_integer_type()) {
            one = spbe::ConstantInt::get_one(
                m_graph, spbe::Type::get_i64_type(m_graph));

            m_temp = m_builder.build_iadd(pre, one);
        } else if (type->is_floating_point_type()) {
            one = spbe::ConstantFP::get_one(m_graph, type);

            m_temp = m_builder.build_fadd(pre, one);
        } else if (type->is_pointer_type()) {
            one = spbe::ConstantInt::get_one(
                m_graph, spbe::Type::get_i64_type(m_graph));

            m_temp = m_builder.build_ap(type, pre, one);
        } else {
            assert(false && "type incompatible with increment operator!");
        }

        m_builder.build_store(m_temp, lval);

        if (node.is_postfix())
            m_temp = pre;

        return;
    }

    case UnaryOp::Decrement: {
        ValueContext ctx = m_vctx;
        m_vctx = LValue;
        expr->accept(*this);

        const spbe::Type* type = lower_type(node.get_type());
        spbe::Value* lval = m_temp;
        spbe::Value* pre = m_builder.build_load(type, lval);
        spbe::Constant* one = nullptr;

        if (type->is_integer_type()) {
            one = spbe::ConstantInt::get_one(
                m_graph, spbe::Type::get_i64_type(m_graph));

            m_temp = m_builder.build_isub(pre, one);
        } else if (type->is_floating_point_type()) {
            one = spbe::ConstantFP::get_one(m_graph, type);

            m_temp = m_builder.build_fsub(pre, one);
        } else if (type->is_pointer_type()) {
            one = spbe::ConstantInt::get(
                m_graph, spbe::Type::get_i64_type(m_graph), -1.f);

            m_temp = m_builder.build_ap(type, pre, one);
        } else {
            assert(false && "type incompatible with increment operator!");
        }

        m_builder.build_store(m_temp, lval);

        if (node.is_postfix())
            m_temp = pre;

        return;
    }

    case UnaryOp::AddressOf: {
        m_vctx = LValue;
        node.get_expr()->accept(*this);
        return;
    }
    
    case UnaryOp::Dereference: {
        ValueContext ctx = m_vctx;
        m_vctx = RValue;
        expr->accept(*this);

        if (ctx == RValue) {
            const spbe::Type* type = lower_type(node.get_type());
            m_temp = m_builder.build_load(type, m_temp);
        }

        return;
    }

    case UnaryOp::Negate: {
        m_vctx = RValue;
        expr->accept(*this);

        const spbe::Type* type = m_temp->get_type();

        if (type->is_integer_type()) {
            if (auto CI = dynamic_cast<const spbe::ConstantInt*>(m_temp)) {
                m_temp = spbe::ConstantInt::get(m_graph, type, -CI->get_value());
            } else {
                m_temp = m_builder.build_ineg(m_temp);
            }
        } else if (type->is_floating_point_type()) {
            if (auto CF = dynamic_cast<const spbe::ConstantFP*>(m_temp)) {
                m_temp = spbe::ConstantFP::get(m_graph, type, -CF->get_value());
            } else {
                m_temp = m_builder.build_fneg(m_temp);
            }
        } else {
            assert(false && "type incompatible with negate operator!");
        }

        return;
    }

    case UnaryOp::Not: {
        m_vctx = RValue;
        expr->accept(*this);

        const spbe::Type* type = m_temp->get_type();

        if (type->is_integer_type()) {
            if (auto CI = dynamic_cast<const spbe::ConstantInt*>(m_temp)) {
                m_temp = spbe::ConstantInt::get(m_graph, type, ~CI->get_value());
            } else {
                m_temp = m_builder.build_not(m_temp);
            }
        } else {
            assert(false && "type incompatible with bitwise not operator!");
        }

        return;
    }

    case UnaryOp::LogicNot: {
        m_vctx = RValue;
        expr->accept(*this);

        const spbe::Type* type = m_temp->get_type();

        if (type->is_integer_type()) {
            if (auto CI = dynamic_cast<const spbe::ConstantInt*>(m_temp)) {
                m_temp = spbe::ConstantInt::get(
                    m_graph, spbe::Type::get_i1_type(m_graph), !CI->get_value());
            } else {
                m_temp = m_builder.build_cmp_ine(
                    m_temp, spbe::ConstantInt::get_zero(m_graph, type));
            }
        } else if (type->is_floating_point_type()) {
            if (auto CF = dynamic_cast<const spbe::ConstantFP*>(m_temp)) {
                m_temp = spbe::ConstantInt::get(
                    m_graph, spbe::Type::get_i1_type(m_graph), !CF->get_value());
            } else { 
                m_temp = m_builder.build_cmp_one(
                    m_temp, spbe::ConstantFP::get_zero(m_graph, type));
            }
        } else if (type->is_pointer_type()) {
            if (auto CN = dynamic_cast<const spbe::ConstantNull*>(m_temp)) {
                m_temp = spbe::ConstantInt::get_true(m_graph);
            } else {
                m_temp = m_builder.build_cmp_ine(
                    m_temp, spbe::ConstantNull::get(m_graph, type));
            }
        } else {
            assert(false && "type incompatible with logical not operator!");
        }

        return;
    }

    case UnaryOp::Unknown:
        m_diags.fatal("unknown unary operator", node.get_span());
    }
}

void SPBECodegen::codegen_cast_integer(
        spbe::Value* value, const spbe::Type* dst, bool is_signed) {
    if (dst->is_integer_type()) {
        const spbe::Target& target = m_graph.get_target();
        uint32_t src_size = target.get_type_size(value->get_type());
        uint32_t dst_size = target.get_type_size(dst);

        if (auto CI = dynamic_cast<spbe::ConstantInt*>(value)) {
            m_temp = spbe::ConstantInt::get(m_graph, dst, CI->get_value());
        } else if (src_size > dst_size) {
            m_temp = m_builder.build_itrunc(dst, value);
        } else if (src_size) {
            if (is_signed) {
            m_temp = m_builder.build_sext(dst, value);
            } else {
                m_temp = m_builder.build_zext(dst, value);
            }
        }
    } else if (dst->is_floating_point_type()) {
        if (auto CI = dynamic_cast<spbe::ConstantInt*>(value)) {
            m_temp = spbe::ConstantFP::get(m_graph, dst, CI->get_value());
        } else if (is_signed) {
            m_temp = m_builder.build_si2fp(dst, value);
        } else {
            m_temp = m_builder.build_ui2fp(dst, value);
        }
    } else if (dst->is_pointer_type()) {
        m_temp = m_builder.build_i2p(dst, value);
    } else {
        assert(false && "unsupported integer type cast!");
    }
}

void SPBECodegen::codegen_cast_float(
        spbe::Value* value, const spbe::Type* dst) {
    if (dst->is_integer_type()) {
        m_temp = m_builder.build_fp2si(dst, value);
    } else if (dst->is_floating_point_type()) {
        const spbe::Target& target = m_graph.get_target();
        uint32_t src_size = target.get_type_size(value->get_type());
        uint32_t dst_size = target.get_type_size(dst);

        if (auto CF = dynamic_cast<spbe::ConstantFP*>(value)) {
            m_temp = spbe::ConstantFP::get(m_graph, dst, CF->get_value());
        } else if (src_size > dst_size) {
            m_temp = m_builder.build_ftrunc(dst, value);
        } else if (src_size < dst_size) {
            m_temp = m_builder.build_fext(dst, value);
        }
    } else {
        assert(false && "unsupported float type cast!");
    }
}

void SPBECodegen::codegen_cast_array(
        spbe::Value* value, const spbe::Type* dst) {
    if (dst->is_pointer_type()) {
        m_temp = m_builder.build_reint(dst, value);
    } else {
        assert(false && "unsupported array type cast!");
    }
}

void SPBECodegen::codegen_cast_pointer(
        spbe::Value* value, const spbe::Type* dst) {
    if (dst->is_integer_type()) {
        m_temp = m_builder.build_p2i(dst, value);
    } else if (dst->is_pointer_type()) {
        if (dynamic_cast<spbe::ConstantNull*>(value)) {
            m_temp = spbe::ConstantNull::get(m_graph, dst);
        } else {
            m_temp = m_builder.build_reint(dst, value);
        }
    } else {
        assert(false && "unsupported pointer type cast!");
    }
}

void SPBECodegen::visit(CastExpr& node) {
    m_vctx = RValue;
    node.get_expr()->accept(*this);
    
    const spbe::Type* src = m_temp->get_type();
    const spbe::Type* dst = lower_type(node.get_type());

    // Skip casting if the types are identical.
    if (*src == *dst)
        return;

    if (src->is_integer_type()) {
        return codegen_cast_integer(
            m_temp, dst, node.get_expr()->get_type()->is_signed_integer());
    } else if (src->is_floating_point_type()) {
        return codegen_cast_float(m_temp, dst);
    } else if (src->is_array_type()) {
        return codegen_cast_array(m_temp, dst);
    } else if (src->is_pointer_type()) {
        return codegen_cast_pointer(m_temp, dst);
    } else {
        assert(false && "unsupported type cast!");
    }
}

void SPBECodegen::visit(ParenExpr& node) {
    node.get_expr()->accept(*this);
}

void SPBECodegen::visit(SizeofExpr& node) {
    const spbe::Target& target = m_graph.get_target();

    m_temp = spbe::ConstantInt::get(
        m_graph, 
        lower_type(node.get_type()), 
        target.get_type_size(lower_type(node.get_target_type())));
}

void SPBECodegen::visit(AccessExpr& node) {

}

void SPBECodegen::visit(SubscriptExpr& node) {
    ValueContext ctx = m_vctx;
    spbe::Value* base = nullptr;
    spbe::Value* index = nullptr;
    const spbe::Type* type = lower_type(node.get_type());

    m_vctx = LValue;
    if (node.get_base()->get_type()->is_pointer()) {
        // If the base is a pointer, then we need to rid one level of 
        // indirection to get at the underlying elements.
        m_vctx = RValue;
    }

    node.get_base()->accept(*this);
    base = m_temp;

    m_vctx = RValue;
    node.get_index()->accept(*this);
    index = m_temp;

    m_temp = m_builder.build_ap(
        spbe::PointerType::get(m_graph, type), base, index);

    if (ctx == RValue)
        m_temp = m_builder.build_load(type, m_temp);
}

void SPBECodegen::visit(DeclRefExpr& node) {
    if (auto variant = dynamic_cast<const VariantDecl*>(node.get_decl())) {
        m_temp = spbe::ConstantInt::get(
            m_graph, 
            lower_type(variant->get_type()), 
            variant->get_value());
        
        return;
    } else if (auto variable = dynamic_cast<const VariableDecl*>(node.get_decl())) {
        if (variable->is_global()) {
            m_temp = m_graph.get_global(node.get_name());
            assert(m_temp && "global not declared for variable!");
        } else {
            m_temp = m_function->get_local(node.get_name());
            assert(m_temp && "local not declared for variable!");
        }

        if (m_vctx == RValue)
            m_temp = m_builder.build_load(lower_type(node.get_type()), m_temp);
    } else if (auto param = dynamic_cast<const ParameterDecl*>(node.get_decl())) {
        m_temp = m_function->get_local(node.get_name());
        assert(m_temp && "local not declared for parameter!");

        if (m_vctx == RValue)
            m_temp = m_builder.build_load(lower_type(node.get_type()), m_temp);
    } else if (auto function = dynamic_cast<const FunctionDecl*>(node.get_decl())) {
        m_temp = m_graph.get_function(node.get_name());
        assert(m_vctx != RValue && 
            "cannot produce function reference as an rvalue");
    }
}

void SPBECodegen::visit(CallExpr& node) {
    m_vctx = LValue;
    node.get_callee()->accept(*this);
    assert(m_temp && "callee does not produce a value!");
    spbe::Value* callee = m_temp;

    vector<spbe::Value*> args(node.num_args(), nullptr);

    for (uint32_t i = 0, e = node.num_args(); i < e; ++i) {
        m_vctx = RValue;
        node.get_arg(i)->accept(*this);
        assert(m_temp && "call argument does not produce a value!");
        args.push_back(m_temp);
    }

    m_temp = m_builder.build_call(
        dynamic_cast<const spbe::FunctionType*>(callee->get_type()), 
        callee, 
        args);
}

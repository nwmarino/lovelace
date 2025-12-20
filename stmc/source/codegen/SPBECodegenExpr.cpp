//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/codegen/SPBECodegen.hpp"
#include "stmc/tree/Decl.hpp"

#include "spbe/graph/Constant.hpp"
#include "spbe/target/Target.hpp"

using namespace stm;

void SPBECodegen::gen_binary_assign(const BinaryOp& op) {

}

void SPBECodegen::gen_binary_add(const BinaryOp& op) {

}

void SPBECodegen::gen_binary_mul(const BinaryOp& op) {

}

void SPBECodegen::gen_binary_mod(const BinaryOp& op) {

}

void SPBECodegen::gen_binary_bitwise_arithmetic(const BinaryOp& op) {

}

void SPBECodegen::gen_binary_numerical_cmp(const BinaryOp& op) {

}

void SPBECodegen::gen_binary_bitwise_cmp(const BinaryOp& op) {

}

void SPBECodegen::gen_binary_logical_and(const BinaryOp& op) {

}

void SPBECodegen::gen_binary_logical_or(const BinaryOp& op) {

}
    
void SPBECodegen::gen_unary_addition(const UnaryOp& op) {

}

void SPBECodegen::gen_unary_memory(const UnaryOp& op) {

}

void SPBECodegen::gen_unary_negation(const UnaryOp& op) {

}

void SPBECodegen::gen_unary_bitwise_not(const UnaryOp& op) {

}

void SPBECodegen::gen_unary_logical_not(const UnaryOp& op) {

}

void SPBECodegen::visit(BoolLiteral& node) {
    m_temp = spbe::ConstantInt::get(
        m_graph, 
        lower_type_to_spbe(node.get_type()), 
        static_cast<int64_t>(node.get_value()));
}

void SPBECodegen::visit(IntegerLiteral& node) {
    m_temp = spbe::ConstantInt::get(
        m_graph, 
        lower_type_to_spbe(node.get_type()), 
        node.get_value());
}

void SPBECodegen::visit(FPLiteral& node) {
    m_temp = spbe::ConstantFP::get(
        m_graph, 
        lower_type_to_spbe(node.get_type()), 
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
        lower_type_to_spbe(node.get_type()));
}

void SPBECodegen::visit(BinaryOp& node) {
    switch (node.get_operator()) {
    case BinaryOp::Assign:
        return gen_binary_assign(node);

    case BinaryOp::Add:
    case BinaryOp::AddAssign:
    case BinaryOp::Sub:
    case BinaryOp::SubAssign:
        return gen_binary_add(node);

    case BinaryOp::Mul:
    case BinaryOp::MulAssign:
    case BinaryOp::Div:
    case BinaryOp::DivAssign:
        return gen_binary_mul(node);

    case BinaryOp::Mod:
    case BinaryOp::ModAssign:
        return gen_binary_mod(node);

    case BinaryOp::And:
    case BinaryOp::AndAssign:
    case BinaryOp::Or:
    case BinaryOp::OrAssign:
    case BinaryOp::Xor:
    case BinaryOp::XorAssign:
        return gen_binary_bitwise_cmp(node);

    case BinaryOp::LeftShift:
    case BinaryOp::LeftShiftAssign:
    case BinaryOp::RightShift:
    case BinaryOp::RightShiftAssign:
        return gen_binary_bitwise_arithmetic(node);

    case BinaryOp::LogicAnd:
        return gen_binary_logical_and(node);

    case BinaryOp::LogicOr:
        return gen_binary_logical_or(node);

    case BinaryOp::Equals:
    case BinaryOp::NotEquals:
    case BinaryOp::LessThan:
    case BinaryOp::LessThanEquals:
    case BinaryOp::GreaterThan:
    case BinaryOp::GreaterThanEquals:
        return gen_binary_numerical_cmp(node);

    case BinaryOp::Unknown:
        m_diags.fatal("unknown binary operator", node.get_span());
    }
}

void SPBECodegen::visit(UnaryOp& node) {
    switch (node.get_operator()) {
    case UnaryOp::Increment:
    case UnaryOp::Decrement:
        return gen_unary_addition(node);

    case UnaryOp::AddressOf:
    case UnaryOp::Dereference:
        return gen_unary_memory(node);

    case UnaryOp::Negate:
        return gen_unary_negation(node);

    case UnaryOp::Not:
        return gen_unary_bitwise_not(node);

    case UnaryOp::LogicNot:
        return gen_unary_logical_not(node);

    case UnaryOp::Unknown:
        m_diags.fatal("unknown unary operator", node.get_span());
    }
}

void SPBECodegen::visit(CastExpr& node) {

}

void SPBECodegen::visit(ParenExpr& node) {
    node.get_expr()->accept(*this);
}

void SPBECodegen::visit(SizeofExpr& node) {
    const spbe::Target& target = m_graph.get_target();

    m_temp = spbe::ConstantInt::get(
        m_graph, 
        lower_type_to_spbe(node.get_type()), 
        target.get_type_size(lower_type_to_spbe(node.get_target_type())));
}

void SPBECodegen::visit(AccessExpr& node) {

}

void SPBECodegen::visit(SubscriptExpr& node) {

}

void SPBECodegen::visit(DeclRefExpr& node) {
    if (auto variant = dynamic_cast<const VariantDecl*>(node.get_decl())) {
        m_temp = spbe::ConstantInt::get(
            m_graph, 
            lower_type_to_spbe(variant->get_type()), 
            variant->get_value());
        
        return;
    } else if (auto variable = dynamic_cast<const VariableDecl*>(node.get_decl())) {
        // @Todo: check if variable is a global, and get it from graph globals.

        m_temp = m_function->get_local(node.get_name());
        assert(m_temp && "local not declared yet!");

        if (m_vctx == RValue)
            m_temp = m_builder.build_load(lower_type_to_spbe(node.get_type()), m_temp);
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

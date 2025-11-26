//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "scc/ast/Codegen.hpp"
#include "scc/ast/Decl.hpp"
#include "scc/ast/Expr.hpp"
#include "scc/ast/Stmt.hpp"
#include "scc/ast/Type.hpp"
#include "scc/ast/Decl.hpp"
#include "scc/core/Logger.hpp"

#include "spbe/graph/Constant.hpp"
#include "spbe/graph/Function.hpp"
#include "spbe/graph/Local.hpp"
#include "spbe/graph/Type.hpp"
#include "spbe/target/Target.hpp"

#include <cassert>

using namespace scc;

Codegen::Codegen(TranslationUnitDecl* unit, spbe::CFG& graph) 
    : m_unit(unit), m_graph(graph), m_builder(graph) {}

const spbe::Type* Codegen::lower_type(const QualType& type) {
    assert(type.get_type() && "type cannot be null!");
    
    switch (type->get_kind()) {
    case Type::Builtin: {
        switch (static_cast<const BuiltinType*>(type.get_type())->get_builtin_kind()) {
        case BuiltinType::Void:
            return nullptr;
        case BuiltinType::Char:
        case BuiltinType::UChar:
            return spbe::IntegerType::get_i8_type(m_graph);
        case BuiltinType::Short:
        case BuiltinType::UShort:
            return spbe::IntegerType::get_i16_type(m_graph);
        case BuiltinType::Int:
        case BuiltinType::UInt:
            return spbe::IntegerType::get_i32_type(m_graph);
        case BuiltinType::Long:
        case BuiltinType::ULong:
        case BuiltinType::LongLong:
        case BuiltinType::ULongLong:
            return spbe::IntegerType::get_i64_type(m_graph);
        case BuiltinType::Float:
            return spbe::FloatType::get_f32_type(m_graph);
        case BuiltinType::Double:
        case BuiltinType::LongDouble:
            return spbe::FloatType::get_f64_type(m_graph);
        }
    }

    case Type::Array:
        return spbe::PointerType::get(m_graph, lower_type(
            static_cast<const ArrayType*>(type.get_type())->get_element()));
    
    case Type::Pointer:
        return spbe::PointerType::get(m_graph, lower_type(
            static_cast<const PointerType*>(type.get_type())->get_pointee()));

    case Type::Typedef:
        return lower_type(
            static_cast<const TypedefType*>(type.get_type())->get_underlying());

    case Type::Record:
        return spbe::StructType::get(m_graph, 
            static_cast<const RecordType*>(type.get_type())->get_decl()->get_name());

    case Type::Enum:
        return spbe::IntegerType::get_i32_type(m_graph);
    
    default:
        assert(false && "no SPBE equivelant!");
    }
}

spbe::Value* Codegen::inject_bool_comparison(spbe::Value* value) {
    if (value->get_type()->is_integer_type(1)) {
        return value;
    } else if (value->get_type()->is_integer_type()) {
        return m_builder.build_cmp_ine(
            value, spbe::ConstantInt::get_zero(m_graph, value->get_type()));
    } else if (value->get_type()->is_floating_point_type()) {
        return m_builder.build_cmp_one(
            value, spbe::ConstantFP::get_zero(m_graph, value->get_type()));
    } else if (value->get_type()->is_pointer_type()) {
        return m_builder.build_cmp_ine(
            value, spbe::ConstantNull::get(m_graph, value->get_type()));
    }
    
    assert(false && "incompatible as boolean value!");
}

void Codegen::visit(TranslationUnitDecl &node) {
    assert(m_unit == &node && 
        "unit is not the same as the one this sema is initialized with!");

    m_phase = Declare;
    for (const auto& tag : node.get_tags())
        tag->accept(*this);

    for (const auto& decl : node.get_decls())
        decl->accept(*this);

    m_phase = Define;
    for (const auto& tag : node.get_tags())
        tag->accept(*this);

    for (const auto& decl : node.get_decls())
        decl->accept(*this);
}

void Codegen::visit(VariableDecl &node) {
    
}

void Codegen::visit(FunctionDecl &node) {
    if (is_declare_phase()) {
        spbe::Function::LinkageType linkage = spbe::Function::Internal;
        if (node.is_main() || node.is_extern())
            linkage = spbe::Function::External;

        vector<const spbe::Type*> arg_types(node.num_params(), nullptr);
        vector<spbe::Argument*> args(node.num_params(), nullptr);

        for (uint32_t i = 0, e = node.num_params(); i != e; ++i) {
            const spbe::Type* a_type = lower_type(node.get_param_type(i));
            arg_types[i] = a_type;

            args[i] = new spbe::Argument(
                a_type, node.get_param(i)->get_name(), i, nullptr);
        }

        const spbe::FunctionType* type = spbe::FunctionType::get(
            m_graph, arg_types, lower_type(node.get_return_type()));

        new spbe::Function(m_graph, linkage, type, node.get_name(), args);
    } else if (is_define_phase()) {
        spbe::Function* func = m_func = m_graph.get_function(node.get_name());
        assert(func);

        if (!node.has_body())
            return;

        spbe::BasicBlock* entry = new spbe::BasicBlock(func);
        m_builder.set_insert(entry);

        for (uint32_t i = 0, e = node.num_params(); i != e; ++i) {
            spbe::Argument* arg = func->get_arg(i);
            spbe::Local* local = new spbe::Local(
                m_graph, 
                arg->get_type(), 
                m_graph.get_target().get_type_align(arg->get_type()), 
                node.get_param(i)->get_name(), 
                func);

            m_builder.build_store(arg, local);
        }

        node.get_body()->accept(*this);

        if (!m_builder.get_insert()->terminates()) {
            if (node.get_return_type()->is_void()) {
                m_builder.build_ret_void();
            } else {
                Logger::error("function '" + node.get_name() 
                    + "' does not always return", node.get_span());
            }
        }

        m_func = nullptr;
        m_builder.clear_insert();
    }
}

void Codegen::visit(RecordDecl &node) {
    if (is_declare_phase() && node.is_struct()) {
        spbe::StructType* s_type = spbe::StructType::get(m_graph, node.get_name());
        assert(s_type && "shell structure type not created!");

        for (const auto& field : node.get_fields())
            s_type->append_field(lower_type(field->get_type()));

    }
    
    // TODO: Implement unions.
}

void Codegen::visit(CompoundStmt &node) {
    for (const auto& stmt : node.get_stmts())
        stmt->accept(*this);
}

void Codegen::visit(DeclStmt &node) {
    for (const auto& decl : node.get_decls())
        decl->accept(*this);
}

void Codegen::visit(ExprStmt &node) {
    node.get_expr()->accept(*this);
}

void Codegen::visit(IfStmt &node) {
    m_vctx = RValue;
    node.get_cond()->accept(*this);
    assert(m_temp);

    spbe::BasicBlock* then_bb = new spbe::BasicBlock(m_func);
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

    // Jump to the merge block if the then body does not terminate on its own.
    if (!m_builder.get_insert()->terminates())
        m_builder.build_jmp(merge_bb);

    if (node.has_else()) {
        m_func->push_back(else_bb);
        m_builder.set_insert(else_bb);
        node.get_else()->accept(*this);

        // Jump to the merge block if the else body does not terminate on its
        // own.
        if (!m_builder.get_insert()->terminates())
            m_builder.build_jmp(merge_bb);
    }

    if (merge_bb->has_preds()) {
        // If the merge block has predecessors, then keep it.
        m_func->push_back(merge_bb);
        m_builder.set_insert(merge_bb);
    } else {
        delete merge_bb;
    }
}

void Codegen::visit(ReturnStmt &node) {
    if (m_builder.get_insert()->terminates())
        return;

    if (node.has_expr()) {
        m_vctx = RValue;
        node.get_expr()->accept(*this);
        assert(m_temp);

        m_builder.build_ret(m_temp);
        m_temp = nullptr;
    } else {
        m_builder.build_ret_void();
    }
}

void Codegen::visit(BreakStmt &node) {
    if (m_builder.get_insert()->terminates())
        return;

    // Jump to the parent merge block (i.e. the code after a loop).
    assert(m_merge);
    m_builder.build_jmp(m_merge);
}

void Codegen::visit(ContinueStmt &node) {
    if (m_builder.get_insert()->terminates())
        return;

    // Jump to the parent condition block (i.e. the loop condition).
    assert(m_condition);
    m_builder.build_jmp(m_condition);
}

void Codegen::visit(WhileStmt &node) {
    spbe::BasicBlock* cond_bb = new spbe::BasicBlock(m_func);
    spbe::BasicBlock* body_bb = nullptr;
    spbe::BasicBlock* merge_bb = new spbe::BasicBlock();

    m_builder.build_jmp(cond_bb);

    m_builder.set_insert(cond_bb);
    m_vctx = RValue;
    node.get_cond()->accept(*this);
    assert(m_temp);

    if (node.has_body()) {
        body_bb = new spbe::BasicBlock();
        m_builder.build_brif(m_temp, body_bb, merge_bb);

        m_func->push_back(body_bb);
        m_builder.set_insert(body_bb);

        spbe::BasicBlock* prev_condition = m_condition;
        spbe::BasicBlock* prev_merge = m_merge;
        m_condition = cond_bb;
        m_merge = merge_bb;

        node.get_body()->accept(*this);

        if (!m_builder.get_insert()->terminates())
            m_builder.build_jmp(cond_bb);

        m_condition = prev_condition;
        m_merge = prev_merge;
    } else {
        m_builder.build_brif(m_temp, cond_bb, merge_bb);
    }

    m_func->push_back(merge_bb);
    m_builder.set_insert(merge_bb);
}

void Codegen::visit(ForStmt &node) {

}

void Codegen::visit(CaseStmt &node) {

}

void Codegen::visit(SwitchStmt &node) {

}

void Codegen::visit(IntegerLiteral& node) {
    m_temp = spbe::ConstantInt::get(
        m_graph, lower_type(node.get_type()), node.get_value());
}

void Codegen::visit(FPLiteral& node) {
    m_temp = spbe::ConstantInt::get(
        m_graph, lower_type(node.get_type()), node.get_value());
}

void Codegen::visit(CharLiteral& node) {
    m_temp = spbe::ConstantInt::get(
        m_graph, spbe::IntegerType::get_i8_type(m_graph), node.get_value());
}

void Codegen::visit(StringLiteral& node) {
    m_temp = m_builder.build_string(
        spbe::ConstantString::get(m_graph, node.get_value()));
}

void Codegen::visit(BinaryExpr &node) {

}

void Codegen::visit(UnaryExpr &node) {

}

void Codegen::visit(ParenExpr &node) {
    node.get_expr()->accept(*this);
}

void Codegen::visit(RefExpr &node) {
    if (auto variant = dynamic_cast<const EnumVariantDecl*>(node.get_decl())) {
        m_temp = spbe::ConstantInt::get(
            m_graph, 
            spbe::IntegerType::get_i32_type(m_graph), 
            variant->get_value());

        return;
    }

    auto variable = dynamic_cast<const VariableDecl*>(node.get_decl());
    if (variable->is_global()) {
        spbe::Global* global = m_graph.get_global(node.get_name());
        assert(global);
        m_temp = global;
    } else {
        spbe::Local* local = m_func->get_local(node.get_name());
        assert(local);
        m_temp = local;
    }

    if (m_vctx == RValue)
        m_temp = m_builder.build_load(lower_type(node.get_type()), m_temp);
}

void Codegen::visit(CallExpr &node) {
    m_vctx = LValue;
    node.get_callee()->accept(*this);
    assert(m_temp);

    spbe::Value* callee = m_temp;

    vector<spbe::Value*> args(node.num_args(), nullptr);
    for (uint32_t i = 0, e = node.num_args(); i != e; ++i) {
        m_vctx = RValue;
        node.get_arg(i)->accept(*this);
        assert(m_temp);
        args[i] = m_temp;
    }

    const spbe::FunctionType* f_type = dynamic_cast<const spbe::FunctionType*>(
        lower_type(node.get_callee()->get_type()));

    m_temp = m_builder.build_call(f_type, callee, args);
}

void Codegen::visit(CastExpr &node) {

}

void Codegen::visit(SizeofExpr &node) {
    const spbe::Type* t_type = lower_type(node.get_target());
    uint32_t size = m_graph.get_target().get_type_size(t_type);

    m_temp = spbe::ConstantInt::get(
        m_graph, spbe::IntegerType::get_i64_type(m_graph), size);
}

void Codegen::visit(SubscriptExpr &node) {
    ValueContext vctx = m_vctx;
    spbe::Value* base = nullptr;
    spbe::Value* index = nullptr;
    const spbe::Type* type = lower_type(node.get_type());

    m_vctx = LValue;
    if (node.get_base()->get_type()->get_kind() == Type::Pointer)
        m_vctx = RValue;

    node.get_base()->accept(*this);
    assert(m_temp);
    base = m_temp;

    m_vctx = RValue;
    node.get_index()->accept(*this);
    assert(m_temp);
    index = m_temp;
    
    m_temp = m_builder.build_ap(
        spbe::PointerType::get(m_graph, type), base, index);

    m_temp = (vctx == LValue) ? m_temp : m_builder.build_load(type, m_temp);
}

void Codegen::visit(MemberExpr &node) {

}

void Codegen::visit(TernaryExpr &node) {
    spbe::Value* cond = nullptr;
    spbe::Value* tval = nullptr;
    spbe::Value* fval = nullptr;
    
    m_vctx = RValue;
    node.get_cond()->accept(*this);
    assert(m_temp);
    cond = m_temp;

    m_vctx = RValue;
    node.get_true_value()->accept(*this);
    assert(m_temp);
    tval = m_temp;

    m_vctx = RValue;
    node.get_false_value()->accept(*this);
    assert(m_temp);
    fval = m_temp;
    
    m_temp = m_builder.build_select(cond, tval, fval);
}

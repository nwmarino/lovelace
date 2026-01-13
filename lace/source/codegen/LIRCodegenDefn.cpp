//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lace/codegen/LIRCodegen.hpp"
#include "lace/core/Diagnostics.hpp"
#include "lace/tree/Defn.hpp"
#include "lace/tree/Type.hpp"

using namespace lace;

void Codegen::declare_ir_global(VariableDefn& node) {
    lir::Global::LinkageType linkage = lir::Global::Internal;
    if (node.has_rune(Rune::Public))
        linkage = lir::Global::External;

    lir::Global::create(
        m_cfg, 
        lower_type(node.get_type()), 
        linkage, 
        !node.get_type().is_mut(), 
        node.get_name());
}

void Codegen::define_ir_global(VariableDefn& node) {
    if (!node.has_init())
        return;

    lir::Global* global = m_cfg.get_global(node.get_name());
    assert(global && "global does not exist!");

    m_vctx = RValue;
    node.get_init()->accept(*this);
    assert(m_temp && "initializer does not produce a value!");
    
    lir::Constant* init = dynamic_cast<lir::Constant*>(m_temp);
    assert(init && "global is not initialized with a constant!");
    global->set_initializer(init);
}

void Codegen::declare_ir_function(FunctionDefn& node) {
    lir::Function::LinkageType linkage = lir::Function::Internal;
    if (node.has_rune(Rune::Public))
        linkage = lir::Function::External;

    std::vector<lir::Type*> types(node.num_params(), nullptr);
    std::vector<lir::Function::Arg*> args(node.num_params(), nullptr);

    for (uint32_t i = 0; i < node.num_params(); ++i) {
        const ParameterDefn* param = node.get_param(i);
        lir::Type* type = lower_type(param->get_type());

        std::string name = param->get_name();
        if (name == "_")
            name = "";

        types[i] = type;
        args[i] = lir::Function::Arg::create(type, name);
    }

    lir::FunctionType* type = lir::FunctionType::get(
        m_cfg, types, lower_type(node.get_return_type()));

    lir::Function::create(m_cfg, linkage, type, node.get_name(), args);
}

void Codegen::define_ir_function(FunctionDefn& node) {
    if (!node.has_body())
        return;

    m_function = m_cfg.get_function(node.get_name());
    assert(m_function && "function does not exist!");

    lir::BasicBlock* entry = lir::BasicBlock::create({}, m_function);
    m_builder.set_insert(entry);

    for (uint32_t i = 0, e = node.num_params(); i < e; ++i) {
        lir::Function::Arg* arg = m_function->get_arg(i);
        lir::Local* local = lir::Local::create(
            m_cfg, 
            arg->get_type(), 
            arg->get_name(), 
            m_mach.get_align(arg->get_type()), 
            m_function);

        m_builder.build_store(arg, local, m_mach.get_align(arg->get_type()));
    }

    node.get_body()->accept(*this);

    if (!m_builder.get_insert()->terminates()) {
        if (m_function->get_return_type()->is_void_type()) {
            m_builder.build_ret();
        } else {
            log::warn("function does not always return", 
                log::Span(m_cfg.get_filename(), node.get_span()));
        }
    }

    m_function = nullptr;
    m_builder.clear_insert();
}

void Codegen::declare_ir_structure(StructDefn& node) {
    lir::StructType::create(m_cfg, node.get_name(), {});
}

void Codegen::define_ir_structure(StructDefn& node) {
    lir::StructType* type = lir::StructType::get(m_cfg, node.get_name());
    assert(type && "type does not exist!");

    for (FieldDefn* field : node.get_fields())
        type->append_field(lower_type(field->get_type()));
}

void Codegen::visit(VariableDefn& node) {
    if (node.is_global()) {
        if (m_phase == Declare) {
            return declare_ir_global(node);
        } else if (m_phase == Define) {
            return define_ir_global(node);
        }
    }

    lir::Type* type = lower_type(node.get_type());
    lir::Local* local = lir::Local::create(
        m_cfg, 
        type, 
        node.get_name(), 
        m_mach.get_align(type),
        m_function);

    if (node.has_init()) {
        m_vctx = RValue;
        node.get_init()->accept(*this);
        assert(m_temp && "initializer does not produce a value!");

        m_builder.build_store(m_temp, local, m_mach.get_align(type));
    }
}

void Codegen::visit(FunctionDefn& node) {
    if (m_phase == Declare) {
        declare_ir_function(node);
    } else if (m_phase == Define) {
        define_ir_function(node);
    }
}

void Codegen::visit(StructDefn& node) {
    if (m_phase == Declare) {
        declare_ir_structure(node);
    } else if (m_phase == Define) {
        define_ir_structure(node);
    }
}

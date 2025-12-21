//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "spbe/graph/Global.hpp"
#include "stmc/codegen/SPBECodegen.hpp"
#include "stmc/tree/Decl.hpp"

#include "spbe/graph/Type.hpp"
#include "spbe/graph/BasicBlock.hpp"
#include "spbe/graph/Function.hpp"
#include "spbe/graph/Local.hpp"
#include "spbe/target/Target.hpp"

using namespace stm;

void SPBECodegen::declare_spbe_global(VariableDecl& decl) {
    const spbe::Type* type = lower_type(decl.get_type());

    spbe::Global::LinkageType linkage = spbe::Global::Internal;

    spbe::Global* GL = new spbe::Global(
        m_graph,
        type,
        linkage,
        !decl.get_type().is_mut(),
        decl.get_name());
}

void SPBECodegen::define_spbe_global(VariableDecl& decl) {
    if (!decl.has_init())
        return;    

    spbe::Global* GL = m_graph.get_global(decl.get_name());
    assert(GL && "global has not been defined yet!");

    m_vctx = RValue;
    decl.get_init()->accept(*this);
    assert(m_temp && "global variable initializer does not produce a value!");

    spbe::Constant* init = dynamic_cast<spbe::Constant*>(m_temp);
    assert(init && "global variable initializer is not a constant!");

    GL->set_initializer(init);
}

void SPBECodegen::declare_spbe_function(FunctionDecl& decl) {
    spbe::Function::LinkageType linkage = spbe::Function::External;

    vector<const spbe::Type*> arg_types(decl.num_params(), nullptr);
    vector<spbe::Argument*> args(decl.num_params(), nullptr);

    for (uint32_t i = 0, e = decl.num_params(); i < e; ++i) {
        const ParameterDecl* param = decl.get_param(i);
        const spbe::Type* type = lower_type(param->get_type());
        arg_types.push_back(type);

        args.push_back(new spbe::Argument(type, param->get_name(), i, nullptr));
    }

    const spbe::FunctionType* type = spbe::FunctionType::get(
        m_graph, arg_types, lower_type(decl.get_return_type()));

    new spbe::Function(m_graph, linkage, type, decl.get_name(), args);
}

void SPBECodegen::define_spbe_function(FunctionDecl& decl) {
    spbe::Function* FN = m_function = m_graph.get_function(decl.get_name());
    assert(FN && "function has not been declared yet!");

    // Stop now if the function has no body, i.e. needs no code generation.
    if (!decl.has_body())
        return;

    // Setup the entry basic block for the function.
    spbe::BasicBlock* entry_bb = new spbe::BasicBlock(FN);
    m_builder.set_insert(entry_bb);

    // For each function argument, designate a new stack local, and store the
    // argument to it.
    for (uint32_t i = 0, e = decl.num_params(); i < e; ++i) {
        spbe::Argument* arg = FN->get_arg(i);
        spbe::Local* local = new spbe::Local(
            m_graph, 
            arg->get_type(), 
            m_graph.get_target().get_type_align(arg->get_type()), 
            arg->get_name(), 
            FN);

        m_builder.build_store(arg, local);
    }

    decl.get_body()->accept(*this);

    // Check if the tail block of the function terminates.
    if (!m_builder.get_insert()->terminates()) {
        // If the block does not terminate, but the function does not return a
        // value, then just insert an empty return instruction.
        if (!FN->get_return_type()) {
            m_builder.build_ret_void();
        } else {
            const string& name = decl.get_name();
            const SourceSpan span = decl.get_span();
            m_diags.warn("function '" + name + "' does not always return", span);
        }
    }

    m_function = nullptr;
    m_builder.clear_insert();
}

void SPBECodegen::declare_spbe_structure(StructDecl& decl) {

}

void SPBECodegen::define_spbe_structure(StructDecl& decl) {
    spbe::StructType::create(m_graph, decl.get_name(), {});
}

void SPBECodegen::visit(TranslationUnitDecl& node) {
    m_diags.set_path(node.get_file());

    m_phase = Declare;
    for (uint32_t i = 0, e = node.num_decls(); i < e; ++i)
        node.get_decl(i)->accept(*this);

    m_phase = Define;
    for (uint32_t i = 0, e = node.num_decls(); i < e; ++i)
        node.get_decl(i)->accept(*this);
}

void SPBECodegen::visit(VariableDecl& node) {
    const spbe::Type* type = lower_type(node.get_type());

    if (node.is_global()) {
        if (m_phase == Declare) {
            declare_spbe_global(node);
        } else if (m_phase == Define) {
            define_spbe_global(node);
        }

        return;
    }

    spbe::Local* local = new spbe::Local(
        m_graph, 
        type, 
        m_graph.get_target().get_type_align(type), 
        node.get_name(), 
        m_function);

    if (node.has_init()) {
        m_vctx = RValue;
        node.get_init()->accept(*this);
        assert(m_temp && "variable initializer does not produce a value!");

        m_builder.build_store(m_temp, local);
    }
}

void SPBECodegen::visit(FunctionDecl& node) {
    if (m_phase == Declare) {
        declare_spbe_function(node);
    } else if (m_phase == Define) {
        define_spbe_function(node);
    }
}

void SPBECodegen::visit(StructDecl& node) {
    if (m_phase == Declare) {
        declare_spbe_structure(node);
    } else if (m_phase == Define) {
        define_spbe_structure(node);
    }
}

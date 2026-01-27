//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lace/codegen/LIRCodegen.hpp"
#include "lace/core/Diagnostics.hpp"
#include "lace/tree/Defn.hpp"
#include "lace/tree/Type.hpp"
#include "lir/graph/Function.hpp"
#include "lir/graph/Type.hpp"

using namespace lace;

void LIRCodegen::codegen_initial_definition(const Defn* defn) {
    switch (defn->get_kind()) {
        case Defn::Function:
            codegen_initial_function(static_cast<const FunctionDefn*>(defn));
            break;

        case Defn::Struct:
            codegen_initial_struct(static_cast<const StructDefn*>(defn));
            break;

        case Defn::Variable:
            codegen_initial_global(static_cast<const VariableDefn*>(defn));
            break;

        default:
            break;
    }
}

void LIRCodegen::codegen_lowered_definition(const Defn* defn) {
    switch (defn->get_kind()) {
        case Defn::Function:
            codegen_lowered_function(static_cast<const FunctionDefn*>(defn));
            break;

        case Defn::Struct:
            codegen_lowered_struct(static_cast<const StructDefn*>(defn));
            break;

        case Defn::Variable:
            codegen_lowered_global(static_cast<const VariableDefn*>(defn));
            break;

        default:
            break;
    }
}

lir::Function* LIRCodegen::codegen_initial_function(const FunctionDefn* defn) {
    lir::Function::LinkageType linkage = lir::Function::Internal;
    if (defn->has_rune(Rune::Public))
        linkage = lir::Function::External;

    std::vector<lir::Type*> types = {};
    std::vector<lir::FunctionArgument*> args = {};

    types.reserve(defn->num_params());
    args.reserve(defn->num_params());

    lir::Type* return_type = to_lir_type(defn->get_return_type());

    /*
    if (!m_mach.is_scalar(return_type)) {
        // Return type is non-scalar/aggregate, so instead we take in a pointer
        // as the first argument with the ARET trait, and write changes to it
        // during returns in this function.

        lir::Type* type = lir::PointerType::get(m_cfg, return_type);
        
        types.push_back(type);
        args.push_back(lir::FunctionArgument::create(
            type, 
            ".ret",
            nullptr, 
            lir::FunctionArgument::Trait::ARet
        ));

        // Change the return type to void now.
        return_type = lir::VoidType::get(m_cfg);
    }
    */

    for (uint32_t i = 0; i < defn->num_params(); ++i) {
        const ParameterDefn* param = defn->get_param(i);
        lir::Type* type = to_lir_type(param->get_type());

        std::string name = param->get_name();
        if (name == "_")
            name = "";

        auto trait = lir::FunctionArgument::Trait::None;
        /*
        if (!m_mach.is_scalar(type)) {
            trait = lir::FunctionArgument::Trait::Valued;
            type = lir::PointerType::get(m_cfg, type);
        }
        */

        types.push_back(type);
        args.push_back(lir::FunctionArgument::create(
            type, 
            name, 
            nullptr, 
            trait
        ));
    }

    return lir::Function::create(
        m_cfg, 
        linkage, 
        lir::FunctionType::get(m_cfg, types, return_type), 
        defn->get_name(), 
        args
    );
}

lir::Function* LIRCodegen::codegen_lowered_function(const FunctionDefn* defn) {
    lir::Function* func = m_cfg.get_function(defn->get_name());
    assert(func && "function does not exist!");

    if (!defn->has_body())
        return func;

    m_func = func;
    lir::BasicBlock* entry = lir::BasicBlock::create({}, m_func);
    m_builder.set_insert(entry);

    uint32_t i = func->has_aret() ? 1 : 0;
    for (const uint32_t e = func->num_args(); i < e; ++i) {
        lir::FunctionArgument* arg = func->get_arg(i);
        lir::Type* type = arg->get_type();
        
        /*
        if (arg->get_trait() == lir::FunctionArgument::Trait::Valued 
          || arg->get_trait() == lir::FunctionArgument::Trait::ARet) {
            continue;
        }
        */
        
        lir::Local* local = lir::Local::create(
            m_cfg, 
            type, 
            arg->get_name(), 
            m_mach.get_align(type), 
            func
        );

        m_builder.build_store(arg, local);
    }

    codegen_statement(defn->get_body());

    if (!m_builder.get_insert()->terminates()) {
        if (m_func->get_return_type()->is_void_type()) {
            m_builder.build_ret();
        } else {
            log::warn("function does not always return", 
                log::Span(m_cfg.get_filename(), defn->get_span()));
        }
    }

    m_func = nullptr;
    m_builder.clear_insert();
    return func;
}

lir::Global* LIRCodegen::codegen_initial_global(const VariableDefn* defn) {
    lir::Global::LinkageType linkage = lir::Global::Internal;
    if (defn->has_rune(Rune::Public))
        linkage = lir::Global::External;

    return lir::Global::create(
        m_cfg, 
        to_lir_type(defn->get_type()), 
        linkage, 
        // @Todo: for now, all lowered globals will be mutable. for the case
        // of arrays like [5]mut s64, where the elements are mutable, but the
        // array itself is not, we need some special semantics here.
        //
        // Cause if we had it as immutable, then the data would be put in 
        // read-only, and thus it wouldn't let us mutate the elements like we
        // should be able to.
        false, /* !node.get_type().is_mut(), */ 
        defn->get_name()
    );
}

lir::Global* LIRCodegen::codegen_lowered_global(const VariableDefn* defn) {
    lir::Global* global = m_cfg.get_global(defn->get_name());
    assert(global && "global does not exist!");

    if (!defn->has_init())
        return global;

    lir::Value* value = codegen_valued_expression(defn->get_init());
    assert(value);
    
    lir::Constant* init = dynamic_cast<lir::Constant*>(value);
    assert(init && "global is not initialized with a constant!");
    global->set_initializer(init);
    return global;
}

lir::StructType* LIRCodegen::codegen_initial_struct(const StructDefn* defn) {
    return lir::StructType::create(m_cfg, defn->get_name(), {});
}

lir::StructType* LIRCodegen::codegen_lowered_struct(const StructDefn* defn) {
    lir::StructType* type = lir::StructType::get(m_cfg, defn->get_name());
    assert(type && "type does not exist!");

    for (FieldDefn* field : defn->get_fields())
        type->append_field(to_lir_type(field->get_type()));

    return type;
}

lir::Local* LIRCodegen::codegen_local_variable(const VariableDefn* defn) {
    lir::Type* type = to_lir_type(defn->get_type());
    lir::Local* local = lir::Local::create(
        m_cfg, 
        type, 
        defn->get_name(), 
        m_mach.get_align(type),
        m_func
    );

    if (!defn->has_init())
        return local;

    if (m_mach.is_scalar(type)) {
        lir::Value* value = codegen_valued_expression(defn->get_init());
        assert(value);

        m_builder.build_store(value, local);
    } else {
        m_place = local;

        lir::Value* value = codegen_addressed_expression(defn->get_init());
        if (value) {
            lir::Function* copy = get_intrinsic(
                "__copy", 
                lir::VoidType::get(m_cfg), 
                {
                    lir::PointerType::get_void_pointer(m_cfg),
                    lir::PointerType::get_void_pointer(m_cfg),
                    lir::IntegerType::get_i64_type(m_cfg)
                }
            );

            m_builder.build_call(copy->get_type(), copy, {
                local,
                value,
                lir::Integer::get(m_cfg, lir::Type::get_i64_type(m_cfg), m_mach.get_size(type))
            });
        }

        m_place = nullptr;
    }

    return local;
}

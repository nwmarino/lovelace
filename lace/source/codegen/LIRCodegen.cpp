//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lace/codegen/LIRCodegen.hpp"
#include "lace/tree/AST.hpp"
#include "lace/tree/Defn.hpp"
#include "lace/tree/Type.hpp"

#include "lir/graph/Type.hpp"

using namespace lace;

LIRCodegen::LIRCodegen(const Options& options, const AST* ast, lir::CFG& cfg)
  : m_options(options), m_mach(cfg.get_machine()), m_ast(ast), m_cfg(cfg),
    m_builder(cfg) {}

void LIRCodegen::run() {
    for (Defn* defn : m_ast->get_loaded())
        codegen_initial_definition(defn);

    for (Defn* defn : m_ast->get_defns())
        codegen_initial_definition(defn);

    for (Defn* defn : m_ast->get_loaded()) {
        // Types need full IR definitions, even if imported.
        if (TypeDefn* type = dynamic_cast<TypeDefn*>(defn))
            codegen_lowered_definition(type);
    }

    for (Defn* defn : m_ast->get_defns()) {
        // Fully define all type definitions before others i.e. functions and
        // globals. Since proper sizes may be needed for types later on, its
        // necessary to fill out structure fields now.
        if (TypeDefn* type = dynamic_cast<TypeDefn*>(defn))
            codegen_lowered_definition(type);
    }

    for (Defn* defn : m_ast->get_defns()) {
        if (!dynamic_cast<TypeDefn*>(defn))
            codegen_lowered_definition(defn);
    }
}

lir::Type* LIRCodegen::to_lir_type(const QualType& type) {
    switch (type->get_class()) {
        case Type::Alias:
            return to_lir_type(static_cast<const AliasType*>
                (type.get_type())->get_underlying());
        
        case Type::Array: {
            auto array = static_cast<const ArrayType*>(type.get_type());
            return lir::ArrayType::get(m_cfg, to_lir_type(
                array->get_element_type()), array->get_size());
        }

        case Type::Builtin: {
            auto builtin = static_cast<const BuiltinType*>(type.get_type());

            switch (builtin->get_kind()) {
                case BuiltinType::Void:
                    return lir::VoidType::get(m_cfg);
                case BuiltinType::Bool:
                case BuiltinType::Char:
                case BuiltinType::Int8:
                case BuiltinType::UInt8:
                    return lir::Type::get_i8_type(m_cfg);
                case BuiltinType::Int16:
                case BuiltinType::UInt16:
                    return lir::Type::get_i16_type(m_cfg);
                case BuiltinType::Int32:
                case BuiltinType::UInt32:
                    return lir::Type::get_i32_type(m_cfg);
                case BuiltinType::Int64:
                case BuiltinType::UInt64:
                    return lir::Type::get_i64_type(m_cfg);
                case BuiltinType::Float32:
                    return lir::Type::get_f32_type(m_cfg);
                case BuiltinType::Float64:
                    return lir::Type::get_f64_type(m_cfg);
            }

            __builtin_unreachable();
        }

        case Type::Deferred:
            assert(false && "cannot lower deferred type!");

        case Type::Enum:
            return to_lir_type(static_cast<const EnumType*>(
                type.get_type())->get_underlying());

        case Type::Function: {
            auto sig = static_cast<const FunctionType*>(type.get_type());
            std::vector<lir::Type*> args(sig->num_params(), nullptr);
            for (uint32_t i = 0; i < sig->num_params(); ++i)
                args[i] = to_lir_type(sig->get_param(i));

            return lir::FunctionType::get(
                m_cfg, args, { to_lir_type(sig->get_return_type()) });
        }

        case Type::Pointer:
            return lir::PointerType::get(m_cfg, to_lir_type(
                static_cast<const PointerType*>(type.get_type())->get_pointee()));

        case Type::Struct:
            return lir::StructType::get(m_cfg, 
                static_cast<const StructType*>(type.get_type())->to_string());
    }
}

lir::Function *LIRCodegen::get_intrinsic(
        const std::string &name, const lir::FunctionType::Params &params,
        const lir::FunctionType::Results &results) { 
    lir::Function* func = m_cfg.get_function(name);
    if (func)
        return func;
        
    std::vector<lir::Parameter*> parameters(params.size(), nullptr);

    for (uint32_t i = 0; i < params.size(); ++i)
        parameters[i] = lir::Parameter::create(params[i]);

    return lir::Function::create(
        m_cfg, 
        lir::Function::LinkageType::Public, 
        lir::FunctionType::get(m_cfg, params, results),
        name, 
        parameters
    );
}

lir::Value* LIRCodegen::inject_comparison(lir::Value* value) {
    lir::Type* type = value->get_type();
    
    if (type->is_integer_type(1)) {
        return value;
    } else if (type->is_integer_type()) {
        return m_builder.build_cmp_ine(value, lir::Integer::get_zero(m_cfg, type));
    } else if (type->is_float_type()) {
        return m_builder.build_cmp_one(value,  lir::Float::get_zero(m_cfg, type));
    } else if (type->is_pointer_type()) {
        return m_builder.build_cmp_ine(value, lir::Null::get(m_cfg, type));
    }

    assert(false && "value cannot be reduced to a boolean!");
}

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

Codegen::Codegen(const Options& options, lir::CFG& cfg)
  : m_options(options), m_cfg(cfg), m_mach(cfg.get_machine()), m_builder(cfg) {}

lir::Function* Codegen::get_intrinsic(const std::string& name, lir::Type* ret, 
                                      const std::vector<lir::Type*>& params) {
    lir::Function* function = m_cfg.get_function(name);
    if (function) {
        return function;
    }

    lir::Function::Args args(params.size(), nullptr);
    for (uint32_t i = 0; i < params.size(); ++i)
        args[i] = lir::Function::Arg::create(params[i], "");

    return lir::Function::create(
        m_cfg, 
        lir::Function::External, 
        lir::FunctionType::get(m_cfg, params, ret), name, 
        args);
}

lir::Type* Codegen::lower_type(const QualType& type) {
    switch (type->get_class()) {
        case Type::Alias: {
            const AliasType* alias_type = static_cast<const AliasType*>(type.get_type());
            return lower_type(alias_type->get_underlying());
        }
        
        case Type::Array: {
            const ArrayType* array_type = static_cast<const ArrayType*>(type.get_type());
            return lir::ArrayType::get(m_cfg, lower_type(
                array_type->get_element_type()), array_type->get_size());
        }

        case Type::Builtin: {
            const BuiltinType* bltn_type = static_cast<const BuiltinType*>(type.get_type());
            switch (bltn_type->get_kind()) {
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
        }

        case Type::Deferred: {
            assert(false && "cannot lower deferred type!");
        }

        case Type::Enum: {
            const EnumType* enum_type = static_cast<const EnumType*>(type.get_type());
            return lower_type(enum_type->get_underlying());
        }

        case Type::Function: {
            const FunctionType* fn_type = static_cast<const FunctionType*>(type.get_type());
            std::vector<lir::Type*> args(fn_type->num_params(), nullptr);
            for (uint32_t i = 0; i < fn_type->num_params(); ++i)
                args[i] = lower_type(fn_type->get_param(i));

            return lir::FunctionType::get(
                m_cfg, args, lower_type(fn_type->get_return_type()));
        }

        case Type::Pointer: {
            const PointerType* ptr_type = static_cast<const PointerType*>(type.get_type());
            return lir::PointerType::get(m_cfg, lower_type(ptr_type->get_pointee()));
        }

        case Type::Struct: {
            const StructType* struct_type = static_cast<const StructType*>(type.get_type());
            return lir::StructType::get(m_cfg, struct_type->to_string());
        }
    }
}

lir::Value* Codegen::inject_bool_comparison(lir::Value* value) {
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

void Codegen::visit(AST& ast) {
    m_phase = Declare;
    for (Defn* loaded : ast.get_loaded())
        loaded->accept(*this);

    for (Defn* defn : ast.get_defns())
        defn->accept(*this);

    m_phase = Define;
    for (Defn* loaded : ast.get_loaded()) {
        // Structs need full IR definitions, even if loaded.
        if (dynamic_cast<StructDefn*>(loaded))
            loaded->accept(*this);
    }

    for (Defn* defn : ast.get_defns())
        defn->accept(*this);
}

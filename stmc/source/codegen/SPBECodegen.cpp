//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/codegen/SPBECodegen.hpp"
#include "stmc/tree/Type.hpp"

#include "spbe/graph/Constant.hpp"
#include "spbe/graph/Type.hpp"

using namespace stm;

SPBECodegen::SPBECodegen(Diagnostics& diags, Options& options, spbe::CFG& graph) 
        : m_diags(diags), m_options(options), m_graph(graph), m_builder(graph) {
    m_diags.set_path(graph.get_file());
}

spbe::Function* SPBECodegen::get_intrinsic(
        const string& name, const spbe::Type* ret,
        const vector<const spbe::Type*>& params) {
    return nullptr;
}

const spbe::Type* SPBECodegen::lower_type(const Type* type) {
    if (auto BT = dynamic_cast<const BuiltinType*>(type)) {
        switch (BT->get_kind()) {
        case BuiltinType::Void:
            return nullptr;
        
        case BuiltinType::Bool:
        case BuiltinType::Char:
        case BuiltinType::Int8:
        case BuiltinType::UInt8:
            return spbe::Type::get_i8_type(m_graph);

        case BuiltinType::Int16:
        case BuiltinType::UInt16:
            return spbe::Type::get_i16_type(m_graph);

        case BuiltinType::Int32:
        case BuiltinType::UInt32:
            return spbe::Type::get_i32_type(m_graph);

        case BuiltinType::Int64:
        case BuiltinType::UInt64:
            return spbe::Type::get_i64_type(m_graph);
        
        case BuiltinType::Float32:
            return spbe::Type::get_f32_type(m_graph);

        case BuiltinType::Float64:
            return spbe::Type::get_f64_type(m_graph);

        case BuiltinType::Float128:
            assert(false && "f128 type not supported!");
        }
    } else if (auto AT = dynamic_cast<const ArrayType*>(type)) {
        return spbe::ArrayType::get(
            m_graph, 
            lower_type(AT->get_element_type()), 
            AT->get_size());
    } else if (auto PT = dynamic_cast<const PointerType*>(type)) {
        return spbe::PointerType::get(
            m_graph, 
            lower_type(PT->get_pointee()));
    }

    assert(false && "type does not have an spbe equivelant!");
}

spbe::Value* SPBECodegen::inject_bool_comparison(spbe::Value* value) {
    const spbe::Type* type = value->get_type();
    
    if (type->is_integer_type(1)) {
        return value;
    } else if (type->is_integer_type()) {
        spbe::Value* zero = spbe::ConstantInt::get_zero(m_graph, type);
        return m_builder.build_cmp_ine(value, zero);
    } else if (type->is_floating_point_type()) {
        spbe::Value* zero = spbe::ConstantFP::get_zero(m_graph, type);
        return m_builder.build_cmp_one(value, zero);
    } else if (type->is_pointer_type()) {
        spbe::Value* null = spbe::ConstantNull::get(m_graph, type);
        return m_builder.build_cmp_ine(value, null);
    }

    assert(false && "value cannot be reduced to a boolean!");
}

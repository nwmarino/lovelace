//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "ast/Type.hpp"
#include "ast/TypeContext.hpp"

using namespace scc;

TypeContext::TypeContext() {
    for (uint32_t i = BuiltinType::Void; i < BuiltinType::LongDouble; ++i) {
        m_bts[static_cast<BuiltinType::Kind>(i)] = new BuiltinType(
            static_cast<BuiltinType::Kind>(i)); 
    }
}

TypeContext::~TypeContext() {
    for (const auto& [kind, type] : m_bts)
        delete type;

    for (ArrayType* type : m_arrays)
        delete type;

    for (PointerType* type : m_ptrs)
        delete type;

    for (FunctionType* type : m_sigs)
        delete type;

    for (TypedefType* type : m_typedefs)
        delete type;

    for (RecordType* type : m_records)
        delete type;

    for (EnumType* type : m_enums)
        delete type;

    m_bts.clear();
    m_arrays.clear();
    m_ptrs.clear();
    m_sigs.clear();
    m_typedefs.clear();
    m_records.clear();
    m_enums.clear();
}

//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/tree/Context.hpp"
#include "stmc/tree/Type.hpp"

using namespace stm;

Context::Context() {
    // Initialize all built-in types.
    for (uint32_t i = static_cast<uint32_t>(BuiltinType::Void); 
         i < static_cast<uint32_t>(BuiltinType::Float64); 
         ++i) {
        BuiltinType::Kind kind = static_cast<BuiltinType::Kind>(i);
        m_builtins[kind] = new BuiltinType(kind);
    }
}

Context::~Context() {
    for (auto& [kind, type] : m_builtins)
        delete type;

    for (auto& [element, pair] : m_arrays) {
        for (auto& [size, type] : pair)
            delete type;
    }

    for (auto& [pointee, type] : m_pointers)
        delete type;

    for (auto& [name, type] : m_aliases)
        delete type;

    for (auto& [name, type] : m_structs)
        delete type;

    for (auto& [name, type] : m_enums)
        delete type;

    for (auto& [name, type] : m_deferred)
        delete type;

    for (auto& type : m_functions)
        delete type;

    m_builtins.clear();
    m_arrays.clear();
    m_pointers.clear();
    m_aliases.clear();
    m_structs.clear();
    m_enums.clear();
    m_deferred.clear();
    m_functions.clear();
}

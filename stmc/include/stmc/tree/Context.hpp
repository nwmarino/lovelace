//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef STATIM_CONTEXT_H_
#define STATIM_CONTEXT_H_

#include "stmc/tree/Type.hpp"
#include "stmc/tree/TypeUse.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace std {

template<>
struct hash<stm::TypeUse> {
    size_t operator()(const stm::TypeUse& type) const {
        return hash<const stm::Type*>()(type.get_type()) ^ 
            (std::hash<uint32_t>()(type.get_qualifiers()) << 1);
    }
};

} // namespace std

namespace stm {

using std::string;
using std::unordered_map;
using std::vector;

class Context final {
    friend class TranslationUnitDecl;
    friend class BuiltinType;
    friend class ArrayType;
    friend class PointerType;
    friend class FunctionType;
    friend class AliasType;
    friend class StructType;
    friend class EnumType;
    friend class NamedTypeRef;

    using BuiltinTypePool = unordered_map<BuiltinType::Kind, BuiltinType*>;
    using ArrayTypePool = unordered_map<TypeUse, unordered_map<uint32_t, ArrayType*>>;
    using PointerTypePool = unordered_map<TypeUse, PointerType*>;
    using AliasTypePool = unordered_map<string, AliasType*>;
    using StructTypePool = unordered_map<string, StructType*>;
    using EnumTypePool = unordered_map<string, EnumType*>;
    using DeferredTypePool = unordered_map<string, NamedTypeRef*>;
    using FunctionTypePool = vector<FunctionType*>;

    BuiltinTypePool m_builtins = {};
    ArrayTypePool m_arrays = {};
    PointerTypePool m_pointers = {};
    AliasTypePool m_aliases = {};
    StructTypePool m_structs = {};
    EnumTypePool m_enums = {};
    DeferredTypePool m_deferred = {};
    FunctionTypePool m_functions = {};

public:
    Context();

    ~Context();

    Context(const Context&) = delete;
    void operator=(const Context&) = delete;

    Context(Context&& other) {
        if (&other != this) {
            m_builtins = std::move(other.m_builtins);
            m_arrays = std::move(other.m_arrays);
            m_pointers = std::move(other.m_pointers);
            m_aliases = std::move(other.m_aliases);
            m_structs = std::move(other.m_structs);
            m_enums = std::move(other.m_enums);
            m_deferred = std::move(other.m_deferred);
            m_functions = std::move(other.m_functions);
        }
    }

    void operator=(Context&& other) {
        if (&other != this) {
            m_builtins = std::move(other.m_builtins);
            m_arrays = std::move(other.m_arrays);
            m_pointers = std::move(other.m_pointers);
            m_aliases = std::move(other.m_aliases);
            m_structs = std::move(other.m_structs);
            m_enums = std::move(other.m_enums);
            m_deferred = std::move(other.m_deferred);
            m_functions = std::move(other.m_functions);
        }
    }
};

} // namespace stm

#endif // STATIM_CONTEXT_H_

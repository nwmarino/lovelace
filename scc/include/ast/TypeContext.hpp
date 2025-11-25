//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SCC_TYPE_CONTEXT_H_
#define SCC_TYPE_CONTEXT_H_

//
// This header file declares the TypeContext class, an important structure for
// managing the types of a translation unit and its abstract syntax tree.
//

#include "ast/Type.hpp"

#include <unordered_map>
#include <vector>

namespace scc {

using std::unordered_map;
using std::vector;

class TypeContext final {
    friend class TranslationUnit;
    friend class BuiltinType;
    friend class ArrayType;
    friend class PointerType;
    friend class FunctionType;
    friend class TypedefType;
    friend class RecordType;
    friend class EnumType;

    using BuiltinTypePool = unordered_map<BuiltinType::Kind, BuiltinType*>;
    using ArrayTypePool = vector<ArrayType*>;
    using PointerTypePool = vector<PointerType*>;
    using FunctionTypePool = vector<FunctionType*>;
    using TypedefTypePool = vector<TypedefType*>;
    using RecordTypePool = vector<RecordType*>;
    using EnumTypePool = vector<EnumType*>;

    /// Pool of types that are built-in to the C language.
    BuiltinTypePool m_bts = {};

    /// Pool of arrays types.
    ArrayTypePool m_arrays = {};

    /// Pool of pointer types.
    PointerTypePool m_ptrs = {};

    /// List of function type signatures declared implicitly by a translation 
    /// unit. These are listed rather than mapped since code cannot directly
    /// reference these signatures.
    FunctionTypePool m_sigs = {};

    /// Pool of typedef types.
    TypedefTypePool m_typedefs = {};

    /// Pool of record types.
    RecordTypePool m_records = {};

    /// Pool of enum types.
    EnumTypePool m_enums = {};

public:
    TypeContext();

    TypeContext(const TypeContext&) = delete;
    TypeContext& operator = (const TypeContext&) = delete;

    ~TypeContext();
};

} // namespace scc

#endif // SCC_TYPE_CONTEXT_H_

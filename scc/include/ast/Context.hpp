//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SCC_CONTEXT_H_
#define SCC_CONTEXT_H_

//
// This header file declares the Context class, an important structure for
// managing types and top-level declarations of a translation unit and its
// abstract syntax tree.
//

#include "ast/Type.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

namespace scc {

class Context final {
    friend class TranslationUnit;
    friend class BuiltinType;
    friend class ArrayType;
    friend class PointerType;
    friend class FunctionType;
    friend class TypedefType;
    friend class EnumType;

    using BuiltinTypePool = 
        std::unordered_map<BuiltinType::Kind, std::unique_ptr<BuiltinType>>;
    using ArrayTypePool = std::vector<std::unique_ptr<ArrayType>>;
    using PointerTypePool = std::vector<std::unique_ptr<PointerType>>;
    using FunctionTypePool = std::vector<std::unique_ptr<FunctionType>>;
    using TypedefTypePool = std::vector<std::unique_ptr<TypedefType>>;
    using EnumTypePool = std::vector<std::unique_ptr<EnumType>>;

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

    /// Pool of enum types.
    EnumTypePool m_enums = {};

    Context();

public:
    Context(const Context&) = delete;
    Context& operator = (const Context&) = delete;

    ~Context() = default;
};

} // namespace scc

#endif // SCC_CONTEXT_H_

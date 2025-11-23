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
#include "ast/QualType.hpp"

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

namespace scc {

class Context final {
    friend class TranslationUnit;
    friend class VoidType;
    friend class IntegerType;
    friend class FPType;
    friend class ArrayType;
    friend class PointerType;
    friend class FunctionType;

    /// Potential kinds of primitive types.
    enum class TypeKind : uint32_t {
        Void = 0,
        UChar = 1, 
        Char = 2,
        UShort = 3, 
        Short = 4,
        UInt = 5, 
        Int = 6,
        ULong = 7, 
        Long = 8,
        ULongLong = 9, 
        LongLong = 10,
        Float = 11,
        Double = 12,
    };

    using PrimitiveTypePool = 
        std::unordered_map<TypeKind, std::unique_ptr<Type>>;
    using ArrayTypePool = std::vector<std::unique_ptr<ArrayType>>;
    using PointerTypePool = std::vector<std::unique_ptr<PointerType>>;
    using FunctionTypeList = std::vector<std::unique_ptr<FunctionType>>;

    /// Pool of primitive types which are built-in to the C language.
    PrimitiveTypePool m_prims = {};

    /// Pool of arrays types.
    ArrayTypePool m_arrays = {};

    /// Pool of pointer types.
    PointerTypePool m_ptrs = {};

    /// List of function type signatures declared implicitly by a translation 
    /// unit. These are listed rather than mapped since code cannot directly
    /// reference these signatures.
    FunctionTypeList m_sigs = {};

    Context();

public:
    Context(const Context&) = delete;
    Context& operator = (const Context&) = delete;

    ~Context() = default;
};

} // namespace scc

#endif // SCC_CONTEXT_H_

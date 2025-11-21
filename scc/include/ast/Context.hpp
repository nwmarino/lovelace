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

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

namespace scc {

class Type;
class PointerType;
class FunctionType;

class Context final {
    friend class TranslationUnit;
    friend class VoidType;
    friend class IntegerType;
    friend class FPType;
    friend class PointerType;
    friend class FunctionType;

    /// Potential kinds of primitive types.
    enum class TypeKind : uint16_t {
        Void,
        UChar, Char,
        UShort, Short,
        UInt, Int,
        ULong, Long,
        ULongLong, LongLong,
        Float,
        Double,
    };

    using PrimitiveTypePool = 
        std::unordered_map<TypeKind, std::unique_ptr<Type>>;
    using PointerTypePool = 
        std::unordered_map<const Type*, std::unique_ptr<PointerType>>;
    using FunctionTypeList = std::vector<std::unique_ptr<FunctionType>>;

    /// Pool of primitive types which are built-in to the C language.
    PrimitiveTypePool m_prims = {};

    /// Pool of pointer types keyed by their pointee.
    PointerTypePool m_ptrs = {};

    /// List of function type signatures declared implicitly by a translation 
    /// unit. These are listed rather than mapped since code cannot directly
    /// reference these signatures.
    ///
    /// TODO: See about reorganizing for sake of function pointers.
    FunctionTypeList m_sigs = {};

    Context();

public:
    Context(const Context&) = delete;
    Context& operator = (const Context&) = delete;

    ~Context() = default;
};

} // namespace scc

#endif // SCC_CONTEXT_H_

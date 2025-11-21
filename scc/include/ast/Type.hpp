//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SCC_TYPE_H_
#define SCC_TYPE_H_

//
// This header file declares classes to represent the visible types in C.
//
// Type instances are managed by the Context class, which is also the only
// class able to create type instances.
//
// Expression nodes and the like can hold onto const pointer borrows which are
// received from the Context. Should the types have qualifiers, the nodes can
// hold onto valued instances of QualType.
//

#include <cstdint>
#include <vector>

namespace scc {

class Context;

/// Base class for all types in the C type system.
class Type {
    friend class Context;

public:
    using id_t = uint32_t;

    enum class Kind : uint32_t {
        Void,
        Integer,
        Float,
        Pointer,
        Function,
    };

protected:
    /// Iterator for type ids during creation.
    static id_t s_id;

    /// A unique identifier for this type. Each instance of a type has an id,
    /// but deduplication means only symbollicaly unique types get an id. 
    ///
    /// The id of a type cannot be changed after creation.
    id_t m_id;

    /// The kind of type this is. This cannot change after creation.
    Kind m_kind;

    Type(Kind kind) : m_id(s_id++), m_kind(kind) {}

public:
    Type(const Type&) = delete;
    Type& operator = (const Type&) = delete;

    ~Type() = default;

    bool operator == (const Type& other) const { return m_id == other.id(); }
    bool operator != (const Type& other) const { return m_id != other.id(); }

    /// Returns the unique numerical identifier of this type.
    id_t id() const { return m_id; }

    /// Returns the kind of type this is.
    Kind kind() const { return m_kind; }

    /// Returns true if this is the void type.
    bool is_void() const { return kind() == Kind::Void; }

    /// Returns true if this is an integer type.
    bool is_integer() const { return kind() == Kind::Integer; }
    
    /// Returns true if this is an integer type with the number of bits \p bits.
    virtual bool is_integer(uint32_t bits) const { return false; }

    /// Returns true if this is a floating point type.
    bool is_float() const { return kind() == Kind::Float; }

    /// Returns true if this is a floating point type with the number of bits
    /// \p bits.
    virtual bool is_float(uint32_t bits) const { return false; }

    /// Returns true if this is a pointer type.
    bool is_pointer() const { return kind() == Kind::Pointer; }
};

/// Represents the fundamental void type `void`.
class VoidType final : public Type {
    friend class Context;

    VoidType() : Type(Kind::Void) {}

public:
    static const VoidType* get(Context& ctx);
};

/// Represents the fundamental integral types, i.e. 'char', 'short', 'int', and
/// 'long', as well their unsigned variants.
class IntegerType final : public Type {
    friend class Context;

    /// The width of this integer type in bits.
    uint32_t m_bits;

    /// If this integer type is marked with 'unsigned' or not.
    bool m_signed;

    IntegerType(uint32_t bits, bool is_signed) 
        : Type(Kind::Integer), m_bits(bits), m_signed(is_signed) {}

public:
    /// Returns the primitive integer type with the bit width defined by 
    /// \p bits, and signedness \p is_signed.
    static const IntegerType* get(Context& ctx, uint32_t bits, bool is_signed);

    /// Returns the number of bits in this integer type.
    uint32_t bits() const { return m_bits; }

    /// Returns true if this integer type is signed, and false if unsigned.
    bool is_signed() const { return m_signed; }

    bool is_integer(uint32_t bits) const override { return m_bits == bits; }
};

/// Represents the fundamental floating point types, i.e. `float` and `double`.
class FPType final : public Type {
    friend class Context;

    /// The width of this floating point type in bits.
    uint32_t m_bits;

    FPType(uint32_t bits) : Type(Kind::Float), m_bits(bits) {}

public:
    /// Returns the primitive floating point type with the bit width defined
    /// by \p bits.
    static const FPType* get(Context& ctx, uint32_t bits);

    /// Returns the number of bits in this floating point type.
    uint32_t bits() const { return m_bits; }

    bool is_float(uint32_t bits) const override { return m_bits == bits; }
};

/// Represents a C pointer type.
class PointerType final : public Type {
    friend class Context;

    /// The pointee type of this pointer. For example, for 'int*', the pointee
    /// is an integral type 'int'.
    const Type* m_pointee;

    PointerType(const Type* pointee) 
        : Type(Kind::Pointer), m_pointee(pointee) {}

public:
    /// Returns the pointer type that points to type \p pointee.
    static const PointerType* get(Context& ctx, const Type* pointee);

    /// Returns the type that this pointer type is pointing to.
    const Type* pointee() const { return m_pointee; }
};

/// Represents the signature type implicitly defined by a function declaration.
class FunctionType final : public Type {
    friend class Context;

    /// The type that the underlying function returns.
    const Type* m_ret;

    /// The list of parameter types of the underlying function.
    std::vector<const Type*> m_params;

    FunctionType(const Type* ret, const std::vector<const Type*>& params)
        : Type(Kind::Function), m_ret(ret), m_params(params) {}

public:
    /// Returns a function signature type that returns type \p ret, and has
    /// parameter types \p params.
    static const FunctionType* get(Context& ctx, const Type* ret, 
                                   const std::vector<const Type*>& params);

    /// Returns the type that this function returns.
    const Type* ret() const { return m_ret; }

    /// Returns true if this function returns void, or nothing.
    bool returns_void() const { return m_ret->is_void(); }

    /// Returns the list of parameter types for this function.
    const std::vector<const Type*>& params() const { return m_params; }
    std::vector<const Type*>& params() { return m_params; }

    /// Returns true if this function has any parameter types.
    bool has_params() const { return !m_params.empty(); }
};

} // namespace scc

#endif // SCC_TYPE_H_

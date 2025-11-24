//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SCC_TYPE_H_
#define SCC_TYPE_H_

//
// This header file declares classes to represent the visible types in C.
//
// Type instances are managed by the TypeContext class, which is also the only
// class able to create type instances.
//
// Expression nodes and the like can hold onto const pointer borrows which are
// received from the TypeContext. Should the types have qualifiers, the nodes can
// hold onto valued instances of QualType.
//

#include "ast/QualType.hpp"

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace scc {

using std::string;
using std::vector;

class TypeContext;

class EnumDecl;
class RecordDecl;
class TypedefDecl;

/// Base class for all types in the C type system.
class Type {
    friend class TypeContext;

public:
    using id_t = uint32_t;

protected:
    /// Iterator for type ids during creation.
    static id_t s_id;

    /// A unique identifier for this type. Each instance of a type has an id,
    /// but deduplication means only symbollicaly unique types get an id. 
    ///
    /// The id of a type cannot be changed after creation.
    id_t m_id;

    Type() : m_id(s_id++) {}

public:
    Type(const Type&) = delete;
    Type& operator = (const Type&) = delete;

    virtual ~Type() = default;

    bool operator == (const Type& other) const { return m_id == other.id(); }
    bool operator != (const Type& other) const { return m_id != other.id(); }

    /// Returns the unique numerical identifier of this type.
    id_t id() const { return m_id; }

    /// Returns true if this is the void type.
    virtual bool is_void() const { return false; }

    /// Returns true if this is an integer type.
    virtual bool is_integer() const { return false; }
    
    /// Returns true if this is a signed integer type.
    virtual bool is_signed_integer() const { return false; }

    /// Returns true if this is an unsigned integer type.
    virtual bool is_unsigned_integer() const { return false; }

    /// Returns true if this is a floating point type.
    virtual bool is_floating_point() const { return false; }

    /// Returns true if this is a pointer type.
    virtual bool is_pointer() const { return false; }

    /// Returns a stringified version of this type.
    virtual string to_string() const = 0;
};

/// Represents fundamental types built-in to C.
class BuiltinType final : public Type {
    friend class TypeContext;
    
public:
    /// Possible kinds of built-in types.
    enum Kind : uint32_t {
        Void = 0,
        Char,
        UChar,
        Short,
        UShort,
        Int,
        UInt,
        Long,
        ULong,
        LongLong,
        ULongLong,
        Float,
        Double,
        LongDouble,
    };

private:
    /// The kind of built-in type this is.
    Kind m_kind;

    BuiltinType(Kind kind) : Type(), m_kind(kind) {}

public:
    /// Returns the 'void' type.
    static const BuiltinType* get_void_type(TypeContext& ctx);

    /// Returns the 'unsigned char' type.
    static const BuiltinType* get_uchar_type(TypeContext& ctx);

    /// Returns the 'char' or 'signed char' type.
    static const BuiltinType* get_char_type(TypeContext& ctx);

    /// Returns the 'unsigned short' type.
    static const BuiltinType* get_ushort_type(TypeContext& ctx);

    /// Returns the 'short' or 'signed short' type.
    static const BuiltinType* get_short_type(TypeContext& ctx);

    /// Returns the 'unsigned int' type.
    static const BuiltinType* get_uint_type(TypeContext& ctx);

    /// Returns the 'signed int' or 'int' type.
    static const BuiltinType* get_int_type(TypeContext& ctx);

    /// Returns the 'unsigned long' type.
    static const BuiltinType* get_ulong_type(TypeContext& ctx);

    /// Returns the 'signed long' or 'long' type.
    static const BuiltinType* get_long_type(TypeContext& ctx);

    /// Returns the 'unsigned long long' type.
    static const BuiltinType* get_ulonglong_type(TypeContext& ctx);

    /// Returns the 'signed long long' or 'long long' type.
    static const BuiltinType* get_longlong_type(TypeContext& ctx);

    /// Returns the 'float' type.
    static const BuiltinType* get_float_type(TypeContext& ctx);

    /// Returns the 'double' type.
    static const BuiltinType* get_double_type(TypeContext& ctx);

    /// Returns the 'long double' type.
    static const BuiltinType* get_longdouble_type(TypeContext& ctx);

    bool is_void() const override { return m_kind == Void; }

    bool is_integer() const override { 
        return Char <= m_kind && ULongLong <= m_kind; 
    }

    bool is_signed_integer() const override;
    
    bool is_unsigned_integer() const override;

    bool is_floating_point() const override { return m_kind >= Float; }

    string to_string() const override;
};

/// Represents an array type in C.
class ArrayType final : public Type {
    friend class TypeContext;

    /// The type of the elements in arrays with this type.
    QualType m_element;

    /// The static size of arrays with this type.
    uint32_t m_size;

    ArrayType(const QualType& element, uint32_t size)
        : Type(), m_element(element), m_size(size) {}
    
public:
    /// Returns the array type with element type \p element and size \p size.
    static const ArrayType* get(TypeContext& ctx, const QualType& element, 
                                uint32_t size);
                        
    /// Returns the type of element in arrays with this type.
    const QualType& get_element() const { return m_element; }
    QualType& get_element() { return m_element; }

    /// Returns the size of arrays with this type.
    uint32_t get_size();

    string to_string() const override {
        return m_element.to_string() + '[' + std::to_string(m_size) + ']';
    }
};

/// Represents a pointer type in C.
class PointerType final : public Type {
    friend class TypeContext;

    /// The pointee type of this pointer. For example, for 'int*', the pointee
    /// is an integral type 'int'.
    QualType m_pointee;

    PointerType(const QualType& pointee) : Type(), m_pointee(pointee) {}

public:
    /// Returns the pointer type that points to type \p pointee.
    static const PointerType* get(TypeContext& ctx, const QualType& pointee);

    /// Returns the type that this pointer type is pointing to.
    const QualType& get_pointee() const { return m_pointee; }
    QualType& get_pointee() { return m_pointee; }

    string to_string() const override { 
        return m_pointee.to_string() + '*'; 
    }
};

/// Represents the signature type implicitly defined by a function declaration.
class FunctionType final : public Type {
    friend class TypeContext;

    /// The type that the underlying function returns.
    QualType m_ret;

    /// The list of parameter types of the underlying function.
    vector<QualType> m_params;

    FunctionType(const QualType& ret, const vector<QualType>& params)
        : Type(), m_ret(ret), m_params(params) {}

public:
    /// Returns a function signature type that returns type \p ret, and has
    /// parameter types \p params.
    static const FunctionType* get(TypeContext& ctx, const QualType& ret, 
                                   const vector<QualType>& params);

    /// Returns the type that this function returns.
    const QualType& get_return_type() const { return m_ret; }
    QualType& get_return_type() { return m_ret; }

    /// Returns true if this function returns void, or nothing.
    bool returns_void() const { return m_ret->is_void(); }

    uint32_t num_params() const { return m_params.size(); }

    /// Returns true if this function has any parameter types.
    bool has_params() const { return !m_params.empty(); }

    /// Returns the list of parameter types for this function.
    const vector<QualType>& get_params() const { return m_params; }
    vector<QualType>& get_params() { return m_params; }

    /// Returns the parameter type at position \p i.
    const QualType& get_param_type(uint32_t i) const {
        assert(i < num_params() && "index out of bounds!");
        return m_params[i];
    }

    QualType& get_param_type(uint32_t i) {
        assert(i < num_params() && "index out of bounds!");
        return m_params[i];
    }
    
    string to_string() const override;
};

/// Represents the type defined by a 'typedef' declaration.
class TypedefType final : public Type {
    friend class TypeContext;

    /// The 'typedef' declaration that defines this type.
    const TypedefDecl* m_decl;

    /// The underlying type.
    QualType m_underlying;

    TypedefType(const TypedefDecl* decl, const QualType& underlying)
        : Type(), m_decl(decl), m_underlying(underlying) {}

public:
    /// Create and return a new type defined by a 'typedef' declaration \p decl
    /// with the underlying type \p underlying.
    static const TypedefType* create(TypeContext& ctx, const TypedefDecl* decl, 
                                     const QualType& underlying);
                               
    /// Returns the 'typedef' declaration that defines this type.
    const TypedefDecl* get_decl() const { return m_decl; }
    
    /// Returns the underlying type.
    const QualType& get_underlying() const { return m_underlying; }
    QualType& get_underlying() { return m_underlying; }

    string to_string() const override;
};

/// Represents the type defined by a 'struct' declaration.
class StructType final : public Type {
    friend class TypeContext;

    /// The 'struct' declaration that defines this type.
    const RecordDecl* m_decl;

    /// The types of the fields in the structure that defines this type.
    vector<QualType> m_fields;

    StructType(const RecordDecl* decl, const vector<QualType>& fields)
        : Type(), m_decl(decl), m_fields(fields) {}

public:
    /// Create and return a new type defined by a 'struct' declaration \p decl
    /// and field types \p fields.
    static const StructType* create(TypeContext& ctx, const RecordDecl* decl, 
                                    const vector<QualType>& fields);

    /// Returns the 'struct' declaration that defines this type.
    const RecordDecl* get_decl() const { return m_decl; }

    /// Returns the number of fields in this type.
    uint32_t num_fields() const { return m_fields.size(); }

    /// Returns the type of the field at position \p i.
    const QualType& get_field(uint32_t i) const {
        assert(i < num_fields() && "index out of bounds!");
        return m_fields[i];
    }

    QualType& get_field(uint32_t i) {
        assert(i < num_fields() && "index out of bounds!");
        return m_fields[i];
    }

    string to_string() const override;
};

/// Represents the type defined by an 'enum' declaration.
class EnumType final : public Type {
    friend class TypeContext;

    /// The 'enum' declaration that defines this type.
    const EnumDecl* m_decl;

    EnumType(const EnumDecl* decl) : Type(), m_decl(decl) {}

public:
    /// Create and return a new type defined by an 'enum' declaration \p decl.
    static const EnumType* create(TypeContext& ctx, const EnumDecl* decl);

    /// Returns the 'enum' declaration that defines this type.
    const EnumDecl* get_decl() const { return m_decl; }

    string to_string() const override;
};

} // namespace scc

#endif // SCC_TYPE_H_

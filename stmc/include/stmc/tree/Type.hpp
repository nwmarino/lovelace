//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef STATIM_TYPE_H_
#define STATIM_TYPE_H_

#include "stmc/tree/TypeUse.hpp"

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace stm {

using std::string;
using std::vector;

class AliasDecl;
class Context;
class EnumDecl;
class StructDecl;

/// Base class for all type nodes used in the abstract syntax tree (AST).
class Type {
public:
    virtual ~Type() = default;

    /// Returns the string equivelant of this type.
    virtual string to_string() const = 0;

    /// Compare this type with \p other.
    virtual bool compare(const Type* other) const { return false; }

    /// Returns true if this type can be casted to \p other. The \p implicitly
    /// flag determines if the cast follows implicit or explicit casting rules.
    virtual bool can_cast(const Type* other, bool implicitly = false) const {
        return false;
    }

    /// Test if this is the 'void' type.
    virtual bool is_void() const { return false; }

    /// Test if this is an integer type of any signedness.
    virtual bool is_integer() const { return false; }

    /// Test if this is a signed integer type.
    virtual bool is_signed_integer() const { return false; }

    /// Test if this is an unsigned integer type.
    virtual bool is_unsigned_integer() const { return false; }

    /// Test if this is a floating point type.
    virtual bool is_floating_point() const { return false; }

    /// Test if this is an array type.
    virtual bool is_array() const { return false; }

    /// Test if this is a pointer type.
    virtual bool is_pointer() const { return false; }

    /// Test if this is a struct type.
    virtual bool is_struct() const { return false; }
};

/// Represents types builtin to the language.
class BuiltinType final : public Type {
    friend class Context;

public:
    /// Possible kinds of builtin types.
    enum Kind : uint32_t {
        Void,
        Bool,
        Char,
        Int8,
        Int16,
        Int32,
        Int64,
        UInt8,
        UInt16,
        UInt32,
        UInt64,
        Float32,
        Float64,
        Float128,
    };

private:
    // The type of builtin kind this is.
    const Kind m_kind;

    BuiltinType(Kind kind) : m_kind(kind) {}

public:
    static const BuiltinType* get(Context& ctx, Kind kind);

    string to_string() const override;

    bool compare(const Type* other) const override;

    bool can_cast(const Type* other, bool implicitly = false) const override;

    bool is_void() const override { return m_kind == Void; }

    bool is_integer() const override { 
        return Bool <= m_kind && m_kind <= UInt64; 
    }

    bool is_signed_integer() const override { 
        return Bool <= m_kind && m_kind <= Int64; 
    }

    bool is_unsigned_integer() const override {
        return UInt8 <= m_kind && m_kind <= UInt64;
    }

    bool is_floating_point() const override { 
        return Float32 <= m_kind && m_kind <= Float128; 
    }

    Kind get_kind() const { return m_kind; }
};

/// Represents static array types.
class ArrayType final : public Type {
private:
    /// The element type of arrays represented by this type.
    const TypeUse m_element;

    /// The size of arrays represented by this type.
    const uint32_t m_size;

    ArrayType(const TypeUse& element, uint32_t size)
        : m_element(element), m_size(size) {}

public:
    static const ArrayType* get(Context& ctx, const TypeUse& element, 
                                uint32_t size);

    string to_string() const override {
        return '[' + std::to_string(m_size) + ']' + m_element.to_string();
    }

    bool compare(const Type* other) const override;

    bool can_cast(const Type* other, bool implicitly = false) const override;

    bool is_array() const override { return true; }

    const TypeUse& get_element_type() const { return m_element; }

    uint32_t get_size() const { return m_size; }
};

/// Represents composite pointer types.
class PointerType final : public Type {
private:
    /// The type that pointers of this type point to.
    const TypeUse m_pointee;

    PointerType(const TypeUse& pointee) : m_pointee(pointee) {}

public:
    static const PointerType* get(Context& ctx, const TypeUse& pointee);

    string to_string() const override { return '*' + m_pointee.to_string(); }

    bool compare(const Type* other) const override;

    bool can_cast(const Type* other, bool implicitly = false) const override;

    bool is_pointer() const override { return true; }

    const TypeUse& get_pointee() const { return m_pointee; }
};

/// Represents the type represented by a function signature i.e. a return type
/// and set of parameter types.
class FunctionType final : public Type {
private:
    /// The return type of this function signature type.
    const TypeUse m_ret;

    /// The list of parameter types of this function signature type.
    const vector<TypeUse> m_params;

    FunctionType(const TypeUse& ret, const vector<TypeUse>& params = {})
        : m_ret(ret), m_params(params) {}

public:
    static const FunctionType* get(Context& ctx, const TypeUse& ret, 
                                   const vector<TypeUse>& params);

    string to_string() const override;

    const TypeUse& get_return_type() const { return m_ret; }

    uint32_t num_params() const { return m_params.size(); }
    bool has_params() const { return !m_params.empty(); }

    const vector<TypeUse>& get_params() const { return m_params; }

    const TypeUse& get_param(uint32_t i) const {
        assert(i < num_params() && "index out of bounds!");
        return m_params[i];
    }
};

/// Returns named type aliases defined by an alias declaration.
class AliasType final : public Type {
private:
    // The declaration that defines this type.
    const AliasDecl* m_decl;

    TypeUse m_underlying;

    AliasType(const AliasDecl* decl) : m_decl(decl) {}

public:
    static const AliasType* create(Context& ctx, const AliasDecl* decl);
    static const AliasType* get(Context& ctx, const string& name);

    string to_string() const override;

    bool compare(const Type* other) const override { 
        return to_string() == other->to_string();
    }

    bool can_cast(const Type* other, bool implicitly = false) const override;

    void set_decl(const AliasDecl* decl) { m_decl = decl; }
    const AliasDecl* get_decl() const { return m_decl; }

    const TypeUse& get_underlying() const { return m_underlying; }
};

/// Represents named types defined by a struct declaration.
class StructType final : public Type {
private:
    /// The declaration that defines this type.
    const StructDecl* m_decl;

    StructType(const StructDecl* decl) : m_decl(decl) {}

public:
    static const StructType* create(Context& ctx, const StructDecl* decl);
    static const StructType* get(Context& ctx, const string& name);    

    string to_string() const override;

    bool compare(const Type* other) const override { 
        return to_string() == other->to_string(); 
    }

    bool is_struct() const override { return true; }

    const StructDecl* get_decl() const { return m_decl; }

    void set_decl(const StructDecl* decl) { m_decl = decl; }
};

/// Represents named types defined by an enum declaration.
class EnumType final : public Type {
private:
    /// The declaration that defines this type.
    const EnumDecl* m_decl;

    EnumType(const EnumDecl* decl) : m_decl(decl) {}

public:
    static const EnumType* create(Context& ctx, const EnumDecl* decl);
    static const EnumType* get(Context& ctx, const string& name);

    string to_string() const override;

    bool compare(const Type* other) const override {
        return to_string() == other->to_string();
    }

    bool can_cast(const Type* other, bool implicitly = false) const override;

    const EnumDecl* get_decl() const { return m_decl; }

    void set_decl(const EnumDecl* decl) { m_decl = decl; }
};

/// Represents the usage of a named type which was deferred at parse-time.
///
/// Instances of this class are given when an identifier is used in the 
/// context of a type signature but a type declaring symbol of the same name
/// could not be found. This typically results in either a resolution during
/// translation unit loads, or an unresolved type reference. 
class NamedTypeRef final : public Type {
private:
    /// The name of the type referenced.
    string m_name;

    const Type* m_underlying = nullptr;

    NamedTypeRef(const string& name) : m_name(name) {}

public:
    static const NamedTypeRef* get(Context& ctx, const string& name);

    string to_string() const override { return m_name; }

    const string& get_name() const { return m_name; }
    string& get_name() { return m_name; }

    bool has_underlying() const { return m_underlying != nullptr; }

    void set_underlying(const Type* type) { m_underlying = type; }
    const Type* get_underlying() const { return m_underlying; }
};

} // namespace stm

#endif // STATIM_TYPE_H_

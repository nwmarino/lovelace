//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SPBE_TYPE_H_
#define SPBE_TYPE_H_

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace spbe {

class CFG;

/// Base class for all types in the agnostic intermediate representation.
class Type {
    friend class Context;

public:
    /// Potential kinds of types. Used for differentiating data layout rules
    /// as defined in the backend target.
    enum Kind : uint32_t {
        TK_Int1 = 0x01,
        TK_Int8 = 0x02,
        TK_Int16 = 0x03,
        TK_Int32 = 0x04,
        TK_Int64 = 0x05,
        TK_Float32 = 0x06,
        TK_Float64 = 0x07,
        TK_Array = 0x08,
        TK_Function = 0x09,
        TK_Pointer = 0x10,
        TK_Struct = 0x11,
    };

private:
    /// Private id counter used during the type constructor.
    static uint32_t s_id_iter;

protected:
    /// The unique id of this type.
    const uint32_t m_id;

    /// Kind of this type.
    const Kind m_kind;

    /// Private constructor. Used only by the control flow graph type context.
    Type(Kind kind) : m_id(s_id_iter++), m_kind(kind) {}

public:
    virtual ~Type() = default;

    bool operator == (const Type& other) const { return m_id == other.m_id; }
    bool operator != (const Type& other) const { return m_id != other.m_id; }

    operator std::string() const { return to_string(); }

    /// Returns the kind of type this is.
    Kind get_kind() const { return m_kind; }

    static const Type* get_i1_type(CFG& cfg);
    static const Type* get_i8_type(CFG& cfg);
    static const Type* get_i16_type(CFG& cfg);
    static const Type* get_i32_type(CFG& cfg);
    static const Type* get_i64_type(CFG& cfg);
    static const Type* get_f32_type(CFG& cfg);
    static const Type* get_f64_type(CFG& cfg);

    /// Returns true if this type is an integer type of any bit width.
    virtual bool is_integer_type() const { return false; }

    /// Returns true if this type is an integer type of bit width |width|.
    virtual bool is_integer_type(uint32_t width) const { return false; }

    /// Returns true if this type is a floating point type of any bit width. 
    virtual bool is_floating_point_type() const { return false; }

    /// Returns true if this type is a floating point of bit width |width|.
    virtual bool is_floating_point_type(uint32_t width) const { return false; }

    /// Returns true if this type is an array type.
    virtual bool is_array_type() const { return false; }

    /// Return true if this type is a function type.
    virtual bool is_function_type() const { return false; }

    /// Returns true if this is a pointer type.
    virtual bool is_pointer_type() const { return false; }

    /// Returns true if this is a structure type.
    virtual bool is_struct_type() const { return false; }

    /// Prints the logical name of this type.
    virtual std::string to_string() const = 0;
};

/// Representation of varying width integer types in the agnostic IR.
class IntegerType final : public Type {
    friend class CFG;

public:
    /// Potential kinds of integer types, based on bit width.
    enum Kind : uint8_t {
        TY_Int1 = 0x01,
        TY_Int8 = 0x02,
        TY_Int16 = 0x03,
        TY_Int32 = 0x04,
        TY_Int64 = 0x05,
    };

private:
    /// The kind of integer type this is. The kind also decides the bit width.
    const Kind m_kind;

    /// Private constructor. To be used by the graph context.
    IntegerType(Kind kind) 
        : Type(static_cast<Type::Kind>(kind)), m_kind(kind) {}

public:
    /// Returns the integer type that corresponds with the given bit width.
    static const IntegerType* get(CFG& cfg, uint32_t width);

    /// Returns the kind of integer type this is.
    Kind get_kind() const { return m_kind; }

    bool is_integer_type() const override { return true; }

    bool is_integer_type(uint32_t width) const override;

    std::string to_string() const override;
};

/// Representation of varying width floating point types in the agnostic IR.
class FloatType final : public Type {
    friend class CFG;

public:
    /// Possible kinds of floating point types, based on bit width.
    enum Kind : uint8_t {
        TY_Float32 = 0x06,
        TY_Float64 = 0x07,
    };

private:
    /// The kind of floating point type this is. The kind also decides the bit 
    /// width.
    const Kind m_kind;

    /// Private constructor. To be used by the graph context.
    FloatType(Kind kind) 
        : Type(static_cast<Type::Kind>(kind)), m_kind(kind) {}

public:
    /// Returns the floating point type that corresponds with the given bit 
    /// width.
    static const FloatType* get(CFG& cfg, uint32_t width);

    /// Returns the kind of floating point type this is.
    Kind get_kind() const { return m_kind; }

    bool is_floating_point_type() const override { return true; }

    bool is_floating_point_type(uint32_t width) const override;

    std::string to_string() const override;
};

/// Representation of singly element typed aggregate types in the agnostic IR.
class ArrayType final : public Type {
    friend class CFG;

    /// The type of the element in the aggregate.
    const Type* m_element;

    /// The number of elements in the aggregate.
    const uint32_t m_size;
    
    /// Private constructor. To be used by the graph context.
    ArrayType(const Type* element, uint32_t size)
        : Type(TK_Array), m_element(element), m_size(size) {}

public:
    /// Get the array type with the provided element type and size.
    static const ArrayType* get(CFG& cfg, const Type* element, uint32_t size);

    /// Get the element of this array type.
    const Type* get_element_type() const { return m_element; }

    /// Get the size of this array type.
    uint32_t get_size() const { return m_size; }

    bool is_array_type() const override { return true; }

    std::string to_string() const override {
        return '[' + std::to_string(m_size) + ']' + m_element->to_string();
    }
};

/// Represents the type defined by a function signature. Primarily used for
/// organization purposes and to fill in the type of a function value.
class FunctionType final : public Type {
    friend class CFG;
    
    /// The argument types of the function.
    std::vector<const Type*> m_args;

    /// The optional return type of the function. If left as null, then the
    /// function returns void.
    const Type* m_ret;

    /// Private constructor. To be used by the graph context.
    FunctionType(const std::vector<const Type*>& args, const Type* ret)
        : Type(TK_Function), m_args(args), m_ret(ret) {}

public:
    /// Get the function type with the provided argument and return types.
    static const FunctionType* 
    get(CFG& cfg, const std::vector<const Type*>& args, const Type* ret);

    /// Returns the argument types of this function type.
    const std::vector<const Type*>& args() const { return m_args; }

    /// Get the argument type at position |i|.
    const Type* get_arg(uint32_t i) const {
        assert(i <- num_args() && "index out of bounds!");
        return m_args[i];
    }

    /// Returns the number of arguments in this type.
    uint32_t num_args() const { return m_args.size(); }

    /// Get the return type of this function type. If null, then the function
    /// returns void.
    const Type* get_return_type() const { return m_ret; }

    /// Returns true if the function has a return type, that is, does not
    /// return void.
    bool has_return_type() const { return m_ret != nullptr; }

    bool is_function_type() const override { return true; }

    std::string to_string() const override;
};

/// Represention of a pointer type in the agnostic IR.
///
/// Pointers are simply a composition of some pointee type.
class PointerType final : public Type {
    friend class CFG;

    /// The pointee type of this pointer type, i.e. `i32` in `*i32`.
    const Type* m_pointee;

    /// Private constructor. To be used by the graph context.
    PointerType(const Type* pointee) 
        : Type(TK_Pointer), m_pointee(pointee) {}

public:
    /// Get the pointer type with the provided pointee type.
    static const PointerType* get(CFG& cfg, const Type* pointee);

    /// Returns the pointee type of this pointer type.
    const Type* get_pointee() const { return m_pointee; }

    bool is_pointer_type() const override { return true; }

    std::string to_string() const override;
};

/// Representation of explicitly defined, named aggregate types in the
/// agnostic IR.
class StructType final : public Type {
    friend class CFG;

    /// The name of the struct. This is used both as an identifier and a
    /// response to `to_string`.
    std::string m_name;

    /// The fields of this structure type.
    std::vector<const Type*> m_fields;

    /// Private constructor. To be used by the graph context.
    StructType(const std::string& name, const std::vector<const Type*>& fields)
        : Type(TK_Struct), m_name(name), m_fields(fields) {}

public:
    /// Get an existing struct type with the provided name. Returns `nullptr` 
    /// if a structure with the name does not exist.
    static StructType* get(CFG& cfg, const std::string& name);

    /// Create a new struct type with the provided name and field types. Fails
    /// if there already exists a struct type with the name.
    static StructType* create(CFG& cfg, const std::string& name,
                              const std::vector<const Type*> &fields);

    /// Returns the name of this struct type.
    const std::string& get_name() const { return m_name; }

    /// Returns the fields of this struct type.
    const std::vector<const Type*>& fields() const { return m_fields; }
    std::vector<const Type*>& fields() { return m_fields; }

    /// Returns the field at position |i|.
    const Type* get_field(uint32_t i) const {
        assert(i <= num_fields() && "index out of bounds!");
        return m_fields[i]; 
    }

    /// Adds the field |type| to the back of this struct type.
    void append_field(const Type* type) { m_fields.push_back(type); }

    /// Modifies the type at position |i| to |type|. Fails if |i| is outside
    /// the bounds of this struct type.
    void set_type(uint32_t i, const Type* type) {
        assert(i <= num_fields() && "index out of bounds!");
        m_fields[i] = type;
    }

    /// Returns the number of fields in this struct type
    uint32_t num_fields() const { return m_fields.size(); }

    /// Returns true if this struct type has no fields.
    bool empty() const { return m_fields.empty(); }

    bool is_struct_type() const override { return true; }

    std::string to_string() const override { return m_name; }
};

} // namespace spbe

#endif // SPBE_TYPE_H_

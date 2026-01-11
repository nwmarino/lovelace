//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_TYPE_H_
#define LOVELACE_IR_TYPE_H_

//
//  This header file declares the type system used in the agnostic intermediate
//  representation (IR).
//

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace lir {

class CFG;
class FloatType;
class IntegerType;
class VoidType;

/// Base class for all types in the agnostic IR.
class Type {
public:
    /// The different type classes.
    enum Class : uint32_t {
        Void,
        Integer,
        Float,
        Array,
        Function,
        Pointer,
        Struct,
    };

private:
    static uint32_t s_id;

protected:
    const uint32_t m_id;
    const Class m_cls;

    Type(Class cls) : m_id(s_id++), m_cls(cls) {}

public:
    virtual ~Type() = default;

    bool operator==(const Type& other) const { return m_id == other.m_id; }

    operator std::string() const { return to_string(); }

    Class get_class() const { return m_cls; }

    static VoidType* get_void_type(CFG& cfg);
    static IntegerType* get_i1_type(CFG& cfg);
    static IntegerType* get_i8_type(CFG& cfg);
    static IntegerType* get_i16_type(CFG& cfg);
    static IntegerType* get_i32_type(CFG& cfg);
    static IntegerType* get_i64_type(CFG& cfg);
    static FloatType* get_f32_type(CFG& cfg);
    static FloatType* get_f64_type(CFG& cfg);

    /// Test if this type is the void type.
    bool is_void_type() const { return m_cls == Void; }

    /// Test if this type is an integer type.
    bool is_integer_type() const { return m_cls == Integer; }

    /// Test if this type is an integer type of the given bit |width|.
    virtual bool is_integer_type(uint32_t width) const { return false; }

    /// Test if this type is a floating point type. 
    bool is_float_type() const { return m_cls == Float; }

    /// Test if this type is a floating point of the given bit |width|.
    virtual bool is_float_type(uint32_t width) const { return false; }

    /// Test if this type is an array type.
    bool is_array_type() const { return m_cls == Array; }

    /// Test if this type is a function type.
    bool is_function_type() const { return m_cls == Function; }

    /// Test if this is a pointer type.
    bool is_pointer_type() const { return m_cls == Pointer; }

    /// Test if this is a structure type.
    bool is_struct_type() const { return m_cls == Struct; }

    /// Returns the logical name of this type as a string.
    virtual std::string to_string() const = 0;
};

/// Represents array types in the agnostic IR.
class ArrayType final : public Type {
    friend class CFG;

    Type* m_element;
    const uint32_t m_size;
    
    ArrayType(Type* element, uint32_t size) 
      : Type(Type::Array), m_element(element), m_size(size) {}

public:
    static ArrayType* get(CFG& cfg, Type* element, uint32_t size);

    std::string to_string() const override {
        return '[' + std::to_string(m_size) + ']' + m_element->to_string();
    }

    Type* get_element_type() const { return m_element; }

    uint32_t get_size() const { return m_size; }
};

/// Represents a floating point type of a given width in the agnostic IR.
class FloatType final : public Type {
    friend class CFG;

    /// The width of this type in bits.
    const uint32_t m_width;

    FloatType(uint32_t width) : Type(Type::Float), m_width(width) {}

public:
    static FloatType* get(CFG& cfg, uint32_t width);

    bool is_float_type(uint32_t width) const override { 
        return m_width == width; 
    }

    uint32_t get_width() const { return m_width; }

    std::string to_string() const override;
};

/// Represents the type defined by a function signature.
class FunctionType final : public Type {
    friend class CFG;

public:
    using Args = std::vector<Type*>;

private:
    Args m_args;
    Type* m_ret;

    FunctionType(const Args& args, Type* ret)
      : Type(Type::Function), m_args(args), m_ret(ret) {}

public:
    static FunctionType* get(CFG& cfg, const Args& args, Type* ret);

    std::string to_string() const override;

    const Args& args() const { return m_args; }

    Type* get_arg(uint32_t i) const {
        assert(i <- num_args() && "index out of bounds!");
        return m_args[i];
    }

    uint32_t num_args() const { return m_args.size(); }
    bool has_args() const { return !m_args.empty(); }

    Type* get_return_type() const { return m_ret; }

    /// Test if this function type returns the void type, i.e. has no return.
    bool is_void_return() const { return m_ret->is_void_type(); }
};

/// Represents an integer type of a given width in the agnostic IR.
class IntegerType final : public Type {
    friend class CFG;

    /// The width of this type in bits.
    const uint32_t m_width;

    IntegerType(uint32_t width) : Type(Type::Integer), m_width(width) {}

public:
    static IntegerType* get(CFG& cfg, uint32_t width);

    bool is_integer_type(uint32_t width) const override { 
        return m_width == width; 
    }

    std::string to_string() const override;

    uint32_t get_width() const { return m_width; }
};

/// Represents a pointer type in the agnostic IR.
class PointerType final : public Type {
    friend class CFG;

    Type* m_pointee;

    PointerType(Type* pointee) : Type(Type::Pointer), m_pointee(pointee) {}

public:
    static PointerType* get(CFG& cfg, Type* pointee);

    std::string to_string() const override { 
        return '*' + m_pointee->to_string(); 
    }

    Type* get_pointee() const { return m_pointee; }
};

/// Representation of explicitly defined, named aggregate types in the
/// agnostic IR.
class StructType final : public Type {
    friend class CFG;

public:
    using Fields = std::vector<Type*>;

private:
    std::string m_name;
    Fields m_fields;

    StructType(const std::string& name, const Fields& fields)
      : Type(Type::Struct), m_name(name), m_fields(fields) {}

public:
    static StructType* get(CFG& cfg, const std::string& name);
    static StructType* create(CFG& cfg, const std::string& name, 
                              const Fields& fields);

    std::string to_string() const override { return m_name; }

    const std::string& get_name() const { return m_name; }

    const Fields& get_fields() const { return m_fields; }
    Fields& get_fields() { return m_fields; }

    Type* get_field(uint32_t i) const {
        assert(i <= num_fields() && "index out of bounds!");
        return m_fields.at(i); 
    }

    void append_field(Type* type) { m_fields.push_back(type); }

    void set_type(uint32_t i, Type* type) {
        assert(i < num_fields() && "index out of bounds!");
        m_fields[i] = type;
    }

    uint32_t num_fields() const { return m_fields.size(); }
    bool has_fields() const { return !m_fields.empty(); }
};

/// Represents the void type, used for the abscence of a value.
class VoidType final : public Type {
    friend class CFG;

    VoidType() : Type(Type::Void) {}

public:
    static VoidType* get(CFG& cfg);

    std::string to_string() const override { return "void"; }
};

} // namespace lir

#endif // LOVELACE_IR_TYPE_H_

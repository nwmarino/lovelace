//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "scc/ast/Decl.hpp"
#include "scc/ast/Type.hpp"
#include "scc/ast/TypeContext.hpp"

#include <cassert>

using namespace scc;

bool BuiltinType::operator == (const Type& other) const {
    if (other.get_kind() != Builtin)
        return false;

    return m_bt_kind == static_cast<const BuiltinType*>(&other)->m_bt_kind;
}

const BuiltinType* BuiltinType::get_void_type(TypeContext& ctx) {
    return ctx.m_bts[Void];
}

const BuiltinType* BuiltinType::get_uchar_type(TypeContext& ctx) {
    return ctx.m_bts[UChar];
}

const BuiltinType* BuiltinType::get_char_type(TypeContext& ctx) {
    return ctx.m_bts[Char];
}

const BuiltinType* BuiltinType::get_ushort_type(TypeContext& ctx) {
    return ctx.m_bts[UShort];
}

const BuiltinType* BuiltinType::get_short_type(TypeContext& ctx) {
    return ctx.m_bts[Short];
}

const BuiltinType* BuiltinType::get_uint_type(TypeContext& ctx) {
    return ctx.m_bts[UInt];
}

const BuiltinType* BuiltinType::get_int_type(TypeContext& ctx) {
    return ctx.m_bts[Int];
}

const BuiltinType* BuiltinType::get_ulong_type(TypeContext& ctx) {
    return ctx.m_bts[ULong];
}

const BuiltinType* BuiltinType::get_long_type(TypeContext& ctx) {
    return ctx.m_bts[Long];
}

const BuiltinType* BuiltinType::get_ulonglong_type(TypeContext& ctx) {
    return ctx.m_bts[ULongLong];
}

const BuiltinType* BuiltinType::get_longlong_type(TypeContext& ctx) {
    return ctx.m_bts[LongLong];
}

const BuiltinType* BuiltinType::get_float_type(TypeContext& ctx) {
    return ctx.m_bts[Float];
}

const BuiltinType* BuiltinType::get_double_type(TypeContext& ctx) {
    return ctx.m_bts[Double];
}

const BuiltinType* BuiltinType::get_longdouble_type(TypeContext& ctx) {
    return ctx.m_bts[LongDouble];
}

bool BuiltinType::is_signed_integer() const {
    switch (m_bt_kind) {
    case Char:
    case Short:
    case Int:
    case Long:
    case LongLong:
        return true;
    default:
        return false;
    }
}

bool BuiltinType::is_unsigned_integer() const {
    switch (m_bt_kind) {
    case UChar:
    case UShort:
    case UInt:
    case ULong:
    case ULongLong:
        return true;
    default:
        return false;
    }
}

string BuiltinType::to_string() const {
    switch (m_bt_kind) {
    case Void:          return "void";
    case Char:          return "char";
    case UChar:         return "unsigned char";
    case Short:         return "short";
    case UShort:        return "unsigned short";
    case Int:           return "int";
    case UInt:          return "unsigned int";
    case Long:          return "long";
    case ULong:         return "unsigned long";
    case LongLong:      return "long long";
    case ULongLong:     return "unsigned long long";
    case Float:         return "float";
    case Double:        return "double";
    case LongDouble:    return "long double";
    }
}

bool ArrayType::operator == (const Type& other) const {
    if (other.get_kind() != Array)
        return false;

    const ArrayType* other_array = static_cast<const ArrayType*>(&other);
    return m_element == other_array->get_element() 
        && m_size == other_array->m_size;
}

const ArrayType* ArrayType::get(
        TypeContext& ctx, const QualType& element, uint32_t size) {
    ArrayType* ty = new ArrayType(element, size);
    ctx.m_arrays.push_back(ty);
    return ty;
}

bool PointerType::operator == (const Type& other) const {
    if (other.get_kind() != Pointer)
        return false;

    return m_pointee == static_cast<const PointerType*>(&other)->get_pointee();
}

const PointerType* PointerType::get(TypeContext& ctx, const QualType& pointee) {
    PointerType* ty = new PointerType(pointee);
    ctx.m_ptrs.push_back(ty);
    return ty;
}

const PointerType* PointerType::get_char_p(TypeContext& ctx) {
    return PointerType::get(ctx, BuiltinType::get_char_type(ctx));
}

const PointerType* PointerType::get_char_pp(TypeContext& ctx) {
    return PointerType::get(ctx, get_char_p(ctx));
}

bool FunctionType::operator == (const Type& other) const {
    if (other.get_kind() != Function)
        return false;

    const FunctionType* other_function = static_cast<const FunctionType*>(&other);
    if (num_params() != other_function->num_params())
        return false;

    for (uint32_t i = 0; i < num_params(); ++i)
        if (m_params[i] != other_function->m_params[i])
            return false;

    return m_ret == other_function->get_return_type();
}

const FunctionType* FunctionType::get(
        TypeContext& ctx, const QualType& ret, const vector<QualType>& params) {
    FunctionType* ty = new FunctionType(ret, params);
    ctx.m_sigs.push_back(std::move(ty));
    return ty;
}

string FunctionType::to_string() const {
    std::string str = m_ret.to_string() + " (";

    for (uint32_t i = 0, e = num_params(); i < e; ++i) {
        str += get_param_type(i).to_string();
        if (i + 1 != e)
            str += ", ";
    }

    return str + ')';
}

bool TypedefType::operator == (const Type& other) const {
    if (other.get_kind() == Typedef) {
        return m_underlying == 
            static_cast<const TypedefType*>(&other)->m_underlying;
    } else {
        return m_underlying == QualType(&other);
    }
}

const TypedefType* TypedefType::create(
        TypeContext& ctx, const TypedefDecl* decl, const QualType& underlying) {
    TypedefType* ty = new TypedefType(decl, underlying);
    ctx.m_typedefs.push_back(ty);
    return ty;
}

string TypedefType::to_string() const {
    return m_decl->get_name();
}

bool RecordType::operator == (const Type& other) const {
    if (other.get_kind() != Record)
        return false;

    return m_decl == static_cast<const RecordType*>(&other)->m_decl;
}

const RecordType* RecordType::create(TypeContext& ctx, const RecordDecl* decl) {
    RecordType* ty = new RecordType(decl);
    ctx.m_records.push_back(ty);
    return ty;
}

string RecordType::to_string() const {
    return (m_decl->is_struct() ? "struct " : "union ") + m_decl->get_name();
}

bool EnumType::operator == (const Type& other) const {
    if (other.get_kind() != Enum)
        return false;

    return m_decl == static_cast<const EnumType*>(&other)->m_decl;
}

const EnumType* EnumType::create(TypeContext& ctx, const EnumDecl* decl) {
    EnumType* ty = new EnumType(decl);
    ctx.m_enums.push_back(ty);
    return ty;
}

string EnumType::to_string() const {
    return "enum " + m_decl->get_name();
}

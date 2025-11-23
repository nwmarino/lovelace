//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "ast/Context.hpp"
#include "ast/Decl.hpp"
#include "ast/Type.hpp"

#include <cassert>
#include <memory>

using namespace scc;

Type::id_t Type::s_id = 0;

const BuiltinType* BuiltinType::get_void_type(Context& ctx) {
    return ctx.m_bts[Void].get();
}

const BuiltinType* BuiltinType::get_uchar_type(Context& ctx) {
    return ctx.m_bts[UChar].get();
}

const BuiltinType* BuiltinType::get_char_type(Context& ctx) {
    return ctx.m_bts[Char].get();
}

const BuiltinType* BuiltinType::get_ushort_type(Context& ctx) {
    return ctx.m_bts[UShort].get();
}

const BuiltinType* BuiltinType::get_short_type(Context& ctx) {
    return ctx.m_bts[Short].get();
}

const BuiltinType* BuiltinType::get_uint_type(Context& ctx) {
    return ctx.m_bts[UInt].get();
}

const BuiltinType* BuiltinType::get_int_type(Context& ctx) {
    return ctx.m_bts[Int].get();
}

const BuiltinType* BuiltinType::get_ulong_type(Context& ctx) {
    return ctx.m_bts[ULong].get();
}

const BuiltinType* BuiltinType::get_long_type(Context& ctx) {
    return ctx.m_bts[Long].get();
}

const BuiltinType* BuiltinType::get_ulonglong_type(Context& ctx) {
    return ctx.m_bts[ULongLong].get();
}

const BuiltinType* BuiltinType::get_longlong_type(Context& ctx) {
    return ctx.m_bts[LongLong].get();
}

const BuiltinType* BuiltinType::get_float_type(Context& ctx) {
    return ctx.m_bts[Float].get();
}

const BuiltinType* BuiltinType::get_double_type(Context& ctx) {
    return ctx.m_bts[Double].get();
}

const BuiltinType* BuiltinType::get_longdouble_type(Context& ctx) {
    return ctx.m_bts[LongDouble].get();
}

bool BuiltinType::is_signed_integer() const {
    switch (m_kind) {
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
    switch (m_kind) {
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

std::string BuiltinType::to_string() const {
    switch (m_kind) {
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

const ArrayType* ArrayType::get(
        Context& ctx, const QualType& element, uint32_t size) {
    auto ty = std::unique_ptr<ArrayType>(new ArrayType(element, size));
    const ArrayType* pTy = ty.get();

    ctx.m_arrays.push_back(std::move(ty));
    return pTy;
}

const PointerType* PointerType::get(Context& ctx, const QualType& pointee) {
    auto ty = std::unique_ptr<PointerType>(new PointerType(pointee));
    const PointerType* pTy = ty.get();

    ctx.m_ptrs.push_back(std::move(ty));
    return pTy;
}

const FunctionType* FunctionType::get(
        Context& ctx, const QualType& ret, const std::vector<QualType>& params) {
    auto ty = std::unique_ptr<FunctionType>(new FunctionType(ret, params));
    const FunctionType* pTy = ty.get();

    ctx.m_sigs.push_back(std::move(ty));
    return pTy;
}

std::string FunctionType::to_string() const {
    std::string str = m_ret->to_string() + " (";

    for (uint32_t i = 0, e = num_params(); i < e; ++i) {
        str += get_param_type(i)->to_string();
        if (i + 1 != e)
            str += ", ";
    }

    return str + ')';
}

const TypedefType* TypedefType::create(
        Context& ctx, const TypedefDecl* decl, const QualType& underlying) {
    auto ty = std::unique_ptr<TypedefType>(new TypedefType(decl, underlying));
    const TypedefType* pTy = ty.get();

    ctx.m_typedefs.push_back(std::move(ty));
    return pTy;
} 

std::string TypedefType::to_string() const {
    return m_decl->name();
}

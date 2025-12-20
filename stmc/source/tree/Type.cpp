//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/tree/Context.hpp"
#include "stmc/tree/Decl.hpp"
#include <cassert>
#include <sys/ucontext.h>
#include "stmc/tree/Type.hpp"

using namespace stm;

const BuiltinType* BuiltinType::get(Context &ctx, Kind kind) {
    return ctx.m_builtins[kind];
}

string BuiltinType::to_string() const {
    switch (get_kind()) {
    case Void:      return "void";
    case Bool:      return "bool";
    case Char:      return "char";
    case Int8:      return "i8";
    case Int16:     return "i16";
    case Int32:     return "i32";
    case Int64:     return "i64";
    case UInt8:     return "u8";
    case UInt16:    return "u16";
    case UInt32:    return "u32";
    case UInt64:    return "u64";
    case Float32:   return "f32";
    case Float64:   return "f64";
    case Float128:  return "f128";
    }
}

const ArrayType* ArrayType::get(
        Context &ctx, const TypeUse &element, uint32_t size) {
    auto type_it = ctx.m_arrays.find(element);
    if (type_it != ctx.m_arrays.end()) {
        auto size_it = type_it->second.find(size);
        if (size_it != type_it->second.end())
            return size_it->second;

        ArrayType* type = new ArrayType(element, size);
        type_it->second[size] = type;
        return type;
    }

    ArrayType* type = new ArrayType(element, size);
    ctx.m_arrays[element] = {};
    ctx.m_arrays[element][size] = type;
    return type;
}

const PointerType* PointerType::get(Context &ctx, const TypeUse &pointee) {
    auto it = ctx.m_pointers.find(pointee);
    if (it != ctx.m_pointers.end())
        return it->second;

    PointerType* type = new PointerType(pointee);
    ctx.m_pointers[pointee] = type;
    return type;
}

const FunctionType* FunctionType::get(
        Context &ctx, const TypeUse &ret, const vector<TypeUse> &params) {
    FunctionType* type = new FunctionType(ret, params);
    ctx.m_functions.push_back(type);
    return type;
}

string FunctionType::to_string() const {
    string str = "(";
    for (uint32_t i = 0, e = num_params(); i != e; ++i) {
        str += get_param(i)->to_string();
        if (i + 1 != e)
            str += ", ";
    }

    return ") -> " + get_return_type()->to_string();
}

const AliasType* AliasType::create(Context &ctx, const AliasDecl *decl) {
    auto it = ctx.m_aliases.find(decl->get_name());
    if (it != ctx.m_aliases.end())
        return nullptr;

    AliasType* type = new AliasType(decl);
    ctx.m_aliases[decl->get_name()] = type;
    return type;
}

const AliasType* AliasType::get(Context &ctx, const string &name) {
    auto it = ctx.m_aliases.find(name);
    if (it != ctx.m_aliases.end())
        return it->second;

    return nullptr;
}

string AliasType::to_string() const {
    assert(m_decl && "type has no declaration set!");
    return m_decl->get_name();
}

const StructType* StructType::create(Context &ctx, const StructDecl *decl) {
    auto it = ctx.m_structs.find(decl->get_name());
    if (it != ctx.m_structs.end())
        return nullptr;

    StructType* type = new StructType(decl);
    ctx.m_structs[decl->get_name()] = type;
    return type;
}

const StructType* StructType::get(Context &ctx, const string &name) {
    auto it = ctx.m_structs.find(name);
    if (it != ctx.m_structs.end())
        return it->second;

    return nullptr;
}

string StructType::to_string() const {
    assert(m_decl && "type has no declaration set!");
    return m_decl->get_name();
}

const EnumType* EnumType::create(Context &ctx, const EnumDecl *decl) {
    auto it = ctx.m_enums.find(decl->get_name());
    if (it != ctx.m_enums.end())
        return nullptr;

    EnumType* type = new EnumType(decl);
    ctx.m_enums[decl->get_name()] = type;
    return type;
}

const EnumType* EnumType::get(Context &ctx, const string &name) {
    auto it = ctx.m_enums.find(name);
    if (it != ctx.m_enums.end())
        return it->second;

    return nullptr;
}

string EnumType::to_string() const {
    assert(m_decl && "type has no declaration set!");
    return m_decl->get_name();
}

const NamedTypeRef* NamedTypeRef::get(Context& ctx, const string& name) {
    auto it = ctx.m_deferred.find(name);
    if (it != ctx.m_deferred.end())
        return it->second;

    NamedTypeRef* type = new NamedTypeRef(name);
    ctx.m_deferred.emplace(name, type);
    return type;
}

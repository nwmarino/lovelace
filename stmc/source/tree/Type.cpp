//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/tree/Context.hpp"
#include "stmc/tree/Decl.hpp"
#include "stmc/types/SourceLocation.hpp"
#include "stmc/tree/Type.hpp"

#include <cassert>

using namespace stm;

const BuiltinType* BuiltinType::get(Context &ctx, Kind kind) {
    return ctx.m_builtins[kind];
}

string BuiltinType::to_string() const {
    switch (get_kind()) {
    case Void:      return "void";
    case Bool:      return "bool";
    case Char:      return "char";
    case Int8:      return "s8";
    case Int16:     return "s16";
    case Int32:     return "s32";
    case Int64:     return "s64";
    case UInt8:     return "u8";
    case UInt16:    return "u16";
    case UInt32:    return "u32";
    case UInt64:    return "u64";
    case Float32:   return "f32";
    case Float64:   return "f64";
    case Float128:  return "f128";
    }
}

bool BuiltinType::compare(const Type* other) const {
    assert(other && "other type cannot be null!");

    auto BT = dynamic_cast<const BuiltinType*>(other);
    return BT && get_kind() == BT->get_kind();
}

bool BuiltinType::can_cast(const Type* other, bool implicitly) const {
    assert(other && "other type cannot be null!");

    auto BT = dynamic_cast<const BuiltinType*>(other);
    if (implicitly) {
        if (!BT)
            return false;

        if (is_floating_point() && other->is_integer())
            return false;

        return is_void() == BT->is_void();
    } else {
        if (BT)
            return is_void() == BT->is_void();

        if (other->is_pointer())
            return is_integer();

        return false;
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

bool ArrayType::compare(const Type* other) const {
    assert(other && "other type cannot be null!");

    auto AT = dynamic_cast<const ArrayType*>(other);
    return AT && get_size() == AT->get_size() 
        && get_element_type().compare(AT->get_element_type());
}

bool ArrayType::can_cast(const Type* other, bool implicitly) const {
    assert(other && "other type cannot be null!");

    // Can only cast [...]T -> *T.
    if (!other->is_pointer())
        return false;

    return get_element_type().can_cast(
        static_cast<const PointerType*>(other)->get_pointee());
}

const PointerType* PointerType::get(Context &ctx, const TypeUse &pointee) {
    auto it = ctx.m_pointers.find(pointee);
    if (it != ctx.m_pointers.end())
        return it->second;

    PointerType* type = new PointerType(pointee);
    ctx.m_pointers[pointee] = type;
    return type;
}

bool PointerType::compare(const Type* other) const {
    assert(other && "other type cannot be null!");

    auto PT = dynamic_cast<const PointerType*>(other);
    return PT && get_pointee().compare(PT->get_pointee());
}

bool PointerType::can_cast(const Type* other, bool implicitly) const {
    assert(other && "other type cannot be null!");

    if (implicitly) {
        // Can implicitly cast *void -> *T.
        if (get_pointee()->is_void())
            return true;

        // Cannot implicitly cast away pointer indirection.
        if (!other->is_pointer())
            return false;

        // Can implicitly cast *T -> *void.
        return static_cast<const PointerType*>(other)->get_pointee()->is_void();
    } else {
        // Can explicitly cast to other pointer types or integers.
        return other->is_pointer() || other->is_integer();
    }
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

const AliasType* AliasType::create(Context &ctx, const TypeUse& underlying,
                                   const AliasDecl *decl) {
    auto it = ctx.m_aliases.find(decl->get_name());
    if (it != ctx.m_aliases.end())
        return nullptr;

    AliasType* type = new AliasType(underlying, decl);
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

bool AliasType::can_cast(const Type* other, bool implicitly) const {
    assert(other && "other type cannot be null!");
    return get_underlying().can_cast(other, implicitly);
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

const EnumType* EnumType::create(
        Context &ctx, const TypeUse& underlying, const EnumDecl *decl) {
    auto it = ctx.m_enums.find(decl->get_name());
    if (it != ctx.m_enums.end())
        return nullptr;

    EnumType* type = new EnumType(underlying, decl);
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

bool EnumType::can_cast(const Type* other, bool implicitly) const {
    assert(other && "other type cannot be null!");
    return other->is_integer();
}

const UnresolvedType* UnresolvedType::get(
        Context& ctx, const string& name, SourceLocation loc) {
    auto it = ctx.m_unresolved.find(name);
    if (it != ctx.m_unresolved.end())
        return it->second;

    UnresolvedType* type = new UnresolvedType(name, loc);
    ctx.m_unresolved.emplace(name, type);
    return type;
}

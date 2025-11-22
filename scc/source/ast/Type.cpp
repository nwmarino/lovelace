//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "ast/Context.hpp"
#include "ast/Type.hpp"

#include <cassert>
#include <memory>

using namespace scc;

Type::id_t Type::s_id = 0;

const VoidType* VoidType::get(Context& ctx) {
    using TK = Context::TypeKind;
    return static_cast<const VoidType*>(ctx.m_prims[TK::Void].get());
}

const IntegerType* IntegerType::get(
        Context& ctx, uint32_t bits, bool is_signed) {
    using TK = Context::TypeKind;

    const Type* ty = nullptr;
    switch (bits) {
    case 8:
        ty = (is_signed ? 
            ctx.m_prims[TK::Char].get() : ctx.m_prims[TK::UChar].get());
        break;
    case 16:
        ty = (is_signed ? 
            ctx.m_prims[TK::Short].get() : ctx.m_prims[TK::UShort].get());
        break;
    case 32:
        ty = (is_signed ? 
            ctx.m_prims[TK::Int].get() : ctx.m_prims[TK::UInt].get());
        break;
    case 64:
        ty = (is_signed ? 
            ctx.m_prims[TK::Long].get() : ctx.m_prims[TK::ULong].get());
        break;
    }

    assert(ty != nullptr && "invalid bits for an integer type!");
    return static_cast<const IntegerType*>(ty);
}

std::string IntegerType::to_string() const {
    switch (m_bits) {
    case 8:
        return (m_signed ? "char" : "unsigned char");
    case 16:
        return (m_signed ? "short" : "unsigned short");
    case 32:
        return (m_signed ? "int" : "unsigned int");
    case 64:
        return (m_signed ? "long" : "unsigned long");
    default:
        assert(false && "invalid bits for an integer type!");
    }
}

const FPType* FPType::get(Context& ctx, uint32_t bits) {
    using TK = Context::TypeKind;

    const Type* ty = nullptr;
    switch (bits) {
    case 32:
        ty = ctx.m_prims[TK::Float].get();
        break;
    case 64:
        ty = ctx.m_prims[TK::Double].get();
        break;
    }

    assert(ty != nullptr && "invalid bits for a floating point type!");
    return static_cast<const FPType*>(ty);
}

std::string FPType::to_string() const {
    switch (m_bits) {
    case 32:
        return "float";
    case 64:
        return "double";
    default:
        assert(false && "invalid bits for a floating point type!");
    }
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

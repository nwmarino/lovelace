//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lir/graph/CFG.hpp"
#include "lir/graph/Type.hpp"

using namespace lir;

uint32_t Type::s_id = 0;

VoidType* Type::get_void_type(CFG& cfg) {
    return cfg.m_types.void_type;
}

IntegerType* Type::get_i1_type(CFG& cfg) {
    return cfg.m_types.ints[1];
}

IntegerType* Type::get_i8_type(CFG& cfg) {
    return cfg.m_types.ints[8];
}

IntegerType* Type::get_i16_type(CFG& cfg) {
    return cfg.m_types.ints[16];
}

IntegerType* Type::get_i32_type(CFG& cfg) {
    return cfg.m_types.ints[32];
}

IntegerType* Type::get_i64_type(CFG& cfg) {
    return cfg.m_types.ints[64];
}

FloatType* Type::get_f32_type(CFG& cfg) {
    return cfg.m_types.floats[32];
}

FloatType* Type::get_f64_type(CFG& cfg) {
    return cfg.m_types.floats[64];
}

ArrayType* ArrayType::get(CFG& cfg, Type* element, uint32_t size) {
    auto element_it = cfg.m_types.arrays.find(element);
    if (element_it != cfg.m_types.arrays.end()) {
        auto size_it = element_it->second.find(size);
        if (size_it != element_it->second.end()) {
            return size_it->second;
        } else {
            ArrayType* type = new ArrayType(element, size);
            element_it->second.emplace(size, type);
            return type;
        }
    } else {
        ArrayType* type = new ArrayType(element, size);
        cfg.m_types.arrays.emplace(type, std::unordered_map<uint32_t, ArrayType*>());
        cfg.m_types.arrays[type].emplace(size, type);
        return type;
    }
}

FloatType* FloatType::get(CFG& cfg, uint32_t width) {
    switch (width) {
        case 32:
            return static_cast<FloatType*>(Type::get_f32_type(cfg));
        case 64:
            return static_cast<FloatType*>(Type::get_f64_type(cfg));
    }

    assert(false && "incompatible bit width!");
}

std::string FloatType::to_string() const {
    switch (m_width) {
        case 32:
            return "f32";
        case 64:
            return "f64";
    }

    assert(false && "incompatible bit width!");
}

FunctionType *FunctionType::get(CFG &cfg, const Params &params, Type *result) {
    FunctionType *type = new FunctionType(params, result);
    assert(type);

    cfg.m_types.functions.push_back(type);
    return type;
}

std::string FunctionType::to_string() const {
    std::string str = "(";
    
    for (uint32_t i = 0, e = num_params(); i < e; ++i) {
        str += m_params[i]->to_string();
        if (i + 1 != e)
            str += ", ";
    }

    str += ")";

    if (has_result())
        str += " -> " + m_result->to_string();
    
    return str;
}

static IntegerType* get(CFG& cfg, uint32_t width) {
    switch (width) {
        case 1:
            return static_cast<IntegerType*>(Type::get_i1_type(cfg));
        case 8:
            return static_cast<IntegerType*>(Type::get_i8_type(cfg));
        case 16:
            return static_cast<IntegerType*>(Type::get_i16_type(cfg));
        case 32:
            return static_cast<IntegerType*>(Type::get_i32_type(cfg));
        case 64:
            return static_cast<IntegerType*>(Type::get_i64_type(cfg));
    }

    assert(false && "incompatible bit width!");
}

std::string IntegerType::to_string() const {
    switch (m_width) {
        case 1:
            return "i1";
        case 8:
            return "i8";
        case 16:
            return "i16";
        case 32:
            return "i32";
        case 64:
            return "i64";
    }

    assert(false && "incompatible bit width!");
}

PointerType* PointerType::get(CFG& cfg, Type* pointee) {
    auto it = cfg.m_types.pointers.find(pointee);
    if (it != cfg.m_types.pointers.end())
        return it->second;

    PointerType* type = new PointerType(pointee);
    cfg.m_types.pointers.emplace(pointee, type);
    return type;
}

PointerType* PointerType::get_void_pointer(CFG& cfg) {
    return get(cfg, lir::VoidType::get(cfg));
}

PointerType* PointerType::get_i8_pointer(CFG& cfg) {
    return get(cfg, lir::IntegerType::get_i8_type(cfg));
}

StructType* StructType::get(CFG& cfg, const std::string& name) {
    auto it = cfg.m_types.structs.find(name);
    if (it != cfg.m_types.structs.end())
        return it->second;

    return nullptr;
}

StructType* StructType::create(CFG& cfg, const std::string& name,
                               const Fields& fields) {
    assert(!get(cfg, name) && "struct type with name already exists!");

    StructType* type = new StructType(name, fields);
    assert(type && "failed to create struct type!");
    cfg.m_types.structs.emplace(name, type);
    return type;
}

VoidType* VoidType::get(CFG& cfg) {
    return cfg.m_types.void_type;
}

//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lir/graph/CFG.hpp"
#include "lir/graph/Constant.hpp"
#include "lir/graph/Type.hpp"

#include <cstdint>

using namespace lir;

Integer* Integer::get_true(CFG& cfg) {
    return cfg.m_constants.one;
}

Integer* Integer::get_false(CFG& cfg) {
    return cfg.m_constants.zero;
}

Integer* Integer::get_zero(CFG& cfg, Type* type) {
    return get(cfg, type, 0);
}

Integer* Integer::get_one(CFG& cfg, Type* type) {
    return get(cfg, type, 1);
}

Integer* Integer::get(CFG& cfg, Type* type, int64_t value) {
    assert(type->is_integer_type() && "integer type must also be an integer!");

    switch (static_cast<const IntegerType*>(type)->get_width()) {
        case 1:
            return value == 0 ? cfg.m_constants.zero : cfg.m_constants.one;

        case 8: {
            auto it = cfg.m_constants.bytes.find(value);
            if (it != cfg.m_constants.bytes.end())
                return it->second;

            Integer* integer = new Integer(value, type);
            cfg.m_constants.bytes.emplace(value, integer);
            return integer;
        }
            
        case 16: {
            auto it = cfg.m_constants.shorts.find(value);
            if (it != cfg.m_constants.shorts.end())
                return it->second;

            Integer* integer = new Integer(value, type);
            cfg.m_constants.shorts.emplace(value, integer);
            return integer;
        }
        
        case 32: {
            auto it = cfg.m_constants.ints.find(value);
            if (it != cfg.m_constants.ints.end())
                return it->second;

            Integer* integer = new Integer(value, type);
            cfg.m_constants.ints.emplace(value, integer);
            return integer;
        }
        
        case 64: {
            auto it = cfg.m_constants.longs.find(value);
            if (it != cfg.m_constants.longs.end())
                return it->second;

            Integer* integer = new Integer(value, type);
            cfg.m_constants.longs.emplace(value, integer);
            return integer;
        }

        default:
            assert(false && "invalid bit width!");
    }
}

Float* Float::get_zero(CFG& cfg, Type* type) {
    return get(cfg, type, 0);
}

Float* Float::get_one(CFG& cfg, Type* type) {
    return get(cfg, type, 1);
}

Float* Float::get(CFG& cfg, Type* type, double value) {
    assert(type->is_float_type() && "float type must also be a float!");

    switch (static_cast<const FloatType*>(type)->get_width()) {
        case 32: {
            auto it = cfg.m_constants.floats.find(value);
            if (it != cfg.m_constants.floats.end())
                return it->second;

            Float* fp = new Float(value, type);
            cfg.m_constants.floats.emplace(value, fp);
            return fp;
        }

        case 64: {
            auto it = cfg.m_constants.doubles.find(value);
            if (it != cfg.m_constants.doubles.end())
                return it->second;

            Float* fp = new Float(value, type);
            cfg.m_constants.doubles.emplace(value, fp);
            return fp;
        }

        default:
            assert(false && "invalid bit width!");
    }
}

Null* Null::get(CFG& cfg, Type* type) {
    assert(type && "null constant must have a type!");

    auto it = cfg.m_constants.nulls.find(type);
    if (it != cfg.m_constants.nulls.end())
        return it->second;

    Null* null = new Null(type);
    cfg.m_constants.nulls.emplace(type, null);
    return null;
}

String* String::get(CFG& cfg, const std::string& string) {
    auto it = cfg.m_constants.strings.find(string);
    if (it != cfg.m_constants.strings.end())
        return it->second;

    String* str = new String(
        PointerType::get(cfg, Type::get_i8_type(cfg)),
        string);
        //ArrayType::get(cfg, Type::get_i8_type(cfg), string.size() + 1));

    cfg.m_constants.strings.emplace(string, str);
    return str;
}

Constant* BlockAddress::get(CFG& cfg, BasicBlock* block) {
    assert(block && "block address must have a target block!");

    auto it = cfg.m_constants.addresses.find(block);
    if (it != cfg.m_constants.addresses.end())
        return it->second;

    BlockAddress* addr = new BlockAddress(VoidType::get(cfg), block);
    cfg.m_constants.addresses.emplace(block, addr);
    return addr;
}

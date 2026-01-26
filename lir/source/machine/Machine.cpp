//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lir/machine/Machine.hpp"

#include <algorithm>
#include <unordered_set>

using namespace lir;

/// Aligns the given |offset| to the specified |align| value.
static uint32_t align_to(uint32_t offset, uint32_t align) {
    return (offset + align - 1) & ~(align - 1);
}

Machine::Machine(OS os) : m_os(os) {
    m_little_endian = true;

    m_pointer.size = 64;
    m_pointer.align = 64;
}

uint32_t Machine::get_size(const Type* type) const {
    switch (type->get_class()) {
        case lir::Type::Array: {
            const lir::ArrayType* array_type = 
                static_cast<const lir::ArrayType*>(type);
            return get_size(array_type->get_element_type()) * array_type->get_size();
        }

        case lir::Type::Float:
            return static_cast<const lir::FloatType*>(type)->get_width() / 8;

        case lir::Type::Integer:
            return std::max(static_cast<uint32_t>(1), 
                static_cast<const lir::IntegerType*>(type)->get_width() / 8);

        case lir::Type::Function:
        case lir::Type::Pointer:
            return get_pointer_size();

        case lir::Type::Struct: {
            const lir::StructType* struct_type = 
                static_cast<const lir::StructType*>(type);
            uint32_t offset = 0;
            for (lir::Type* field : struct_type->get_fields()) {
                offset = align_to(offset, get_align(field));
                offset += get_size(field);
            }

            return align_to(offset, get_align(type));
        }

        case lir::Type::Void:
            return 0;
    }
}

uint32_t Machine::get_align(const Type* type) const {
    switch (type->get_class()) {
        case lir::Type::Array:
            return get_align(
                static_cast<const lir::ArrayType*>(type)->get_element_type());

        case lir::Type::Float:
            return static_cast<const lir::FloatType*>(type)->get_width() / 8;

        case lir::Type::Integer:
            return static_cast<const lir::IntegerType*>(type)->get_width() / 8;

        case lir::Type::Function:
        case lir::Type::Pointer:
            return get_pointer_align();

        case lir::Type::Struct: {
            const lir::StructType* struct_type = static_cast<const lir::StructType*>(type);
            uint32_t max_align = 1;
            for (lir::Type* field : struct_type->get_fields())
                max_align = std::max(max_align, get_align(field));

            return max_align;
        }

        case lir::Type::Void:
            return 0;
    }
}

bool Machine::is_scalar(const Type* type) const {
    switch (type->get_class()) {
        case Type::Void:
        case Type::Integer:
        case Type::Float:
        case Type::Pointer:
            return true;
        default:
            return false;
    }
}

uint32_t Machine::get_element_offset(const ArrayType* type, uint32_t i) const {
    return get_size(type->get_element_type()) * i;
}

uint32_t Machine::get_pointee_offset(const PointerType* type, uint32_t i) const {
    return get_size(type->get_pointee()) * i;
}

uint32_t Machine::get_field_offset(const StructType* type, uint32_t i) const {
    uint32_t offset = 0;
    for (uint32_t j = 0; j < i; ++j) {
        const Type* field = type->get_field(j);
        uint32_t align = get_align(field);
        offset = align_to(offset, align) + get_size(field);
    }

    return align_to(offset, get_align(type->get_field(i)));
}

bool Machine::is_callee_saved(X64_Register reg) const {
    switch (m_os) {
    case Linux:
        static std::unordered_set<X64_Register> regs = { 
            RBX, R12, R13, R14, R15, RSP, RBP 
        };
        return regs.contains(reg);

    case Windows:
        assert(false && "windows ABI not implemented yet!");
    }
}

bool Machine::is_caller_saved(X64_Register reg) const {
    switch (m_os) {
    case Linux:
        static std::unordered_set<X64_Register> regs = {
            RAX, RCX, RDX, 
            RDI, RSI, 
            R8, R9, R10, R11, 
            R12, R13, R14, R15, 
            XMM0, XMM1, XMM2, XMM3, 
            XMM4, XMM5, XMM6, XMM7, 
            XMM8, XMM9, XMM10, XMM11, 
            XMM12, XMM13, XMM14, XMM15,
        };
        return regs.contains(reg);

    case Windows:
        assert(false && "windows ABI not implemented yet!");
    }
}

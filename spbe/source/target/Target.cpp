#include "graph/Type.hpp"
#include "target/Target.hpp"

using namespace spbe;

/// Aligns the given |offset| to the specified |align| value.
static uint32_t align_to(uint32_t offset, uint32_t align) {
    return (offset + align - 1) & ~(align - 1);
}

Target::Target(Arch arch, ABI abi, OS os) : m_arch(arch), m_abi(abi), m_os(os) {
    switch (arch) {
    case Arch::x64:
        m_little_endian = true;
        m_ptr_size = 64;
        m_ptr_align = 64;
        break;
    }

    m_rules[Type::TK_Int1] = { 8, 8 };
    m_rules[Type::TK_Int8] = { 8, 8 };
    m_rules[Type::TK_Int16] = { 16, 16 };
    m_rules[Type::TK_Int32] = { 32, 32 };
    m_rules[Type::TK_Int64] = { 64, 64 };
    m_rules[Type::TK_Float32] = { 32, 32 };
    m_rules[Type::TK_Float64] = { 64, 64 };
}

uint32_t Target::get_type_size(const Type* ty) const {
    switch (ty->get_kind()) {
    case Type::TK_Pointer:
        return get_pointer_size();

    case Type::TK_Array: {
        auto aty = static_cast<const ArrayType*>(ty);
        return get_type_size(aty->get_element_type()) * aty->get_size();
    }

    case Type::TK_Struct: {
        auto sty = static_cast<const StructType*>(ty);
        uint32_t offset = 0;
        for (const auto& field : sty->fields()) {
            offset = align_to(offset, get_type_align(field));
            offset += get_type_size(field);
        }

        return align_to(offset, get_type_align(ty));
    }

    default:
        return m_rules.at(ty->get_kind()).size_in_bits / 8;
    }
}

uint32_t Target::get_type_size_in_bits(const Type* ty) const {
    switch (ty->get_kind()) {
    case Type::TK_Pointer:
        return get_pointer_size_in_bits();

    case Type::TK_Array: {
        auto aty = static_cast<const ArrayType*>(ty);
        return get_type_size_in_bits(aty->get_element_type()) * aty->get_size();
    }

    case Type::TK_Struct: {
        auto sty = static_cast<const StructType*>(ty);
        uint32_t offset = 0;
        for (const auto& field : sty->fields()) {
            offset = align_to(offset, get_type_align(field));
            offset += get_type_size_in_bits(field);
        }

        return align_to(offset, get_type_align(ty));
    }

    default:
        return m_rules.at(ty->get_kind()).size_in_bits;
    }
}

uint32_t Target::get_type_align(const Type* ty) const {
    switch (ty->get_kind()) {
    case Type::TK_Pointer:
        return get_pointer_align();
        
    case Type::TK_Array: {
        auto aty = static_cast<const ArrayType*>(ty);
        return get_type_align(aty->get_element_type());
    }

    case Type::TK_Struct: {
        auto sty = static_cast<const StructType*>(ty);
        uint32_t max_align = 1;
        for (const auto& field : sty->fields())
            max_align = std::max(max_align, get_type_align(field));

        return max_align;
    }

    default:
        return m_rules.at(ty->get_kind()).abi_align / 8;
    }
}

uint32_t Target::get_type_align_in_bits(const Type* ty) const {
    switch (ty->get_kind()) {
    case Type::TK_Pointer:
        return get_pointer_align();

    case Type::TK_Array: {
        auto aty = static_cast<const ArrayType*>(ty);
        return get_type_align_in_bits(aty->get_element_type());
    }

    case Type::TK_Struct: {
        auto sty = static_cast<const StructType*>(ty);
        uint32_t max_align = 1;
        for (const auto& field : sty->fields())
            max_align = std::max(max_align, get_type_align_in_bits(field));

        return max_align;
    }

    default:
        return m_rules.at(ty->get_kind()).abi_align;
    }
}

bool Target::is_scalar_type(const Type* type) const {
    switch (type->get_kind()) {
    case Type::TK_Int1:
    case Type::TK_Int8:
    case Type::TK_Int16:
    case Type::TK_Int32:
    case Type::TK_Int64:
    case Type::TK_Float32:
    case Type::TK_Float64:
    case Type::TK_Pointer:
        return true;
    default:
        return false;
    }
}

uint32_t Target::get_element_offset(
        const ArrayType* type, uint32_t idx) const {
    return get_type_size(type->get_element_type()) * idx;
}

uint32_t Target::get_pointee_offset(
        const PointerType* type, uint32_t idx) const {
    return get_type_size(type->get_pointee()) * idx;
}

uint32_t Target::get_field_offset(const StructType* type, uint32_t idx) const {
    uint32_t offset = 0;
    for (uint32_t i = 0; i != idx; ++i) {
        const Type* field = type->get_field(i);
        uint32_t align = get_type_align(field);
        offset = align_to(offset, align) + get_type_size(field);
    }

    return align_to(offset, get_type_align(type->get_field(idx)));
}

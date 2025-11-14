#ifndef SPBE_TARGET_H_
#define SPBE_TARGET_H_

#include "graph/Type.hpp"

#include <cstdint>
#include <unordered_map>

namespace spbe {

/// A backend target, used for various code generation and type information.
class Target final {
public:
    /// Recognized CPU architectures.
    enum class Arch : uint8_t {
        x64,
    };

    /// Recognized application binary interfaces.
    enum class ABI : uint8_t {
        SystemV, Win32,
    };

    /// Recognized operating systems.
    enum class OS : uint8_t {
        Linux, Windows,
    };

private:
    /// The architecture of this target.
    const Arch m_arch;

    /// The ABI of this target.
    const ABI m_abi;

    /// The operating system of this target.
    const OS m_os;

    /// If true, this target architecture is little endian (LSB is at lowest
    /// address), and if false, this target is big endian.
    bool m_little_endian = true;

    /// The pointer size and alignment of this target in bits.
    uint32_t m_ptr_size = 64;
    uint32_t m_ptr_align = 64;
    
    /// A layout rule for a type. 
    struct LayoutRule final {
        uint32_t size_in_bits;
        uint32_t abi_align;
    };

    /// Default type layout rules for types that always get created, i.e.
    /// integer and floating point types.
    std::unordered_map<Type::Kind, LayoutRule> m_rules = {};
    
public:
    /// Create a new target.
    Target(Arch arch, ABI abi, OS os);

    /// Returns the architecture of this target.
    Arch arch() const { return m_arch; }

    /// Returns the ABI of this target.
    ABI abi() const { return m_abi; }

    /// Returns the operating system of this target.
    OS os() const { return m_os; }

    /// Returns the size of |ty| in bytes.
    uint32_t get_type_size(const Type* ty) const;

    /// Returns the size of |ty| in bits.
    uint32_t get_type_size_in_bits(const Type* ty) const;

    /// Returns the natural alignment in bytes for |ty|.
    uint32_t get_type_align(const Type* ty) const;

    /// Returns the natural alignment in bits for |ty|.
    uint32_t get_type_align_in_bits(const Type* ty) const;

    /// Returns the target pointer size in bytes.
    uint32_t get_pointer_size() const { return m_ptr_size / 8; }

    /// Returns the target pointer size in bits.
    uint32_t get_pointer_size_in_bits() const { return m_ptr_size; }

    /// Returns the target natural pointer alignment in bytes.
    uint32_t get_pointer_align() const { return m_ptr_align / 8; }

    /// Returns the target natural pointer alignment in bits.
    uint32_t get_pointer_align_in_bits() const { return m_ptr_align; }

    /// Returns true if this target is little-endian.
    bool is_little_endian() const { return m_little_endian; }
    
    /// Returns true if this target is big-endian.
    bool is_big_endian() const { return !m_little_endian; }

    /// Returns true if |type| is a scalar type, that is, not an aggregate
    /// of values or a complex type. Pointers are considered scalar.
    bool is_scalar_type(const Type* type) const;

    /// Returns the array element offset for |type| at the index |idx|.
    uint32_t get_element_offset(const ArrayType* type, uint32_t idx) const;

    /// Returns the pointer pointee offset for |type| at the index |idx|.
    uint32_t get_pointee_offset(const PointerType* type, uint32_t idx) const;

    /// Returns the offset of a structure field of |type| at the index |idx|.
    uint32_t get_field_offset(const StructType* type, uint32_t idx) const;
};

} // namespace spbe

#endif // SPBE_TARGET_H_

//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_MACHINE_H_
#define LOVELACE_IR_MACHINE_H_

#include "lir/graph/Type.hpp"
#include "lir/machine/Register.hpp"

namespace lir {

class Machine final {
public:
    enum OS : uint8_t {
        Linux, Windows,
    };

private:
    OS m_os;

    bool m_little_endian;

    /// Machine pointer size & alignment, in bits.
    struct {
        uint8_t size;
        uint8_t align;
    } m_pointer;

public:
    Machine(OS os);

    inline bool is_little_endian() const { return m_little_endian; }
    inline bool is_big_endian() const { return !m_little_endian; }

    /// Returns the size of pointers, in bytes.
    uint32_t get_pointer_size() const { return m_pointer.size / 8; }

    /// Returns the natural alignment of pointers, in bytes.
    uint32_t get_pointer_align() const { return m_pointer.align / 8; }

    /// Returns the size of the given |type|, in bytes.
    uint32_t get_size(const Type* type) const;

    /// Returns the natural alignment of the given |type|, in bytes.
    uint32_t get_align(const Type* type) const;

    /// Test if the given |type| is considered scalar for the target.
    bool is_scalar(const Type* type) const;

    /// Returns the byte offset for the |i|-th element under |type|.
    uint32_t get_element_offset(const ArrayType* type, uint32_t i) const;

    /// Returns the byte offset for the |i|-th pointee under |type|.
    uint32_t get_pointee_offset(const PointerType* type, uint32_t i) const;

    /// Returns the byte offset for the |i|-th field in the given structure
    /// |type|.
    uint32_t get_field_offset(const StructType* type, uint32_t i) const;

    /// Test if the given |reg| is a callee-saved register around callsites
    /// per the OS ABI of this machine.
    bool is_callee_saved(X64_Register reg) const;

    /// Test if the given |reg| is a caller-saved register around callsites
    /// per the OS ABI of this machine.
    bool is_caller_saved(X64_Register reg) const;
};

} // namespace lir

#endif // LOVELACE_IR_MACHINE_H_

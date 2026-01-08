//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_REGISTER_H_
#define LOVELACE_IR_REGISTER_H_

//
//  This header file declares a list of recognized X64 registers per the
//  X64_Register enum, as well as the Register type, which is used to represent
//  both virtual and physical registers in the X64 machine IR.
//

#include <cstdint>

namespace lir {

/// Potential physical register classes.
///
/// Used in tandem with virtual registers for register allocation.
enum RegisterClass : uint8_t {
    GeneralPurpose, FloatingPoint,
};

/// The different X64 registers.
enum X64_Register : uint32_t {
    None = 0,
    RAX, 
    RBX, 
    RCX, 
    RDX,
    RDI, 
    RSI,
    R8, 
    R9, 
    R10, 
    R11, 
    R12, 
    R13, 
    R14, 
    R15,
    RSP, 
    RBP,
    RIP,
    XMM0, 
    XMM1, 
    XMM2, 
    XMM3,
    XMM4, 
    XMM5, 
    XMM6, 
    XMM7,
    XMM8, 
    XMM9, 
    XMM10, 
    XMM11,
    XMM12, 
    XMM13, 
    XMM14, 
    XMM15,
};

/// Returns the X64 register class for the given |reg|.
RegisterClass get_register_class(X64_Register reg);

/// Represents a virtual or physical register.
class Register final {
public:
    static constexpr uint32_t NoRegister = 0u;
    static constexpr uint32_t PhysicalBarrier = 1u;
    static constexpr uint32_t VirtualBarrier = 1u << 31;

private:
    /// The number of this register.
    ///
    /// All registers share a global namespace, and the value about barriers
    /// determine if the register is physical or virtual.
    ///
    /// 0               Non-register
    /// [1, 2^31)       Physical registers
    /// [2^31, 2^32)    Virtual registers
    uint32_t m_reg;

public:
    Register() = default;
    Register(uint32_t reg) : m_reg(reg) {}

    /// Returns true if this register is valid.
    bool is_valid() const { return m_reg != NoRegister; }

    /// Returns true if this register is physical.
    bool is_physical() const { 
        return PhysicalBarrier <= m_reg && m_reg < VirtualBarrier; 
    }

    /// Returns true if this register is virtual.
    bool is_virtual() const { return m_reg >= VirtualBarrier; }

    uint32_t id() const { return m_reg; }

    bool operator==(const Register& other) const {
        return m_reg == other.m_reg;
    }

    bool operator==(uint32_t other) const { return m_reg == other; }

    static bool is_valid(uint32_t reg) { return reg != NoRegister; }

    static bool is_physical(uint32_t reg) {
        return PhysicalBarrier <= reg && reg < VirtualBarrier;
    }

    static bool is_virtual(uint32_t reg) { return reg >= VirtualBarrier; }
};

} // namespace lir

#endif // LOVELACE_IR_X64_REGISTER_H_

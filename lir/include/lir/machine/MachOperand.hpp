//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_MACH_OPERAND_H_
#define LOVELACE_IR_MACH_OPERAND_H_

//
//  This header file declares the MachOperand class, which is used to represent
//  operands to X64 instructions.
//

#include "lir/machine/Register.hpp"

#include <cassert>
#include <cstdint>

namespace lir {

class MachLabel;

class MachOperand final {
public:
    /// The different kinds of operands.
    enum Kind : uint16_t {
        Reg,         ///< Register refernce, physical or virtual.
        Memory,      ///< Memory references on a base register.
        Stack,       ///< Index of an object on a function stack.
        Immediate,   ///< Immediate, less than 64-bits.
        Label,       ///< Reference to a block label.
        Constant,    ///< Index of a pooled function constant. 
        Symbol,      ///< Reference to named symbol.
    };

private:
    /// The kind of operand this is, discriminating the union.
    Kind m_kind : 4;

    /// subreg - optional subregister for register operands. 0 indicates
    /// no subregister, and subregisters are measured in bytes, e.g. 8 for %rax
    /// and 1 for %al.
    uint16_t m_subreg : 9;
    
    /// is_def - true if this register operand is a def, false if it is a 
    /// use.
    uint16_t m_is_def : 1;

    /// is_kill_or_dead - true if a. this operand is a use and is the last 
    /// use of a register or b. this operand is a def and is never used by a
    /// following instruction.
    uint16_t is_kill_or_dead : 1;

    /// is_implicit - true if this register operand is an implicit def or 
    /// use, false if it is explicit.
    uint16_t m_is_implicit : 1;

    union {
        Register m_reg;

        struct {
            Register reg;
            int32_t disp;
        } m_mem;

        uint32_t m_stack;

        int64_t m_imm;

        MachLabel* m_label;

        uint32_t m_const;

        const char* m_symbol;
    };

public:
    static MachOperand create_reg(Register reg, uint16_t subreg, 
                                  bool is_def, bool is_implicit = false, 
                                  bool is_kill = false, bool is_dead = false);

    static MachOperand create_mem(Register reg, int32_t disp);

    static MachOperand create_stack_ref(uint32_t index);

    static MachOperand create_imm(int64_t imm);

    static MachOperand create_label(MachLabel* label);

    static MachOperand create_constant_ref(uint32_t index);

    static MachOperand create_symbol(const char* symbol);

    Kind kind() const { return m_kind; }

    bool is_reg() const { return kind() == Reg; }
    bool is_mem() const { return kind() == Memory; }
    bool is_stack() const { return kind() == Stack; }
    bool is_imm() const { return kind() == Immediate; }
    bool is_label() const { return kind() == Label; }
    bool is_constant() const { return kind() == Constant; }
    bool is_symbol() const { return kind() == Symbol; }

    Register get_reg() const {
        assert(is_reg());
        return m_reg;
    }

    uint16_t get_subreg() const {
        assert(is_reg());
        return m_subreg;
    }

    bool is_def() const { 
        assert(is_reg());
        return m_is_def; 
    }

    bool is_explicit_def() const {
        assert(is_reg());
        return m_is_def && !m_is_implicit;
    }

    bool is_implicit_def() const {
        assert(is_reg());
        return m_is_def && m_is_implicit;
    }

    bool is_use() const {
        assert(is_reg());
        return !m_is_def; 
    }

    bool is_explicit_use() const {
        assert(is_reg());
        return !m_is_def && !m_is_implicit;
    }

    bool is_implicit_use() const {
        assert(is_reg());
        return !m_is_def && m_is_implicit;
    }

    bool is_kill() const {
        assert(is_reg());
        return is_kill_or_dead & !m_is_def;
    }

    bool is_dead() const {
        assert(is_reg());
        return is_kill_or_dead && m_is_def;
    }

    bool is_implicit() const {
        assert(is_reg());
        return m_is_implicit;
    }

    Register get_mem_base() const {
        assert(is_mem());
        return m_mem.reg;
    }

    uint32_t get_mem_disp() const {
        assert(is_mem());
        return m_mem.disp;
    }

    uint32_t get_stack() const {
        assert(is_stack());
        return m_stack;
    }

    int64_t get_imm() const {
        assert(is_imm());
        return m_imm;
    }

    MachLabel* get_label() const {
        assert(is_label());
        return m_label;
    }

    uint32_t get_constant() const {
        assert(is_constant());
        return m_const;
    }
    
    const char* get_symbol() const {
        assert(is_symbol());
        return m_symbol;
    }

    void set_reg(Register reg) {
        assert(is_reg());
        m_reg = reg;
    }

    void set_subreg(uint16_t subreg) {
        assert(is_reg());
        m_subreg = subreg;
    }

    void set_is_def(bool value = true) {
        assert(is_reg());
        m_is_def = value;
    }

    void set_is_use(bool value = true) {
        assert(is_reg());
        m_is_def = !value;
    }

    void set_is_kill(bool value = true) {
        assert(is_reg());
        assert(is_use());
        is_kill_or_dead = value;
    }

    void set_is_dead(bool value = true) {
        assert(is_reg());
        assert(is_def());
        is_kill_or_dead = value;
    }

    void set_is_implicit(bool value = true) {
        assert(is_reg());
        m_is_implicit = value;
    }

    void set_mem_base(Register reg) {
        assert(is_mem());
        m_mem.reg = reg;
    }

    void set_mem_disp(int32_t disp) {
        assert(is_mem());
        m_mem.disp = disp;
    }

    void set_stack(uint32_t index) {
        assert(is_stack());
        m_stack = index;
    }

    void set_imm(int64_t imm) {
        assert(is_imm());
        m_imm = imm;
    }

    void set_label(MachLabel* label) {
        assert(is_label());
        m_label = label;
    }

    void set_constant(uint32_t index) {
        assert(is_constant());
        m_const = index;
    }

    void set_symbol(const char* symbol) {
        assert(is_symbol());
        m_symbol = symbol;
    }
};

} // namespace lir

#endif // LOVELACE_IR_MACH_OPERAND_H_

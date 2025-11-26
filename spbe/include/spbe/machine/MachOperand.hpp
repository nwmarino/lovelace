//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SPBE_MACH_OPERAND_H_
#define SPBE_MACH_OPERAND_H_

#include "spbe/machine/MachRegister.hpp"

#include <cassert>
#include <cstdint>

namespace spbe {

class MachBasicBlock;

/// Represents a target-dependent operand to a machine instruction.
class MachOperand final {
public:
    /// Different kinds of machine operands.
    enum MachOperandKind : uint16_t {
        MO_Register,    ///< Register, physical or virtual.
        MO_Memory,      ///< Memory references on a base register.
        MO_StackIdx,    ///< Function stack reference.
        MO_Immediate,   ///< Immediate, less than 64-bits.
        MO_BasicBlock,  ///< Reference to a basic block.
        MO_ConstantIdx, ///< Index of a function constant. 
        MO_Symbol,      ///< Reference to named symbol.
    };

private:
    /// Defines the kind of operand this is, discriminating the union.
    MachOperandKind m_kind : 4;

    /// subreg - optional subregister for register operands. 0 indicates
    /// no subregister.
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
        /// For MO_Register operands.
        MachRegister m_reg;

        /// For MO_Memory operands.
        struct {
            MachRegister reg;
            int32_t disp;
        } m_mem;

        /// For MO_StackIdx operands.
        uint32_t m_stack_idx;

        /// For MO_Immediate operands.
        int64_t m_imm;

        /// For MO_BasicBlock operands.
        MachBasicBlock* m_mbb;

        /// For MO_ConstantIdx operands.
        uint32_t m_constant_idx;

        /// For MO_Symbol operands.
        const char* m_symbol;
    };

public:
    static MachOperand create_reg(MachRegister reg, uint16_t subreg, 
                                  bool is_def, bool is_implicit = false, 
                                  bool is_kill = false, bool is_dead = false);

    static MachOperand create_mem(MachRegister reg, int32_t disp);

    static MachOperand create_stack_index(uint32_t idx);

    static MachOperand create_imm(int64_t imm);

    static MachOperand create_block(MachBasicBlock* mbb);

    static MachOperand create_constant_index(uint32_t idx);

    static MachOperand create_symbol(const char* symbol);

    MachOperandKind kind() const { return m_kind; }

    bool is_reg() const { return kind() == MO_Register; }
    bool is_mem() const { return kind() == MO_Memory; }
    bool is_stack_index() const { return kind() == MO_StackIdx; }
    bool is_imm() const { return kind() == MO_Immediate; }
    bool is_mmb() const { return kind() == MO_BasicBlock; }
    bool is_constant_index() const { return kind() == MO_ConstantIdx; }
    bool is_symbol() const { return kind() == MO_Symbol; }

    MachRegister get_reg() const {
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

    MachRegister get_mem_base() const {
        assert(is_mem());
        return m_mem.reg;
    }

    uint32_t get_mem_disp() const {
        assert(is_mem());
        return m_mem.disp;
    }

    uint32_t get_stack_index() const {
        assert(is_stack_index());
        return m_stack_idx;
    }

    int64_t get_imm() const {
        assert(is_imm());
        return m_imm;
    }

    MachBasicBlock* get_mmb() const {
        assert(is_mmb());
        return m_mbb;
    }

    uint32_t get_constant_index() const {
        assert(is_constant_index());
        return m_constant_idx;
    }
    
    const char* get_symbol() const {
        assert(is_symbol());
        return m_symbol;
    }

    void set_reg(MachRegister reg) {
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

    void set_mem_base(MachRegister reg) {
        assert(is_mem());
        m_mem.reg = reg;
    }

    void set_mem_disp(int32_t disp) {
        assert(is_mem());
        m_mem.disp = disp;
    }

    void set_stack_index(uint32_t idx) {
        assert(is_stack_index());
        m_stack_idx = idx;
    }

    void set_imm(int64_t imm) {
        assert(is_imm());
        m_imm = imm;
    }

    void set_mbb(MachBasicBlock* mbb) {
        assert(is_mmb());
        m_mbb = mbb;
    }

    void set_constant_index(uint32_t idx) {
        assert(is_constant_index());
        m_constant_idx = idx;
    }

    void set_symbol(const char* symbol) {
        assert(is_symbol());
        m_symbol = symbol;
    }
};

} // namespace spbe

#endif // SPBE_MACH_OPERAND_H_

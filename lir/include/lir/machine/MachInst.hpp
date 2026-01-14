//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_MACH_INST_H_
#define LOVELACE_IR_MACH_INST_H_

//
//  This header file declares the MachInst class used to represent X64
//  instructions, as well as the set of recognized X64 mnemonics per the 
//  X64_Mnemonic enumeration.
//

#include "lir/machine/Register.hpp"
#include "lir/machine/MachOperand.hpp"

#include <cassert>
#include <ranges>
#include <vector>

namespace lir {

class MachFunction;
class MachLabel;

/// The different recognized X64 mnemonics.
enum class X64_Mnemonic : uint32_t {
    None = 0,
    NOP, 
    JMP, 
    UD2, 
    CQO, 
    SYSCALL,
    CALL, 
    RET, 
    LEA, 
    PUSH, 
    POP,
    MOV,
    ADD,
    SUB,
    MUL,
    IMUL,
    DIV,
    IDIV,
    AND,
    OR,
    XOR,
    SHL,
    SHR,
    SAR,
    CMP,
    NOT,
    NEG,
    MOVABS,
    MOVSX,
    MOVSXD,
    MOVZX,
    JE, JNE, JZ, JNZ,
    JL, JLE, JG, JGE,
    JA, JAE, JB, JBE,
    SETE, SETNE, SETZ, SETNZ,
    SETL, SETLE, SETG, SETGE,
    SETA, SETAE, SETB, SETBE,
    MOVS,
    MOVAP,
    UCOMIS,
    ADDS,
    SUBS,
    MULS,
    DIVS,
    ANDP,
    ORP,
    XORP,
    CVTSS2SD, CVTSD2SS,
    CVTSI2SS, CVTSI2SD,
    CVTTSS2SI,
    CVTTSD2SI,
    // VCVCTSS2USI for unsigned
    
};

enum class X64_Size : uint32_t {
    None,
    Byte,
    Word,
    Long,
    Quad,
    Single,
    Double,
};

/// Represents an machine instruction.
class MachInst final {
public:
    using Operands = std::vector<MachOperand>;

private:
    X64_Mnemonic m_op;
    X64_Size m_size;
    MachLabel* m_parent;
    Operands m_ops;

public:
    explicit MachInst(X64_Mnemonic op, X64_Size size = X64_Size::None, 
                      const Operands& ops = {}, MachLabel* parent = nullptr);

    X64_Mnemonic op() const { return m_op; }
    X64_Size size() const { return m_size; }

    void set_parent(MachLabel* parent) { m_parent = parent; }
    void clear_parent() { m_parent = nullptr; }
    const MachLabel* get_parent() const { return m_parent; }
    MachLabel* get_parent() { return m_parent; }

    const MachFunction* get_function() const;
    MachFunction* get_function() {
        return const_cast<MachFunction*>(
            static_cast<const MachInst*>(this)->get_function());
    }

    const Operands& get_operands() const { return m_ops; }
    Operands& get_operands() { return m_ops; }

    const MachOperand& get_operand(uint32_t idx) const {
        assert(idx < num_operands() && "index out of bounds!");
        return m_ops[idx];
    }

    MachOperand& get_operand(uint32_t idx) { 
        assert(idx < num_operands() && "index out of bounds!");
        return m_ops[idx];
    }

    uint32_t num_operands() const { return m_ops.size(); }
    bool has_operands() const { return !m_ops.empty(); }

    /// Returns the number of defining operands part of this instruction.
    uint32_t num_defs() const;

    /// Returns the number of implicit operands part of this instruction.
    uint32_t num_implicit_operands() const;

    /// Returns the number of explicit operands part of this instruction.
    uint32_t num_explicit_operands() const;

    /// Returns the number of implicitly defining operands that are part of
    /// this instruction.
    uint32_t num_implicit_defs() const;

    /// Returns the number of explicitly defining operands that are part of
    /// this instruction.
    uint32_t num_explicit_defs() const;

    /// Returns true if any of this instruction's operands are implicit defs.
    bool has_implicit_def() const;

    /// Returns a list of all explicit def operands.
    const auto defs() const {
        return m_ops | std::views::filter([](const MachOperand& mo) { 
            return mo.is_reg() && mo.is_def() && !mo.is_implicit(); 
        });
    }

    auto defs() {
        return m_ops | std::views::filter([](MachOperand& mo) { 
            return mo.is_reg() && mo.is_def() && !mo.is_implicit(); 
        });
    }

    /// Returns a list of all explicit use operands.
    const auto uses() const {
        return m_ops | std::views::filter([](const MachOperand& mo) { 
            return mo.is_reg() && mo.is_use() && !mo.is_implicit(); 
        });
    }

    auto uses() {
        return m_ops | std::views::filter([](MachOperand& mo) { 
            return mo.is_reg() && mo.is_use() && !mo.is_implicit(); 
        });
    }

    /// Returns a list of all explicit & implicit def operands.
    const auto all_defs() const {
        return m_ops | std::views::filter([](const MachOperand& mo) {
            return mo.is_reg() && mo.is_def();
        });
    }

    auto all_defs() {
        return m_ops | std::views::filter([](MachOperand& mo) {
            return mo.is_reg() && mo.is_def();
        });
    }

    /// Returns a list of all explicit & implicit use operands.
    const auto all_uses() const {
        return m_ops | std::views::filter([](const MachOperand& mo) {
            return mo.is_reg() && mo.is_use();
        });
    }

    auto all_uses() {
        return m_ops | std::views::filter([](MachOperand& mo) {
            return mo.is_reg() && mo.is_use();
        });
    }

    /// Add a new operand |op| to this instruction.
    MachInst& add_operand(const MachOperand& op) {
        m_ops.push_back(op);
        return *this;
    }

    MachInst& add_reg(Register reg, uint16_t subreg, bool is_def, 
                             bool is_implicit = false, bool is_kill = false, 
                             bool is_dead = false) {
        m_ops.push_back(MachOperand::create_reg(
            reg, subreg, is_def, is_implicit, is_kill, is_dead));
        return *this;
    }

    MachInst& add_mem(Register reg, int32_t disp) {
        m_ops.push_back(MachOperand::create_mem(reg, disp));
        return *this;
    }

    MachInst& add_stack(uint32_t index) {
        m_ops.push_back(MachOperand::create_stack_ref(index));
        return *this;
    }

    MachInst& add_imm(int64_t imm) {
        m_ops.push_back(MachOperand::create_imm(imm));
        return *this;
    }

    MachInst& add_zero() {
        m_ops.push_back(MachOperand::create_imm(0));
        return *this;
    }

    MachInst& add_label(MachLabel* label) {
        m_ops.push_back(MachOperand::create_label(label));
        return *this;
    }

    MachInst& add_constant(uint32_t index) {
        m_ops.push_back(MachOperand::create_constant_ref(index));
        return *this;
    }

    MachInst& add_symbol(const char* symbol) {
        m_ops.push_back(MachOperand::create_symbol(symbol));
        return *this;
    }

    MachInst& add_symbol(const std::string& symbol) {
        m_ops.push_back(MachOperand::create_symbol(symbol.c_str()));
        return *this;
    }
};

} // namespace lir

#endif // LOVELACE_IR_MACH_INST_H_

//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SPBE_MACH_INSTRUCTION_H_
#define SPBE_MACH_INSTRUCTION_H_

#include "spbe/machine/MachOperand.hpp"
#include "spbe/machine/MachRegister.hpp"

#include <cassert>
#include <ranges>
#include <vector>

namespace spbe {

class MachFunction;

/// Represents a target-dependent machine instruction.
class MachInstruction final {
    /// The opcode of this instruction.
    uint32_t m_opcode;

    /// The parent basic block of this instruction, if it exists.
    MachBasicBlock* m_parent;
    
    /// The operands of this instruction.
    ///
    /// TODO: Pack this into a MachOperand* array, and optimize allocations
    /// to reduce on instruction size.
    std::vector<MachOperand> m_operands;

public:
    MachInstruction(uint32_t opcode, 
                    const std::vector<MachOperand>& operands = {}, 
                    MachBasicBlock* parent = nullptr);

    /// Returns the opcode of this machine instruction.
    uint32_t opcode() const { return m_opcode; }

    /// Returns the parent basic block of this machine instruction, if there
    /// is a parent.
    const MachBasicBlock* get_parent() const { return m_parent; }
    MachBasicBlock* get_parent() { return m_parent; }

    /// Clear the parent link of this machine instruction. Does not detach this
    /// instruction from any existing parent block.
    void clear_parent() { m_parent = nullptr; }

    /// Set the parent basic block of this instruction to |mbb|.
    void set_parent(MachBasicBlock* mbb) { m_parent = mbb; }

    /// Returns the parent function of this machine instruction, if there is
    /// a parent.
    const MachFunction* get_mf() const;
    MachFunction* get_mf() {
        return const_cast<MachFunction*>(
            static_cast<const MachInstruction*>(this)->get_mf());
    }

    /// Returns the raw number of operands included in this instruction.
    uint32_t num_operands() const { return m_operands.size(); }

    const std::vector<MachOperand>& operands() const { return m_operands; }
    std::vector<MachOperand>& operands() { return m_operands; }

    /// Returns the machine operand at position |idx| of this instruction.
    const MachOperand& get_operand(uint32_t idx) const {
        assert(idx <= num_operands() && "index out of bounds!");
        return m_operands[idx];
    }

    MachOperand& get_operand(uint32_t idx) { 
        assert(idx <= num_operands() && "index out of bounds!");
        return m_operands[idx];
    }

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
        return m_operands | std::views::filter([](const MachOperand& mo) { 
            return mo.is_reg() && mo.is_def() && !mo.is_implicit(); 
        });
    }

    auto defs() {
        return m_operands | std::views::filter([](MachOperand& mo) { 
            return mo.is_reg() && mo.is_def() && !mo.is_implicit(); 
        });
    }

    /// Returns a list of all explicit use operands.
    const auto uses() const {
        return m_operands | std::views::filter([](const MachOperand& mo) { 
            return mo.is_reg() && mo.is_use() && !mo.is_implicit(); 
        });
    }

    auto uses() {
        return m_operands | std::views::filter([](MachOperand& mo) { 
            return mo.is_reg() && mo.is_use() && !mo.is_implicit(); 
        });
    }

    /// Returns a list of all explicit & implicit def operands.
    const auto all_defs() const {
        return m_operands | std::views::filter([](const MachOperand& mo) {
            return mo.is_reg() && mo.is_def();
        });
    }

    auto all_defs() {
        return m_operands | std::views::filter([](MachOperand& mo) {
            return mo.is_reg() && mo.is_def();
        });
    }

    /// Returns a list of all explicit & implicit use operands.
    const auto all_uses() const {
        return m_operands | std::views::filter([](const MachOperand& mo) {
            return mo.is_reg() && mo.is_use();
        });
    }

    auto all_uses() {
        return m_operands | std::views::filter([](MachOperand& mo) {
            return mo.is_reg() && mo.is_use();
        });
    }

    /// Add a new operand |op| to this instruction.
    MachInstruction& add_operand(const MachOperand& op) {
        m_operands.push_back(op);
        return *this;
    }

    MachInstruction& add_reg(MachRegister reg, uint16_t subreg, bool is_def, 
                             bool is_implicit = false, bool is_kill = false, 
                             bool is_dead = false) {
        m_operands.push_back(MachOperand::create_reg(
            reg, subreg, is_def, is_implicit, is_kill, is_dead));
        return *this;
    }

    MachInstruction& add_mem(MachRegister reg, int32_t disp) {
        m_operands.push_back(MachOperand::create_mem(reg, disp));
        return *this;
    }

    MachInstruction& add_stack_index(uint32_t idx) {
        m_operands.push_back(MachOperand::create_stack_index(idx));
        return *this;
    }

    MachInstruction& add_imm(int64_t imm) {
        m_operands.push_back(MachOperand::create_imm(imm));
        return *this;
    }

    MachInstruction& add_zero() {
        m_operands.push_back(MachOperand::create_imm(0));
        return *this;
    }

    MachInstruction& add_block(MachBasicBlock* mbb) {
        m_operands.push_back(MachOperand::create_block(mbb));
        return *this;
    }

    MachInstruction& add_constant_index(uint32_t idx) {
        m_operands.push_back(MachOperand::create_constant_index(idx));
        return *this;
    }

    MachInstruction& add_symbol(const char* symbol) {
        m_operands.push_back(MachOperand::create_symbol(symbol));
        return *this;
    }

    MachInstruction& add_symbol(const std::string& symbol) {
        m_operands.push_back(MachOperand::create_symbol(symbol.c_str()));
        return *this;
    }
};

} // namespace spbe

#endif // SPBE_MACH_INSTRUCTION_H_

//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SPBE_X64_INSTR_SELECTOR_H_
#define SPBE_X64_INSTR_SELECTOR_H_

#include "spbe/X64/X64.hpp"
#include "spbe/graph/Instruction.hpp"
#include "spbe/machine/InstrSelector.hpp"
#include "spbe/machine/MachFunction.hpp"
#include "spbe/machine/MachBasicBlock.hpp"
#include "spbe/machine/MachInstruction.hpp"
#include "spbe/machine/MachOperand.hpp"
#include "spbe/machine/MachRegister.hpp"

#include <unordered_map>
#include <vector>

namespace spbe::x64 {

class X64InstrSelector final : public spbe::InstrSelector {
    using RegisterTable = std::unordered_map<uint32_t, MachRegister>;
    using LocalTable = std::unordered_map<const Local*, uint32_t>;

    const Target& m_target;
    MachBasicBlock* m_insert = nullptr;

    RegisterTable m_regs = {};
    LocalTable m_locals = {};
    std::vector<const Instruction*> m_deferred = {};

    /// Reinterpet the defining instruction |inst| as a new or existing
    /// virtual machine register.
    MachRegister as_register(const Instruction* inst);

    /// Returns a temporary machine register for scratch use with the register
    /// class |cls|.
    MachRegister get_temporary(RegisterClass cls);

    /// Returns the expected X64 general-purpose subregister for a given type.
    /// This function always returns 1, 2, 4, or 8.
    uint16_t get_subregister(const Type* type) const;

    /// Returns a lowered machine operand of |value|.
    MachOperand as_operand(const Value* value);

    /// Returns a machine operand representing the destination of a call
    /// argument as per the ABI defined in the parent function target.
    MachOperand as_argument(const Value* value, uint32_t index) const;

    /// Returns the X64 conditional jump mnemonic equivelant of |op|.
    x64::Opcode get_jcc_op(spbe::Opcode op) const;

    /// Returns the X64 conditional set mnemonic equivelant of |op|.
    x64::Opcode get_setcc_op(spbe::Opcode op) const;

    /// Returns the X64 move opcode with suffix based on the size of |type|.
    x64::Opcode get_move_op(const Type* type) const;

    /// Returns the X64 comparison opcode with suffix based on the size of 
    /// |type|.
    x64::Opcode get_cmp_op(const Type* type) const;

    /// Returns the X64 add opcode with suffix based on the size of |type|.
    x64::Opcode get_add_op(const Type* type) const;

    /// Returns the X64 sub opcode with suffix based on the size of |type|.
    x64::Opcode get_sub_op(const Type* type) const;

    /// Returns the X64 imul opcode with suffix based on the size of |type|.
    x64::Opcode get_imul_op(const Type* type) const;

    /// Returns the X64 mul opcode with suffix based on the size of |type|.
    x64::Opcode get_mul_op(const Type* type) const;

    /// Returns the X64 idiv opcode with suffix based on the size of |type|.
    x64::Opcode get_idiv_op(const Type* type) const;

    /// Returns the X64 div opcode with suffix based on the size of |type|.
    x64::Opcode get_div_op(const Type* type) const;

    /// Returns the X64 and opcode with suffix based on the size of |type|.
    x64::Opcode get_and_op(const Type* type) const;

    /// Returns the X64 or opcode with suffix based on the size of |type|.
    x64::Opcode get_or_op(const Type* type) const;

    /// Returns the X64 xor opcode with suffix based on the size of |type|.
    x64::Opcode get_xor_op(const Type* type) const;

    /// Returns the X64 shl opcode with suffix based on the size of |type|.
    x64::Opcode get_shl_op(const Type* type) const;

    /// Returns the X64 shr opcode with suffix based on the size of |type|.
    x64::Opcode get_shr_op(const Type* type) const;

    /// Returns the X64 sar opcode with suffix based on the size of |type|.
    x64::Opcode get_sar_op(const Type* type) const;

    /// Returns the X64 not opcode with suffix based on the size of |type|.
    x64::Opcode get_not_op(const Type* type) const;

    /// Returns the X64 neg opcode with suffix based on the size of |type|.
    x64::Opcode get_neg_op(const Type* type) const;

    /// Flip the conditional jump opcode |jcc| operand-wise. This is not the
    /// same as negating it.
    x64::Opcode flip_jcc(x64::Opcode jcc) const;

    /// Flip the conditional set opcode |setcc| operand-wise. This is not the
    /// same as negating it.
    x64::Opcode flip_setcc(x64::Opcode setcc) const;

    /// Negate the conditional jump opcode |jcc|, retaining signedness.
    x64::Opcode negate_jcc(x64::Opcode jcc) const;

    /// Negate the conditional set opcode |setcc|, retaining signedness.
    x64::Opcode negate_setcc(x64::Opcode setcc) const;

    /// Emit a new machine instruction with opcode |op| and operand list |ops|.
    ///
    /// If |before_terminators| is true, then the new instruction will get
    /// inserted into the active block before any terminating instructions.
    ///
    /// If an active insertion block is not set, then the instruction will not
    /// be inserted anywhere.
    MachInstruction& emit(x64::Opcode op, 
                          const std::vector<MachOperand>& ops = {}, 
                          bool before_terminators = false);

    /// Returns true if the comparison |inst| has been deferred.
    bool is_deferred(const Instruction* inst) const;

    /// Defer the comparison |inst|. This is mainly used when the sole user
    /// of a comparison instruction is a branch, and so by deferring the 
    /// comparison, we can skip out on conditional sets and branch based on
    /// register flags.
    void defer(const Instruction* inst);

    void select_constant(const Instruction* inst) override;

    void select_string_constant(const Instruction* inst) override;

    void select_load_store(const Instruction* inst) override;

    void select_access_ptr(const Instruction* inst) override;

    void select_select(const Instruction* inst) override;

    void select_branch_if(const Instruction* inst) override;

    void select_phi(const Instruction* inst) override;

    void select_return(const Instruction* inst) override;

    void select_call(const Instruction* inst) override;

    void select_add(const Instruction* inst) override;

    void select_sub(const Instruction* inst) override;

    void select_imul(const Instruction* inst) override;

    void select_idiv_irem(const Instruction* inst) override;

    void select_fmul_fdiv(const Instruction* inst) override;

    void select_bit_op(const Instruction* inst) override;

    void select_shift(const Instruction* inst) override;

    void select_not(const Instruction* inst) override;

    void select_neg(const Instruction* inst) override;

    void select_ext(const Instruction* inst) override;

    void select_trunc(const Instruction* inst) override;

    void select_int_to_fp_cvt(const Instruction* inst) override;

    void select_fp_to_int_cvt(const Instruction* inst) override;

    void select_ptr_to_int_cvt(const Instruction* inst) override;

    void select_int_to_ptr_cvt(const Instruction* inst) override;

    void select_type_reinterpret(const Instruction* inst) override;

    void select_comparison(const Instruction* inst) override;

public:
    X64InstrSelector(MachFunction& MF)
        : spbe::InstrSelector(MF), m_target(MF.get_target()) {}

    X64InstrSelector(const X64InstrSelector&) = delete;
    X64InstrSelector& operator = (const X64InstrSelector&) = delete;

    ~X64InstrSelector() = default;

    void run() override;
};

} // namespace spbe::x64

#endif // SPBE_X64_INSTR_SELECTOR_H_

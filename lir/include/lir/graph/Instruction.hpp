//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_INSTRUCTION_H_
#define LOVELACE_IR_INSTRUCTION_H_

#include "lir/graph/Constant.hpp"
#include "lir/graph/User.hpp"
#include "lir/graph/Value.hpp"

#include <cstdint>

namespace lir {

class BasicBlock;

/// The different mnemonics for instructions in the IR.
enum Mnemonic : uint16_t {
    OP_ABORT,
    OP_ACCESS,
    OP_AND,
    OP_AP,
    OP_CALL,
    OP_CMP,
    OP_F2S,
    OP_F2U,
    OP_FADD,
    OP_FDIV,
    OP_FEXT,
    OP_FMUL,
    OP_FNEG,
    OP_FSUB,
    OP_FTRUNC,
    OP_I2P,
    OP_IADD,
    OP_IMUL,
    OP_INEG,
    OP_ISUB,
    OP_ITRUNC,
    OP_JIF,
    OP_JMP,
    OP_LOAD,
    OP_NOT,
    OP_OR,
    OP_P2I,
    OP_REINT,
    OP_RET,
    OP_S2F,
    OP_SAR,
    OP_SDIV,
    OP_SEXT,
    OP_SHL,
    OP_SHR,
    OP_SMOD,
    OP_STORE,
    OP_STRING,
    OP_U2F,
    OP_UDIV,
    OP_UMOD,
    OP_UNREACHABLE,
    OP_XOR,
    OP_ZEXT,
};

/// The different predicates for comparison operations.
enum CMPPredicate : uint16_t {
    CMP_IEQ,
    CMP_INE,
    CMP_OEQ,
    CMP_ONE,
    CMP_SLT,
    CMP_SLE,
    CMP_SGT,
    CMP_SGE,
    CMP_ULT,
    CMP_ULE,
    CMP_UGT,
    CMP_UGE,
    CMP_OLT,
    CMP_OLE,
    CMP_OGT,
    CMP_OGE,
};

/// Descriptive information for certain operations.
union Descriptor final {
public:
    /// Alignment for memory accesses (OP_LOAD, OP_STORE).
    uint16_t alignment;

    /// Predicates for comparisons (OP_CMP).
    CMPPredicate cmp;
};

/// Represents an instruction in the IR.
class Instruction final : public User {
    friend class Builder;

    Mnemonic m_op;

    Descriptor m_desc;

    /// The result of a defining instruction. For non-defs, this is always a
    /// sentinel value of 0.
    uint32_t m_def;

    /// The basic block that this instruction is contained in.
    BasicBlock* m_parent;

    /// Links to the instructions before and after this one in the parent block. 
    /// These pointers effectively make up the doubly-linked list managed by 
    /// the parent block.
    Instruction* m_prev = nullptr;
    Instruction* m_next = nullptr;

    Instruction(Type* type, Mnemonic op, BasicBlock* parent, uint32_t def = 0, 
                Descriptor desc = {}, const std::vector<Value*>& ops = {})
      : User(type, ops), m_op(op), m_parent(parent), m_def(def), m_desc(desc) {}

public:
    void print(std::ostream& os) const override;

    /// Returns the mnemonic for the operation this instruction performs.
    Mnemonic op() const { return m_op; }

    /// Returns the descriptor information for this instruction.
    const Descriptor& desc() const { return m_desc; }
    Descriptor& desc() { return m_desc; }

    /// Returns the defined vreg for this instruction, if it is a defining
    /// operation, and 0 otherwise.
    uint32_t get_def() const { return m_def; }

    const Value* get_operand(uint32_t i) const {
        assert(i < num_operands() && "index out of bounds!");
        return m_operands[i]->get_value();
    }

    Value* get_operand(uint32_t i) {
        assert(i < num_operands() && "index out of bounds!");
        return m_operands.at(i)->get_value();
    }

    void set_parent(BasicBlock* block) { m_parent = block; }
    void clear_parent() { m_parent = nullptr; }
    const BasicBlock* get_parent() const { return m_parent; }
    BasicBlock* get_parent() { return m_parent; }

    bool has_parent() const { return m_parent != nullptr; }

    /// Detach this instruction from its parent basic block, removing it from
    /// the list of instructions.
    ///
    /// This does not free any memory allocated for this instruction.
    void detach();

    /// Prepend this instruction to the front of the given |block|.
    void prepend_to(BasicBlock* block);

    /// Append this instruction to the back of the given |block|.
    void append_to(BasicBlock* block);

    /// Insert this instruction before |inst|. Fails if this instruction 
    /// already belongs to a basic block, i.e. has a parent block.
    void insert_before(Instruction* inst);

    /// Insert this instruction after |inst|. Fails if this instruction already
    /// belongs to a basic block, i.e. has a parent block.
    void insert_after(Instruction* inst);

    void set_prev(Instruction* inst) { m_prev = inst; }
    const Instruction* get_prev() const { return m_prev; }
    Instruction* get_prev() { return m_prev; }

    void set_next(Instruction* inst) { m_next = inst; }
    const Instruction* get_next() const { return m_next; }
    Instruction* get_next() { return m_next; }

    /// Test if this instruction defines a value.
    bool is_def() const { return m_def != 0; }

    /// Test if this instruction terminates control flow from a block.
    bool is_terminator() const;

    /// For JMP instructions, returns the arguments used when branching to the
    /// destination block.
    std::vector<const Value*> get_jump_args() const;

    /// For JIF instructions, returns the arguments used when branching to
    /// the "true" destination. Fails by assertion if this is not a JIF 
    /// instruction.
    std::vector<const Value*> get_jif_true_args() const;

    /// For JIF instructions, returns the arguments used when branching to
    /// the "false" destination. Fails by assertion if this is not a JIF 
    /// instruction. 
    std::vector<const Value*> get_jif_false_args() const;

    bool is_trivially_dead() const;
};

} // namespace lir

#endif // LOVELACE_IR_INSTRUCTION_H_

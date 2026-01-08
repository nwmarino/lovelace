//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_BASICBLOCK_H_
#define LOVELACE_IR_BASICBLOCK_H_

//
//  This header file declares the BasicBlock class, which is used to organize
//  a list of instructions.
//

#include "lir/graph/Instruction.hpp"

#include <cassert>
#include <vector>

namespace lir {

class BasicBlock;
class Function;

class BasicBlock final {
public:
    using Preds = std::vector<BasicBlock*>;
    using Succs = std::vector<BasicBlock*>;

    /// Represents an argument to a basic block.
    class Arg final : public Value {
        BasicBlock* m_parent;

        Arg(Type* type, BasicBlock* parent) : Value(type), m_parent(parent) {}

    public:
        [[nodiscard]] static Arg* create(Type* type, BasicBlock* parent = nullptr);

        void set_parent(BasicBlock* block) { m_parent = block; }
        const BasicBlock* get_parent() const { return m_parent; }
        BasicBlock* get_parent() { return m_parent; }

        /// Returns the index of this argument in its parent block. Fails if 
        /// this argument does not belong to a block.
        uint32_t get_index() const;

        void print(std::ostream& os) const override;
    };

    using Args = std::vector<Arg*>;

private:
    Function* m_parent;
    Args m_args;
    BasicBlock* m_prev = nullptr;
    BasicBlock* m_next = nullptr;
    Instruction* m_head = nullptr;
    Instruction* m_tail = nullptr;
    Preds m_preds = {};
    Succs m_succs = {};

    BasicBlock(Function* parent, const Args& args) 
      : m_parent(parent), m_args(args) {}

public:
    /// Create a new basic block. If the |parent| argument is provided, the new
    /// block will be automatically append to it.
    [[nodiscard]] static BasicBlock* create(const Args& args = {}, 
                                            Function* parent = nullptr);

    ~BasicBlock();

    BasicBlock(const BasicBlock&) = delete;
    void operator=(const BasicBlock&) = delete;

    BasicBlock(BasicBlock&&) noexcept = delete;
    void operator=(BasicBlock&&) noexcept = delete;

    void set_parent(Function* function) { m_parent = function; }
    void clear_parent() { m_parent = nullptr; }
    const Function* get_parent() const { return m_parent; }
    Function* get_parent() { return m_parent; }

    bool has_parent() const { return m_parent != nullptr; }

    /// Detach this basic block from its parent function. 
    /// 
    /// Does not free any memory allocated for this block.
    void detach();

    const Args& get_args() const { return m_args; }
    Args& get_args() { return m_args; }

    const BasicBlock::Arg* get_arg(uint32_t i) const {
        assert(i < num_args() && "index out of bounds!");
        return m_args[i];
    }

    BasicBlock::Arg* get_arg(uint32_t i) {
        assert(i < num_args() && "index out of bounds!");
        return m_args[i];
    }

    uint32_t num_args() const { return m_args.size(); }
    bool has_args() const { return !m_args.empty(); }

    /// Set the argument at position |i| to |arg|.
    void set_arg(uint32_t i, BasicBlock::Arg* arg) {
        assert(i < num_args() && "index out of bounds!");
        m_args[i] = arg;
    }

    /// Append the given |arg| to the back of this blocks' argument list.
    void append_arg(BasicBlock::Arg* arg) { m_args.push_back(arg); }

    /// Append this basic block to the given |function|. Fails if this block
    /// already belongs to a function.
    void append_to(Function* function);

    /// Insert this basic block before |block|. Fails if this block already
    /// belongs to a function.
    void insert_before(BasicBlock* block);

    /// Insert this basic block after |block|. Fails if this block already
    /// belongs to a function.
    void insert_after(BasicBlock* block);

    /// Remove the given |inst| from this basic block, if it belongs.
    void remove(Instruction* inst);

    /// Test if this block has a parent function and is the first block in that 
    /// function.
    inline bool is_entry_block() const { 
        return m_parent != nullptr && m_prev == nullptr; 
    }

    void set_prev(BasicBlock* block) { m_prev = block; }
    const BasicBlock* get_prev() const { return m_prev; }
    BasicBlock* get_prev() { return m_prev; }

    void set_next(BasicBlock* block) { m_next = block; }
    const BasicBlock* get_next() const { return m_next; }
    BasicBlock* get_next() { return m_next; }

    void set_head(Instruction* inst) { m_head = inst; }
    const Instruction* get_head() const { return m_head; }
    Instruction* get_head() { return m_head; }

    void set_tail(Instruction* inst) { m_tail = inst; }
    const Instruction* get_tail() const { return m_tail; }
    Instruction* get_tail() { return m_tail; }

    /// Prepend the given |inst| to this block.
    void prepend(Instruction* inst);

    /// Append the given |inst| to this block.
    void append(Instruction* inst);

    /// Insert the given |inst| into this block at index |i|.
    void insert(Instruction* inst, uint32_t i);

    /// Insert the given |inst| into this basic block immediately after 
    /// |insert_after|. Fails if |inst| is already in a block, or 
    /// if |insert_after| is not part of this block.
    void insert(Instruction* inst, Instruction* insert_after);

    /// Test if this basic block has zero instructions.
    bool empty() const { return m_head == nullptr; }

    /// Returns the size of this basic block based on the number of 
    /// instructions in it.
    uint32_t size() const;

    /// Returns the numeric position of this basic block relative to other
    /// blocks in the parent function.
    uint32_t get_number() const;

    const Preds& get_preds() const { return m_preds; }
    Preds& get_preds() { return m_preds; }

    uint32_t num_preds() const { return m_preds.size(); }
    bool has_preds() const { return !m_preds.empty(); }

    const Preds& get_succs() const { return m_succs; }
    Preds& get_succs() { return m_succs; }

    uint32_t num_succs() const { return m_succs.size(); }
    bool has_succs() const { return !m_succs.empty(); }

    /// Test if this basic block contains a terminating instruction.
    bool terminates() const;

    /// Returns the number of terminating instructions in this basic block.
    uint32_t terminators() const;
    
    /// Returns the earliest terminating instruction in this basic block, if 
    /// one exists.
    const Instruction* terminator() const;
    Instruction* terminator() {
        return const_cast<Instruction*>(
            static_cast<const BasicBlock*>(this)->terminator());
    }

    /// Print this basic block in a plaintext format to |os|.
    void print(std::ostream& os) const;
};

} // namespace lir

#endif // LOVELACE_IR_BASICBLOCK_H_

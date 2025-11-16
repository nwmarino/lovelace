#ifndef SPBE_MACH_BASIC_BLOCK_H_
#define SPBE_MACH_BASIC_BLOCK_H_

#include "MachInstruction.hpp"

#include <cassert>
#include <vector>

namespace spbe {

class BasicBlock;

/// Represents a target-dependent basic block, derived from a bytecode block.
class MachBasicBlock final {
    friend class MachFunction;

    /// The bytecode block this basic block derives from.
    const BasicBlock* m_bb;

    /// The parent function of this basic block.
    MachFunction* m_parent;

    /// The instructions in this block.
    std::vector<MachInstruction> m_insts;

    /// Links to the previous and next block in the parent function.
    MachBasicBlock* m_prev = nullptr;
    MachBasicBlock* m_next = nullptr;

public:
    MachBasicBlock(const BasicBlock* bb, MachFunction* parent = nullptr);

    /// Returns the corresponding SIIR basic block this block derives from, 
    /// if it exists.
    const BasicBlock* get_basic_block() const { return m_bb; }

    /// Returns the parent function to this basic block.
    const MachFunction* get_parent() const { return m_parent; }
    MachFunction* get_parent() { return m_parent; }

    /// Clears the parent link of this basic block. Does not detach it from
    /// any existing parent function.
    void clear_parent() { m_parent = nullptr; }

    /// Set the parent function of this basic block to |mf|.
    void set_parent(MachFunction* mf) { m_parent = mf; }

    /// Returns the position of this block relative to other blocks in its
    /// parent function.
    uint32_t position() const;

    /// Returns true if this basic block has no instructions.
    bool empty() const { return m_insts.empty(); }

    /// Returns the number of instructions in this block.
    uint32_t size() const { return m_insts.size(); }

    const MachInstruction& front() const { return m_insts.front(); }
    MachInstruction& front() { return m_insts.front(); }

    const MachInstruction& back() const { return m_insts.back(); }
    MachInstruction& back() { return m_insts.back(); }

    const std::vector<MachInstruction>& insts() const { return m_insts; }
    std::vector<MachInstruction>& insts() { return m_insts; }

    const MachBasicBlock* prev() const { return m_prev; }
    MachBasicBlock* prev() { return m_prev; }

    const MachBasicBlock* next() const { return m_next; }
    MachBasicBlock* next() { return m_next; }

    void set_prev(MachBasicBlock* prev) { m_prev = prev; }
    void set_next(MachBasicBlock* next) { m_next = next; }

    /// Append |inst| to the back of this basic block.
    void push_front(MachInstruction& inst) {
        inst.set_parent(this);
        m_insts.insert(m_insts.begin(), inst);
    }

    /// Prepend |inst| to the front of this basic block.
    void push_back(MachInstruction& inst) {
        inst.set_parent(this);
        m_insts.push_back(inst);
    }

    void insert(MachInstruction& inst, uint32_t i) {
        assert(i < size());
        inst.set_parent(this);
        m_insts.insert(m_insts.begin() + i, inst);
    }
};

} // namespace spbe

#endif // SPBE_MACH_BASIC_BLOCK_H_

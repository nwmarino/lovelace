//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_MACH_LABEL_H_
#define LOVELACE_IR_MACH_LABEL_H_

//
//  This header file declares the MachLabel class, which is used to represent
//  basic block derivatives in the X64 machine IR.
//

#include "lir/machine/MachInst.hpp"

#include <cassert>
#include <vector>

namespace lir {

class BasicBlock;

class MachLabel final {
    friend class MachFunction;

    /// The CFG block this label derives from.
    const BasicBlock* m_block;

    /// The parent function of this label.
    MachFunction* m_parent;

    /// The instructions under this label.
    std::vector<MachInst> m_insts;

    /// Links to the previous and next label in the parent function.
    MachLabel* m_prev = nullptr;
    MachLabel* m_next = nullptr;

public:
    MachLabel(const BasicBlock* block, MachFunction* parent = nullptr);

    /// Returns the CFG basic block this label was derived from.
    const BasicBlock* get_basic_block() const { return m_block; }

    void set_parent(MachFunction* parent) { m_parent = parent; }
    void clear_parent() { m_parent = nullptr; }
    const MachFunction* get_parent() const { return m_parent; }
    MachFunction* get_parent() { return m_parent; }

    /// Returns the position of this block relative to other blocks in its
    /// parent function.
    uint32_t position() const;

    /// Returns true if this basic block has no instructions.
    bool empty() const { return m_insts.empty(); }

    /// Returns the number of instructions in this block.
    uint32_t size() const { return m_insts.size(); }

    const MachInst& front() const { return m_insts.front(); }
    MachInst& front() { return m_insts.front(); }

    const MachInst& back() const { return m_insts.back(); }
    MachInst& back() { return m_insts.back(); }

    const std::vector<MachInst>& insts() const { return m_insts; }
    std::vector<MachInst>& insts() { return m_insts; }

    void set_prev(MachLabel* prev) { m_prev = prev; }
    const MachLabel* get_prev() const { return m_prev; }
    MachLabel* get_prev() { return m_prev; }

    void set_next(MachLabel* next) { m_next = next; }
    const MachLabel* get_next() const { return m_next; }
    MachLabel* get_next() { return m_next; }

    /// Prepend the given |inst| to the front of this label.
    void prepend(MachInst& inst) {
        inst.set_parent(this);
        m_insts.insert(m_insts.begin(), inst);
    }

    /// Append the given |inst| to the back of this label.
    void append(MachInst& inst) {
        inst.set_parent(this);
        m_insts.push_back(inst);
    }

    void insert(MachInst& inst, uint32_t i) {
        assert(i < size() && "index out of bounds!");
        inst.set_parent(this);
        m_insts.insert(m_insts.begin() + i, inst);
    }
};

} // namespace lir

#endif // LOVELACE_IR_MACH_LABEL_H_

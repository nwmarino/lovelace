//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_MACH_FUNCTION_H_
#define LOVELACE_IR_MACH_FUNCTION_H_

#include "lir/graph/Constant.hpp"
#include "lir/graph/Local.hpp"
#include "lir/machine/Machine.hpp"
#include "lir/machine/Register.hpp"
#include "lir/machine/MachLabel.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace lir {

class Function;
class X64_Inst;

/// An entry in the stack frame of a function.
///
/// This databag effectively reverses space on the stack of a function for a 
/// local in the CFG-equivelant function.
struct StackEntry final {
    /// The offset of this entry in the stack.
    int32_t offset;

    /// The number of bytes this entry reserves.
    uint32_t size;

    /// The desired alignment of this entry.
    uint32_t align;

    /// The local that defines this entry, if there is one.
    ///
    /// It should be noted that some entries used for spills after instruction
    /// selection do not have a corresponding local.
    const Local* local = nullptr;
};

/// Information about the stack frame of a function.
struct StackFrame final {
    std::vector<StackEntry> entries;

    /// Returns the number of entries in this stack.
    uint32_t num_entries() const { return entries.size(); }

    /// Returns the size of the stack in bytes, without any alignment.
    uint32_t size() const {
        if (entries.empty())
            return 0;

        return entries.back().offset + entries.back().size;
    }

    /// Returns the necessary byte alignment for this stack frame.
    uint32_t alignment() const {
        uint32_t max_align = 1;
        for (const auto& entry : entries)
            if (entry.align > max_align)
                max_align = entry.align;
        
        uint32_t size = this->size();
        while (max_align < size)
            max_align += 16;

        if (max_align % 16 != 0)
            max_align += 16 - (max_align % 16);

        return max_align;
    }
};

/// An entry in the constant pool of a function.
struct ConstantPoolEntry final {
    const Constant* constant;
    uint32_t align;
};

/// Read-only constants used by a function.
struct ConstantPool final {
    std::vector<ConstantPoolEntry> entries;

    /// Returns the number of entries in this pool.
    uint32_t num_entries() const { return entries.size(); }

    uint32_t get_or_create_constant(const Constant* constant, uint32_t align) {
        uint32_t index = 0;
        for (uint32_t e = entries.size(); index < e; ++index) {
            ConstantPoolEntry& entry = entries[index];

            // @Todo: Optimize comparisons to reduce duplicate constants.
            if (entry.constant == constant && entry.align == align)
                return index;
        }

        entries.emplace_back(constant, align);
        return index;
    }
};

/// Information about a virtual register.
///
/// This databag does not represent virtual registers in the machine IR, 
/// rather, it is used for functions to organize mappings before and after
/// register allocation.
struct VirtualRegister final {
    /// The desired class for a virtual register post-allocation.
    RegisterClass cls = GeneralPurpose;

    /// The resulting allocation of a virtual register.
    Register alloc = Register::NoRegister;
};

/// Represents a machine function, derived from a bytecode function.
class MachFunction final {
    friend class RegisterAnalysis;

public:
    using RegisterTable = std::unordered_map<uint32_t, VirtualRegister>;

private:
    ConstantPool m_pool;
    StackFrame m_stack;
    RegisterTable m_regs;

    const Function* m_function;
    const Machine& m_mach;

    /// Links to the first and last block labels in this function. 
    MachLabel* m_head = nullptr;
    MachLabel* m_tail = nullptr;

public:
    MachFunction(const Function* function, const Machine& mach);

    ~MachFunction();

    MachFunction(const MachFunction&) = delete;
    void operator=(const MachFunction&) = delete;

    MachFunction(MachFunction&&) noexcept = delete;
    void operator=(MachFunction&&) noexcept = delete;

    /// Returns the CFG function that this derives from.
    const Function* get_function() const { return m_function; }
    
    /// Returns the target machine this function was compiled for.
    const Machine& get_machine() const { return m_mach; }

    /// Returns the name of this function, as it was defined in the SIIR.
    const std::string& get_name() const;

    const ConstantPool& get_constant_pool() const { return m_pool; }
    ConstantPool& get_constant_pool() { return m_pool; }

    const StackFrame& get_stack_frame() const { return m_stack; }
    StackFrame& get_stack_frame() { return m_stack; }

    const RegisterTable& get_register_table() const { return m_regs; }
    RegisterTable& get_register_table() { return m_regs; }

    const MachLabel* get_head() const { return m_head; }
    MachLabel* get_head() { return m_head; }

    const MachLabel* get_tail() const { return m_tail; }
    MachLabel* get_tail() { return m_tail; }

    const MachLabel* at(uint32_t i) const;
    MachLabel* at(uint32_t i) {
        return const_cast<MachLabel*>(
            static_cast<const MachFunction*>(this)->at(i));
    }

    /// Returns the number of basic blocks in this function.
    uint32_t size() const;

    /// Test if this function has no basic blocks.
    bool empty() const { return m_head == nullptr; }

    /// Prepend the given |label| to the front of this function.
    void prepend(MachLabel* label);

    /// Append the given |label| to the back of this function.
    void append(MachLabel* label);
};

} // namespace lir

#endif // LOVELACE_IR_MACH_FUNCTION_H_

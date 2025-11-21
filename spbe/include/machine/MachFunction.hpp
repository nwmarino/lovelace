#ifndef SPBE_MACH_FUNCTION_H_
#define SPBE_MACH_FUNCTION_H_

#include "MachBasicBlock.hpp"
#include "MachRegister.hpp"
#include "../graph/Constant.hpp"
#include "../graph/Local.hpp"
#include "../target/Target.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace spbe {

class Function;
class MachInstruction;

/// An entry in the stack frame of a function.
///
/// This databag effectively reverses space on the stack of a function for a 
/// local in the SIIR equivelant function.
struct FunctionStackEntry final {
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

/// Information about the stack of a machine function.
struct FunctionStackInfo final {
    std::vector<FunctionStackEntry> entries;

    /// Returns the number of entries in this stack.
    uint32_t num_entries() const { return entries.size(); }

    /// Returns the size of the stack in bytes, without any alignment.
    uint32_t size() const {
        if (entries.empty())
            return 0;

        return entries.back().offset + entries.back().size;
    }

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

/// Information about a virtual register.
struct VRegInfo final {
    /// The desired class for a virtual register post-allocation.
    RegisterClass cls = GeneralPurpose;

    /// The resulting allocation of a virtual register.
    MachRegister alloc = MachRegister::NoRegister;
};

/// Information about the registers used by a machine function.
struct FunctionRegisterInfo final {
    std::unordered_map<uint32_t, VRegInfo> vregs;
};

/// An entry in the constant pool of a function.
struct FunctionConstantPoolEntry final {
    const Constant* constant;
    uint32_t align;
};

/// Constants referenced by a function that should be emitted to read-only
/// data sections.
struct FunctionConstantPool final {
    std::vector<FunctionConstantPoolEntry> entries;

    /// Returns the number of entries in this pool.
    uint32_t num_entries() const { return entries.size(); }

    uint32_t get_or_create_constant(const Constant* constant, uint32_t align) {
        uint32_t idx = 0;
        for (uint32_t e = entries.size(); idx != e; ++idx) {
            FunctionConstantPoolEntry& entry = entries[idx];
            // TODO: Optimize comparisons to reduce duplicate constants.
            if (entry.constant == constant && entry.align == align)
                return idx;
        }

        entries.push_back({ constant, align });
        return idx;
    }
};

/// Represents a machine function, derived from a bytecode function.
class MachFunction final {
    friend class FunctionRegisterAnalysis;

    /// Internal information about this function.
    FunctionStackInfo m_stack;
    FunctionRegisterInfo m_regi;
    FunctionConstantPool m_pool;

    /// The bytecode function this derives from.
    const Function* m_fn;
    const Target& m_target;

    /// Links to the first and last basic blocks in this function. 
    MachBasicBlock* m_front = nullptr;
    MachBasicBlock* m_back = nullptr;

public:
    MachFunction(const Function* fn, const Target& target);

    MachFunction(const MachFunction&) = delete;
    MachFunction& operator = (const MachFunction&) = delete;

    ~MachFunction();

    /// Returns the SIIR function that this function derives from.
    const Function* get_function() const { return m_fn; }
    
    /// Returns the target that this function was compiled for.
    const Target& get_target() const { return m_target; }

    /// Returns the name of this function, as it was defined in the SIIR.
    const std::string& get_name() const;

    const FunctionStackInfo& get_stack_info() const { return m_stack; }
    FunctionStackInfo& get_stack_info() { return m_stack; }

    const FunctionRegisterInfo& get_register_info() const { return m_regi; }
    FunctionRegisterInfo& get_register_info() { return m_regi; }

    const FunctionConstantPool& get_constant_pool() const { return m_pool; }
    FunctionConstantPool& get_constant_pool() { return m_pool; }

    const MachBasicBlock* front() const { return m_front; }
    MachBasicBlock* front() { return m_front; }

    const MachBasicBlock* back() const { return m_back; }
    MachBasicBlock* back() { return m_back; }

    /// Return the basic block at position |idx| in this function.
    const MachBasicBlock* at(uint32_t idx) const;
    MachBasicBlock* at(uint32_t idx);

    /// Returns the number of basic blocks in this function.
    uint32_t size() const;

    /// Returns true if this function has no basic blocks.
    bool empty() const { return !m_front; }

    /// Prepend |mbb| to the front of this function.
    void prepend(MachBasicBlock* mbb);

    /// Append |mbb| to the back of this function.
    void append(MachBasicBlock* mbb);
};

} // namespace spbe

#endif // SPBE_MACH_FUNCTION_H_

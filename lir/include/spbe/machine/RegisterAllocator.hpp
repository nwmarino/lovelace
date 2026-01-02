//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SPBE_REGISTER_ALLOCATOR_H_
#define SPBE_REGISTER_ALLOCATOR_H_

#include "spbe/machine/MachFunction.hpp"
#include "spbe/machine/MachRegister.hpp"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace spbe {

struct RegisterSet final {
    RegisterClass cls;
    std::vector<uint32_t> regs;
};

struct TargetRegisters final {
    std::unordered_map<RegisterClass, RegisterSet> regs;
};

/// Represents the positional range in which a register is live.
struct LiveRange final {
    /// The register that this range represents, pre-allocations. For ranges
    /// made for physical registers, this still represents the physical
    /// register.
    MachRegister reg;

    /// The physical register that was allocated over this range.
    MachRegister alloc;

    /// The start and end positions of this range.
    uint32_t start, end;

    /// The desired register class for this range.
    RegisterClass cls;

    /// If true, then this range is considered dead and should no longer be
    /// extended.
    bool killed;

    /// Returns true if this range in any way overlaps with the given position.
    bool overlaps(uint32_t pos) const {
        return this->start < pos && pos < this->end;
    }

    /// Returns true if this range in any way overlaps with the bounds 
    /// [start, end].
    bool overlaps(uint32_t start, uint32_t end) const {
        return this->start < end && this->end > start;
    }
};

class RegisterAllocator {
    const TargetRegisters& m_pool;
    MachFunction& m_function;
    
    std::vector<LiveRange>& m_ranges;
    std::vector<LiveRange> m_active = {};
    
    bool active_contains(MachRegister reg) const;
    bool is_available(MachRegister reg, uint32_t start, uint32_t end) const;

    void expire_intervals(LiveRange& curr);
    void assign_register(LiveRange& range);

public:
    RegisterAllocator(MachFunction& function, const TargetRegisters& pool,
                      std::vector<LiveRange>& ranges);

    RegisterAllocator(const RegisterAllocator&) = delete;
    RegisterAllocator& operator = (const RegisterAllocator&) = delete;

    ~RegisterAllocator() = default;

    void run();
};

} // namespace spbe

#endif // SPBE_REGISTER_ALLOCATOR_H_

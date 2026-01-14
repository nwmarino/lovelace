//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_REGISTER_ALLOCATOR_H_
#define LOVELACE_IR_REGISTER_ALLOCATOR_H_

#include "lir/machine/MachFunction.hpp"
#include "lir/machine/Register.hpp"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace lir {

/// Represents the range in which a register is live.
struct LiveRange final {
    /// The register that this range represents, pre-allocations. For ranges
    /// made for physical registers, this still represents the physical
    /// register.
    Register reg;

    /// The physical register that was allocated over this range.
    Register alloc;

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

class RegisterAllocator final {
public:
    using RegisterPool = std::unordered_map<
        RegisterClass, std::vector<X64_Register>>;

private:
    RegisterPool m_pool;
    MachFunction& m_function;
    
    std::vector<LiveRange>& m_ranges;
    std::vector<LiveRange> m_active = {};
    
    bool active_contains(Register reg) const;
    bool is_available(Register reg, uint32_t start, uint32_t end) const;

    void expire_intervals(LiveRange& curr);
    void assign_register(LiveRange& range);

public:
    RegisterAllocator(MachFunction& function, std::vector<LiveRange>& ranges);

    void run();
};

} // namespace lir

#endif // LOVELACE_IR_REGISTER_ALLOCATOR_H_

//
//  Copyright (c) 2025 Nick Marino
//  All rights reserved.
//

#include "lir/machine/RegisterAllocator.hpp"

using namespace lir;

bool RegisterAllocator::is_available(
        Register reg, uint32_t start, uint32_t end) const {
    // @Todo: This ends up being very slow, since it covers all ranges in a
    // function. Should be optimized, i.e. keeping a set of non-active but
    // overlapping ranges, which active is a subset of.
    for (const LiveRange& range : m_ranges) {
        // For each range within the function, if it allocates |reg| and 
        // overlaps with [start, end], then |reg| is considered unavailable.
        if (range.alloc == reg && range.overlaps(start, end))
            return false;
    }

    return true;
}

void RegisterAllocator::expire_intervals(LiveRange& curr) {
    for (auto it = m_active.begin(); it != m_active.end(); ) {
        if (it->end < curr.start) {
            m_active.erase(it);
        } else {
            ++it;
        }
    }
}

void RegisterAllocator::assign_register(LiveRange& range) {
    const auto& set = m_pool.at(range.cls);
    for (const auto& reg : set) {
        assert(Register(reg).is_physical() && "expected physical register!");

        if (is_available(reg, range.start, range.end)) {
            range.alloc = reg;
            break;
        }
    }

    assert(range.alloc != Register::NoRegister &&
        "failed to allocate register!");
}

RegisterAllocator::RegisterAllocator(MachFunction& function,
                                     std::vector<LiveRange>& ranges)
  : m_function(function), m_ranges(ranges) {
    m_pool[RegisterClass::GeneralPurpose] = {
        RAX, RCX, RDX, RSI, RDX, 
        R8, R9, R10, R11, 
        R12, R13, R14, R15
    };

    m_pool[RegisterClass::FloatingPoint] = {
        XMM0, XMM1, XMM2, XMM3, 
        XMM4, XMM5, XMM6, XMM7, 
        XMM8, XMM9, XMM10, XMM11,
        XMM12, XMM13, XMM14, XMM15,
    };
}

void RegisterAllocator::run() {
    for (auto& range : m_ranges) {
        expire_intervals(range);

        if (range.alloc == Register::NoRegister)
            assign_register(range);
        
        m_active.push_back(range);
    }
}

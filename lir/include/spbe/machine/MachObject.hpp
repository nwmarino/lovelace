//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SPBE_MACH_OBJECT_H_
#define SPBE_MACH_OBJECT_H_

#include "spbe/machine/MachFunction.hpp"
#include "spbe/target/Target.hpp"

#include <string>
#include <unordered_map>

namespace spbe {

class MachObject final {
    const Target* m_target;
    const CFG* m_cfg;
    std::unordered_map<std::string, MachFunction*> m_functions;

public:
    /// Create a new machine object for the given target.
    MachObject(const CFG* cfg, const Target* target) 
        : m_cfg(cfg), m_target(target) {}

    MachObject(const MachObject&) = delete;
    MachObject& operator = (const MachObject&) = delete;

    ~MachObject();

    /// Returns the SIIR control flow graph this machine object derives from.
    const CFG* get_graph() const { return m_cfg; }

    /// Return the target that this machine object was compiled for.
    const Target* get_target() const { return m_target; }

    const std::unordered_map<std::string, MachFunction*>& functions() const {
        return m_functions;
    }

    std::unordered_map<std::string, MachFunction*>& functions() {
        return m_functions;
    }
};

} // namespace spbe

#endif // SPBE_MACH_OBJECT_H_

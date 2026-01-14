//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_SEGMENT_H_
#define LOVELACE_IR_SEGMENT_H_

#include "lir/graph/CFG.hpp"
#include "lir/machine/Machine.hpp"
#include "lir/machine/MachFunction.hpp"

#include <map>
#include <string>

namespace lir {

class Segment final {
public:
    using FunctionTable = std::map<std::string, MachFunction*>;

private:
    const CFG& m_cfg;
    FunctionTable m_functions = {};

public:
    Segment(const CFG& cfg) : m_cfg(cfg) {}

    ~Segment();

    Segment(const Segment&) = delete;
    void operator=(const Segment&) = delete;

    Segment(Segment&&) noexcept = delete;
    void operator=(Segment&&) noexcept = delete;

    const CFG& get_graph() const { return m_cfg; }

    const Machine& get_machine() const { return m_cfg.get_machine(); }

    const FunctionTable& get_functions() const { return m_functions; }
    FunctionTable& get_functions() { return m_functions; }
};

} // namespace lir

#endif // LOVELACE_IR_SEGMENT_H_

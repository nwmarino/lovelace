//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SPBE_TRIVIAL_DCE_PASS_H_
#define SPBE_TRIVIAL_DCE_PASS_H_

#include "spbe/analysis/Pass.hpp"
#include "spbe/graph/Instruction.hpp"

#include <vector>

namespace spbe {

class TrivialDCEPass final : public Pass {
    void process(Function* fn);

    std::vector<Instruction*> m_to_remove = {};

public:
    TrivialDCEPass(CFG& cfg) : Pass(cfg) {}

    TrivialDCEPass(const TrivialDCEPass&) = delete;
    TrivialDCEPass& operator = (const TrivialDCEPass&) = delete;

    void run() override;
};

} // namespace spbe

#endif // SPBE_TRIVIAL_DCE_PASS_H_

//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "spbe/analysis/TrivialDCEPass.hpp"

using namespace spbe;

void TrivialDCEPass::run() {
    for (const auto& fn : m_cfg.functions())
        process(fn);
}

void TrivialDCEPass::process(Function* fn) {
    for (auto block = fn->front(); block != nullptr; block = block->next())
        for (auto inst = block->front(); inst != nullptr; inst = inst->next())
            if (inst->is_trivially_dead())
                m_to_remove.push_back(inst);

    for (const auto& inst : m_to_remove) {
        inst->detach_from_parent();
        delete inst;
    }

    m_to_remove.clear();
}

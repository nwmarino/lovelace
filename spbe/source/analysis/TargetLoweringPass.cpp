//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "spbe/analysis/TargetLoweringPass.hpp"
#include "spbe/target/Target.hpp"
#include "spbe/X64/X64.hpp"
#include "spbe/X64/X64InstrSelector.hpp"

using namespace spbe;

void TargetLoweringPass::run() {
    for (const auto& function : m_cfg.functions()) {
        // Empty functions should not be lowered, they should either be
        // resolved at link time or with some library.
        if (function->empty())
            continue;

        MachFunction* mf = new MachFunction(function, *m_obj.get_target());
        m_obj.functions().emplace(mf->get_name(), mf);

        for (auto* curr = function->front(); curr != nullptr; curr = curr->next())
            new MachBasicBlock(curr, mf);

        switch (m_obj.get_target()->arch()) {
        case Target::Arch::x64: {
            x64::X64InstrSelector isel(*mf);
            isel.run();
            break;
        }

        default:
            assert(false && "unsupported architecture!");
        }
    }
}

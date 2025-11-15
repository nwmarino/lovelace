#include "../../include/analysis/TargetLoweringPass.hpp"
#include "../../include/target/Target.hpp"
#include "../../include/target/x64.hpp"

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
            x64::X64InstSelection isel { mf };
            isel.run();
            break;
        }

        default:
            assert(false && "unsupported architecture!");
        }
    }
}

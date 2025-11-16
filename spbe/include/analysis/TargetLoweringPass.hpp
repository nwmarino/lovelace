#ifndef SPBE_TARGET_LOWERING_PASS_H_
#define SPBE_TARGET_LOWERING_PASS_H_

#include "Pass.hpp"
#include "machine/MachObject.hpp"

namespace spbe {

class TargetLoweringPass final : public Pass {
    MachObject& m_obj;

public:
    TargetLoweringPass(CFG& cfg, MachObject& obj) : Pass(cfg), m_obj(obj) {}

    TargetLoweringPass(const TargetLoweringPass&) = delete;
    TargetLoweringPass& operator = (const TargetLoweringPass&) = delete;

    void run() override;
};

} // namespace spbe

#endif // SPBE_TARGET_LOWERING_PASS_H_

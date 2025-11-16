#ifndef SPBE_PASS_H_
#define SPBE_PASS_H_

#include "../graph/CFG.hpp"

namespace spbe {
     
class Pass {
protected:
    CFG& m_cfg;

public:
    Pass(CFG& cfg) : m_cfg(cfg) {}

    Pass(const Pass&) = delete;
    Pass& operator = (const Pass&) = delete;

    virtual ~Pass() = default;

    virtual void run() = 0;
};

} // namespace spbe

#endif // SPBE_PASS_H_

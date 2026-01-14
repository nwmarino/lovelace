//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_REGISTER_ANALYSIS_H_
#define LOVELACE_IR_REGISTER_ANALYSIS_H_

#include "lir/machine/Segment.hpp"

namespace lir {

class RegisterAnalysis {
    Segment& m_seg;

public:
    RegisterAnalysis(Segment& seg) : m_seg(seg) {}

    void run();
};

} // namespace lir

#endif // LOVELACE_IR_REGISTER_ANALYSIS_H_

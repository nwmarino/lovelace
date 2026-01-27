//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_PRINTER_H_
#define LOVELACE_IR_PRINTER_H_

#include "lir/machine/Segment.hpp"

namespace lir {

class Printer final {
    const Segment &m_seg;

public:
    Printer(const Segment &seg) : m_seg(seg) {}

    void run(std::ostream &os) const;
};

} // namespace lir

#endif // LOVELACE_IR_PRINTER_H_

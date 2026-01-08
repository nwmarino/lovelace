//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SPBE_X64_PRINTER_H_
#define SPBE_X64_PRINTER_H_

#include "spbe/machine/MachObject.hpp"

namespace spbe::x64 {

class X64Printer final {
    const MachObject& m_obj;

public:
    X64Printer(const MachObject& object) : m_obj(object) {}

    void run(std::ostream& os) const;
};

} // namespace spbe::x64

#endif // SPBE_X64_PRINTER_H_

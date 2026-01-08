//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lir/machine/Segment.hpp"

using namespace lir;

Segment::~Segment() {
    for (auto& [name, function] : m_functions)
        delete function;

    m_functions.clear();
}

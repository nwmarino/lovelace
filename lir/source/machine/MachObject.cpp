//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "spbe/machine/MachObject.hpp"

using namespace spbe;

MachObject::~MachObject() {
    for (auto& [name, function] : m_functions) {
        delete function;
        function = nullptr;
    }

    m_functions.clear();
}

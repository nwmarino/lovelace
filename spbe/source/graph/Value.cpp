//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "spbe/graph/Use.hpp"
#include "spbe/graph/Value.hpp"

#include <algorithm>

using namespace spbe;

void Value::del_use(Use* use) {
    auto it = std::find(m_uses.begin(), m_uses.end(), use);
    if (it != m_uses.end())
        m_uses.erase(it);
}

void Value::replace_all_uses_with(Value* value) {
    std::vector<Use*> uses_copy = m_uses;
    for (Use* use : uses_copy)
        use->set_value(value);
}

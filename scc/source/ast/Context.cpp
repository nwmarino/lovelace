//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "ast/Context.hpp"
#include "ast/Type.hpp"

using namespace scc;

Context::Context() {
    for (uint32_t i = BuiltinType::Void; i < BuiltinType::LongDouble; ++i) {
        m_bts[static_cast<BuiltinType::Kind>(i)] = 
            std::unique_ptr<BuiltinType>(new BuiltinType(
                static_cast<BuiltinType::Kind>(i)));
    }
}

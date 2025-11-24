//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "ast/Type.hpp"
#include "ast/TypeContext.hpp"

using namespace scc;

TypeContext::TypeContext() {
    for (uint32_t i = BuiltinType::Void; i < BuiltinType::LongDouble; ++i) {
        m_bts[static_cast<BuiltinType::Kind>(i)] = new BuiltinType(
            static_cast<BuiltinType::Kind>(i)); 
    }
}

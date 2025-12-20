//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/tree/Type.hpp"
#include "stmc/tree/TypeUse.hpp"

using namespace stm;

bool TypeUse::compare(const TypeUse& other) const {
    return m_quals == other.m_quals && m_type->compare(other);
}

bool TypeUse::can_cast(const TypeUse& other, bool implicitly) const {
    // @Todo: reinvent this, but careful of (mut lval) <- (immut rval) failing.
    // In the above case, the type checker falls back to trying a cast, but 
    // fails due to differences in mutability. Needs more thorough context.
    
    //if (other.is_mut() && !is_mut())
    //    return false;

    return get_type()->can_cast(other.get_type(), implicitly);
}

string TypeUse::to_string() const {
    string str = "";

    if (is_mut())
        str += "mut ";

    return str + get_type()->to_string();
}

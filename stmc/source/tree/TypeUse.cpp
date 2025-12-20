//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/tree/Type.hpp"
#include "stmc/tree/TypeUse.hpp"

using namespace stm;

string TypeUse::to_string() const {
    string str = "";

    if (is_mut())
        str += "mut ";

    return str + get_type()->to_string();
}

//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "ast/Type.hpp"
#include "ast/QualType.hpp"

using namespace scc;

bool QualType::operator == (const QualType& other) const {
    return *m_type == *other.get_type() && m_quals == other.m_quals;
}

bool QualType::operator != (const QualType& other) const {
    return *m_type != *other.get_type() || m_quals != other.m_quals;
}

string QualType::to_string() const {
    string str = "";
    if (is_const())
        str += "const ";

    if (is_volatile())
        str += "volatile ";

    return str + m_type->to_string();
}

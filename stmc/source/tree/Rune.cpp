//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/tree/Expr.hpp"
#include "stmc/tree/Rune.hpp"

using namespace stm;

Rune::~Rune() {
    for (Expr* arg : m_args)
        delete arg;

    m_args.clear();
}

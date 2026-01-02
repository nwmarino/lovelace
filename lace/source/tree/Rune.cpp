//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lace/tree/Expr.hpp"
#include "lace/tree/Rune.hpp"

using namespace lace;

Rune::~Rune() {
    for (Expr* arg : m_args)
        delete arg;

    m_args.clear();
}

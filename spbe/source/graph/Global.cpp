//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "spbe/graph/CFG.hpp"
#include "spbe/graph/Global.hpp"
#include "spbe/graph/Type.hpp"

using namespace spbe;

Global::Global(CFG& cfg, const Type* type, LinkageType linkage, bool read_only, 
               const std::string& name, Constant* init)
    : Constant({ init }, PointerType::get(cfg, type)), 
      m_linkage(linkage), m_read_only(read_only), m_name(name), m_init(init) {
    cfg.add_global(this);
}

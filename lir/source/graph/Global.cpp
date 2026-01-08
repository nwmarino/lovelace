//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lir/graph/CFG.hpp"
#include "lir/graph/Global.hpp"
#include "lir/graph/Type.hpp"

using namespace lir;

Global* Global::create(CFG& cfg, Type* type, LinkageType linkage, 
                       bool read_only, const std::string& name, Constant* init) {
	Global* global = new Global(type, &cfg, linkage, read_only, name, init);
	cfg.add_global(global);
	return global;
}

//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lir/graph/Function.hpp"
#include "lir/graph/Local.hpp"
#include "lir/graph/Type.hpp"

using namespace lir;

Local* Local::create(CFG &cfg, Type *type, const std::string &name, 
					 uint32_t align, Function* parent) {
	Local* local = new Local(
		PointerType::get(cfg, type), parent, name, type, align);

	if (parent)
		parent->add_local(local);

	return local;
}

void Local::detach() {
	assert(m_parent && "local does not belong to a function!");
	m_parent->remove_local(this);
}

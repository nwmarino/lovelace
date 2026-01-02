//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "spbe/graph/Function.hpp"
#include "spbe/graph/Local.hpp"
#include "spbe/graph/Type.hpp"

using namespace spbe;

Local::Local(CFG& cfg, 
			 const Type* type, 
			 uint32_t align, 
			 const std::string& name, 
             Function* parent)
    : Value(PointerType::get(cfg, type)), m_alloc_type(type), m_align(align),
	  m_name(name), m_parent(parent) {

	if (parent != nullptr)
		parent->add_local(this);
}

void Local::detach_from_parent() {
	assert(m_parent && "local does not belong to a function!");
	m_parent->remove_local(this);
}

#include "../../include/ast/Decl.hpp"
#include "../../include/ast/Scope.hpp"

#include <cassert>

using namespace scc;

void Scope::add(Decl* decl) {
    assert(!contains(decl->get_name()) && 
        "symbol with name already exists in scope!");

    m_decls.emplace(decl->get_name(), decl);
}

Decl* Scope::get(const std::string& name) const {
    auto it = m_decls.find(name);
    if (it != m_decls.end())
        return it->second;

    if (has_parent())
        return m_parent->get(name);

    return nullptr;
}

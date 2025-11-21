//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SCC_SCOPE_H_
#define SCC_SCOPE_H_

//
// This header file declares the Scope class - a tree representation of scoped
// declarations in a program.
//

#include <memory>
#include <string>
#include <unordered_map>

namespace scc {

class Decl;

class Scope final {
public:
    using SymbolTable = std::unordered_map<std::string, Decl*>;

private:
    /// The parent node to this scope, if it exists.
    std::shared_ptr<Scope> m_parent = nullptr;

    /// The map <name, symbol> of named declarations defined in this scope.
    /// This does not include symbols accessible in this scope but defined in
    /// parent nodes. Those can be accessed by traversing the tree upwards.
    SymbolTable m_decls = {};

public:
    /// Create a new scope with an optional parent node \p parent.
    Scope(const std::shared_ptr<Scope>& parent = nullptr) : m_parent(parent) {}

    Scope(const Scope&) = delete;
    Scope& operator = (const Scope&) = delete;

    ~Scope() = default;

    /// Returns the parent scope of this scope, if it exists.
    const std::shared_ptr<Scope>& get_parent() const { return m_parent; }
    std::shared_ptr<Scope>& get_parent() { return m_parent; }

    /// Returns true if this scope has a parent scope.
    bool has_parent() const { return m_parent != nullptr; }

    /// Returns the declarations in this scope.
    const SymbolTable& get_decls() const { return m_decls; }

    /// Add a new declaration \p decl to this scope. This function will fails 
    /// if a symbol with the same name as \p decl already exists in this scope.
    void add(Decl* decl);

    /// Returns the declaration with named by \p name if it exists in this
    /// scope or any parent scope. Returns `nullptr` if a symbol couldn't be
    /// found.
    Decl* get(const std::string& name) const;

    /// Returns true if a declaration named by \p name exists in this scope.
    bool contains(const std::string& name) const {
        return get(name) != nullptr;
    }
};

} // namespace scc

#endif // SCC_SCOPE_H_

//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef STATIM_SCOPE_H_
#define STATIM_SCOPE_H_

#include <string>
#include <unordered_map>
#include <vector>

namespace stm {

using std::string;
using std::unordered_map;
using std::vector;

class NamedDecl;

/// Represents a node in a greater scope tree.
///
/// Scopes make up a tree-like structure that is linked to the corresponding
/// syntax tree, and contain a set of named symbols.
class Scope final {
public:
    using SymbolTable = unordered_map<string, const NamedDecl*>;

private:
    Scope* m_parent;
    SymbolTable m_symbols = {};

public:
    Scope(Scope* parent = nullptr) : m_parent(parent) {}

    ~Scope() = default;

    Scope(const Scope&) = delete;
    void operator=(const Scope&) = delete;

    Scope(Scope&&) noexcept = delete;
    void operator=(Scope&&) noexcept = delete;

    bool has_parent() const { return m_parent != nullptr; }

    Scope* get_parent() const { return m_parent; }

    /// Add a new symbol \p decl to this scope. 
    /// 
    /// If it conflicts name-wise with another symbol, then this function 
    /// returns false. Otherwise, it returns true.
    bool add(const NamedDecl* decl);

    /// Returns the symbol in this scope named by \p name, and null if one 
    /// does not exist with the name.
    const NamedDecl* get(const string& name) const;
};

} // namespace stm

#endif // STATIM_SCOPE_H_

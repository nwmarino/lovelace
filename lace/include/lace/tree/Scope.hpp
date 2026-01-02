//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_SCOPE_H_
#define LOVELACE_SCOPE_H_

//
//  This header file defines the Scope class, which is a data structure created
//  at parse time to organize which named definitions are visible at different
//  points in the program. The scope trees work in a lexical manner, and are
//  used during syntax tree analysis passes.
//

#include <string>
#include <unordered_map>

namespace lace {

class NamedDefn;

/// Represents a node in a greater scope tree.
///
/// Scopes make up a tree-like structure that is linked to the corresponding
/// syntax tree, and contain a set of named symbols.
class Scope final {
public:
    using DefnTable = std::unordered_map<std::string, NamedDefn*>;

private:
    Scope* m_parent;
    DefnTable m_defns = {};

public:
    Scope(Scope* parent = nullptr) : m_parent(parent) {}

    ~Scope() = default;

    Scope(const Scope&) = delete;
    void operator=(const Scope&) = delete;

    Scope(Scope&&) noexcept = delete;
    void operator=(Scope&&) noexcept = delete;

    Scope* get_parent() const { return m_parent; }

    /// Test if this scope has a parent scope.
    bool has_parent() const { return m_parent != nullptr; }

    /// Add the given |defn| to this scope. 
    /// 
    /// If it conflicts name-wise with another definition, then this routine 
    /// returns false. Otherwise, it returns true.
    bool add(NamedDefn* defn);

    /// Returns the definition in this scope with the given |name|, and null 
    /// if one does not exist.
    NamedDefn* get(const std::string& name) const;
};

} // namespace lace

#endif // LOVELACE_SCOPE_H_

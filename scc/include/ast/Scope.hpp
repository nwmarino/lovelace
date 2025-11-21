#ifndef SCC_SCOPE_H_
#define SCC_SCOPE_H_

#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>

namespace scc {

class Decl;

class Scope final {
    std::shared_ptr<Scope> m_parent = nullptr;
    std::unordered_map<std::string, Decl*> m_decls = {};

public:
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
    const std::unordered_map<std::string, Decl*> get_decls() const { return m_decls; }
    std::unordered_map<std::string, Decl*> get_decls() { return m_decls; }

    /// Add a new declaration |decl| to this scope. Fails if a symbol with the
    /// same name as |decl| already exists in this scope.
    void add(Decl* decl);

    /// Returns the declaration with the given name if it exists in this scope,
    /// and `nullptr` otherwise.
    Decl* get(const std::string& name) const;

    /// Returns true if a declaration with the given name exists in this scope.
    bool contains(const std::string& name) const {
        return get(name) != nullptr;
    }
};

} // namespace scc

#endif // SCC_SCOPE_H_

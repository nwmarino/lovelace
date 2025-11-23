//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SCC_TRANSLATION_UNIT_H_
#define SCC_TRANSLATION_UNIT_H_

//
// This header file declares the important TranslationUnit structure that
// organizes fields describing an input file to the compiler.
//
// A single translation unit represents a source file, and also its syntax
// tree prior to code generation.
//

#include "ast/Context.hpp"
#include "ast/Decl.hpp"

#include <cstdint>
#include <memory>
#include <ostream>
#include <vector>

namespace scc {

class TranslationUnit final {
    friend class Parser;

    /// The type context of this translation unit's syntax tree.
    Context m_context = {};

    /// The global scope of this translation unit.
    std::unique_ptr<Scope> m_scope = nullptr;
    
    /// The top-level declarations of this translation unit.
    std::vector<std::unique_ptr<Decl>> m_decls = {};

public:
    TranslationUnit() = default;

    TranslationUnit(const TranslationUnit&) = delete;
    TranslationUnit& operator = (const TranslationUnit&) = delete;

    ~TranslationUnit() = default;

    /// Returns the global scope of this translation unit.
    const Scope* get_scope() const { return m_scope.get(); }
    Scope* get_scope() { return m_scope.get(); }

    /// Returns the number of top-level declarations in this translation unit.
    uint32_t num_decls() const { return m_decls.size(); }

    /// Returns true if this translation unit carries any top-level declarations.
    bool has_decls() const { return !m_decls.empty(); }

    /// Returns the top-level declaration at position \p i of this translation
    /// unit.
    const Decl* get_decl(uint32_t i) const {
        assert(i < num_decls() && "index out of bounds!");
        return m_decls[i].get();
    }

    Decl* get_decl(uint32_t i) {
        return const_cast<Decl*>(
            static_cast<const TranslationUnit*>(this)->get_decl(i));
    }

    /// Returns the top-level declaration named by \p name if it exists, and
    /// 'nullptr' otherwise.
    const Decl* get_decl(const std::string& name) const {
        for (const auto& decl : m_decls)
            if (decl->name() == name) return decl.get();

        return nullptr;
    }

    Decl* get_decl(const std::string& name) {
        return const_cast<Decl*>(
            static_cast<const TranslationUnit*>(this)->get_decl(name));
    }

    void print(std::ostream& os) const;
};

} // namespace scc

#endif // SCC_TRANSLATION_UNIT_H_

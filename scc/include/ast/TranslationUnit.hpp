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

#include <memory>
#include <vector>

namespace scc {

class TranslationUnit final {
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
};

} // namespace scc

#endif // SCC_TRANSLATION_UNIT_H_

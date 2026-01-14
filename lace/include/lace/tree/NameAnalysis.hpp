//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_NAME_ANALYSIS_H_
#define LOVELACE_NAME_ANALYSIS_H_

#include "lace/core/Options.hpp"
#include "lace/tree/Scope.hpp"
#include "lace/tree/Type.hpp"
#include "lace/tree/Visitor.hpp"

namespace lace {

class NameAnalysis final : public Visitor {
    const Options& m_options;
    
    AST* m_ast = nullptr;
    AST::Context* m_context = nullptr;
    const Scope* m_scope = nullptr;

    /// Replace all deferred types composed in |type| and return the new,
    /// fully resolved type. If a part could not be resolved, then null is
    /// returned.
    bool resolve_type(const QualType& type) const;

public:
    NameAnalysis(const Options& options);

    void visit(AST& ast) override;

    void visit(VariableDefn& node) override;
    void visit(FunctionDefn& node) override;
    void visit(FieldDefn& node) override;
    void visit(VariantDefn& node) override;
    void visit(StructDefn& node) override;
    void visit(EnumDefn& node) override;
};

} // namespace lace

#endif // LOVELACE_NAME_ANALYSIS_H_

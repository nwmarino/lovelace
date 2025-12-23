//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/tree/SymbolAnalysis.hpp"
#include "stmc/tree/Decl.hpp"
#include "stmc/tree/Expr.hpp"
#include "stmc/tree/Stmt.hpp"
#include "stmc/tree/Type.hpp"
#include "stmc/types/SourceSpan.hpp"

using namespace stm;

SymbolAnalysis::SymbolAnalysis(Diagnostics& diags, Options& options)
    : m_diags(diags), m_options(options) {}

void SymbolAnalysis::visit(TranslationUnitDecl& node) {
    m_diags.set_path(node.get_file());

    m_scope = node.get_scope();

    for (const auto& [name, type] : node.get_context().m_unresolved) {
        const SourceSpan span = type->get_location();

        const NamedDecl* ND = m_scope->get(name);
        if (!ND)
            m_diags.fatal("unresolved type: '" + name + "'", span);

        const TypeDecl* TD = dynamic_cast<const TypeDecl*>(ND);
        if (!TD)
            m_diags.fatal("invalid type: '" + name + "'", span);

        type->set_underlying(TD->get_type());
    }

    for (uint32_t i = 0, e = node.num_decls(); i < e; ++i)
        node.get_decl(i)->accept(*this);

    m_scope = m_scope->get_parent();
}

void SymbolAnalysis::visit(VariableDecl& node) {
    if (node.has_init())
        node.get_init()->accept(*this);
}

void SymbolAnalysis::visit(FunctionDecl& node) {
    if (!node.has_body())
        return;

    m_scope = node.get_scope();
    node.get_body()->accept(*this);
    m_scope = m_scope->get_parent();
}

void SymbolAnalysis::visit(AsmStmt& node) {
    for (uint32_t i = 0, e = node.num_args(); i < e; ++i)
        node.get_arg(i)->accept(*this);
}

void SymbolAnalysis::visit(BlockStmt& node) {
    m_scope = node.get_scope();

    for (uint32_t i = 0, e = node.num_stmts(); i < e; ++i)
        node.get_stmt(i)->accept(*this);

    m_scope = m_scope->get_parent();
}

void SymbolAnalysis::visit(DeclStmt& node) {
    node.get_decl()->accept(*this);
}

void SymbolAnalysis::visit(RetStmt& node) {
    if (node.has_expr())
        node.get_expr()->accept(*this);
}

void SymbolAnalysis::visit(IfStmt& node) {
    node.get_cond()->accept(*this);
    node.get_then()->accept(*this);

    if (node.has_else())
        node.get_else()->accept(*this);
}

void SymbolAnalysis::visit(WhileStmt& node) {
    node.get_cond()->accept(*this);

    if (node.has_body())
        node.get_body()->accept(*this);
}

void SymbolAnalysis::visit(BinaryOp& node) {
    node.get_lhs()->accept(*this);
    node.get_rhs()->accept(*this);
}

void SymbolAnalysis::visit(UnaryOp& node) {
    node.get_expr()->accept(*this);
}

void SymbolAnalysis::visit(CastExpr& node) {
    node.get_expr()->accept(*this);
}

void SymbolAnalysis::visit(ParenExpr& node) {
    node.get_expr()->accept(*this);
}

void SymbolAnalysis::visit(AccessExpr& node) {
    const SourceSpan span = node.get_span();
    const string& name = node.get_name();

    node.get_base()->accept(*this);

    // Check that the base type is a struct.
    TypeUse& base_type = node.get_base()->get_type();
    if (base_type->is_pointer())
        base_type = static_cast<const PointerType*>(base_type.get_type())->get_pointee();

    if (auto UNR = dynamic_cast<const UnresolvedType*>(base_type.get_type())) {
        base_type = UNR->get_underlying();
        assert(base_type);
    }

    if (!base_type->is_struct())
        m_diags.fatal("'.' base must be a struct or a pointer to one", span);

    // Resolve the struct declaration from the base type.
    const StructDecl* struct_decl = static_cast<const StructType*>(
        base_type.get_type())->get_decl();

    // Resolve the target field from the struct declaration.
    const FieldDecl* field = struct_decl->get_field(name);
    if (!field)
        m_diags.fatal("field '" + name + "' does not exist", span);

    node.set_field(field);
}

void SymbolAnalysis::visit(SubscriptExpr& node) {
    node.get_base()->accept(*this);
    node.get_index()->accept(*this);
}

void SymbolAnalysis::visit(DeclRefExpr& node) {
    const SourceSpan span = node.get_span();
    const string& name = node.get_name();

    const NamedDecl* ND = m_scope->get(name);
    if (!ND)
        m_diags.fatal("unresolved reference: '" + name + "'", span);

    const ValueDecl* VD = dynamic_cast<const ValueDecl*>(ND);
    if (!VD)
        m_diags.fatal("invalid reference: '" + name + "'", span);

    node.set_decl(VD);
    node.set_type(VD->get_type());
}

void SymbolAnalysis::visit(CallExpr& node) {
    node.get_callee()->accept(*this);

    for (uint32_t i = 0, e = node.num_args(); i < e; ++i)
        node.get_arg(i)->accept(*this);
}

//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lace/core/Diagnostics.hpp"
#include "lace/tree/AST.hpp"
#include "lace/tree/Defn.hpp"
#include "lace/tree/Expr.hpp"
#include "lace/tree/Stmt.hpp"
#include "lace/tree/SymbolAnalysis.hpp"
#include "lace/tree/Type.hpp"

using namespace lace;

SymbolAnalysis::SymbolAnalysis(const Options& options) : m_options(options) {}

bool SymbolAnalysis::resolve_type(const QualType& type) const {
    switch (type->get_class()) {
        case Type::Array:
            return resolve_type(
                static_cast<const ArrayType*>(type.get_type())->get_element_type());

        case Type::Deferred: {
            NamedDefn* named_defn = m_scope->get(static_cast<const DeferredType*>(
                type.get_type())->get_name());
            if (!named_defn)
                return false;

            TypeDefn* type_defn = dynamic_cast<TypeDefn*>(named_defn);
            if (!type_defn)
                return false;

            type.set_type(type_defn->get_type());
            return true;
        }

        case Type::Enum:
            return resolve_type(
                static_cast<const EnumType*>(type.get_type())->get_underlying());

        case Type::Function: {
            const FunctionType* func_type = static_cast<const FunctionType*>(
                type.get_type());

            if (!resolve_type(func_type->get_return_type()))
                return false;

            for (auto& param : func_type->get_params())
                if (!resolve_type(param))
                    return false;

            return true;
        }

        case Type::Pointer:
            return resolve_type(
                static_cast<const PointerType*>(type.get_type())->get_pointee());

        default:
            return true;
    }
}

void SymbolAnalysis::visit(AST& ast) {
    m_ast = &ast;
    m_context = &ast.get_context();
    m_scope = ast.get_scope();

    for (Defn* defn : ast.get_defns())
        defn->accept(*this);
}

void SymbolAnalysis::visit(VariableDefn& node) {
    if (!resolve_type(node.get_type()))
        log::error("unresolved type: " + node.get_type().to_string(), 
            log::Span(m_ast->get_file(), node.get_span()));
            
    if (node.has_init())
        node.get_init()->accept(*this);
}

void SymbolAnalysis::visit(FunctionDefn& node) {
    m_scope = node.get_scope();

    if (node.has_body())
        node.get_body()->accept(*this);

    m_scope = m_scope->get_parent();
}

void SymbolAnalysis::visit(StructDefn& node) {

}

/*
void SymbolAnalysis::visit(AsmStmt& node) {
    for (uint32_t i = 0, e = node.num_args(); i < e; ++i)
        node.get_arg(i)->accept(*this);
}
*/

void SymbolAnalysis::visit(AdapterStmt& node) {
    switch (node.get_flavor()) {
    case AdapterStmt::Definitive:
        node.get_defn()->accept(*this);
        break;

    case AdapterStmt::Expressive:
        node.get_expr()->accept(*this);
        break;
    }
}

void SymbolAnalysis::visit(BlockStmt& node) {
    m_scope = node.get_scope();

    for (Stmt* stmt : node.get_stmts())
        stmt->accept(*this);

    m_scope = m_scope->get_parent();
}

void SymbolAnalysis::visit(IfStmt& node) {
    node.get_cond()->accept(*this);
    node.get_then()->accept(*this);

    if (node.has_else())
        node.get_else()->accept(*this);
}

void SymbolAnalysis::visit(RetStmt& node) {
    if (node.has_expr())
        node.get_expr()->accept(*this);
}

void SymbolAnalysis::visit(UntilStmt& node) {
    node.get_cond()->accept(*this);

    if (node.has_body())
        node.get_body()->accept(*this);
}

void SymbolAnalysis::visit(RuneStmt& node) {
    return; // Nothing to do (yet).
}

void SymbolAnalysis::visit(BinaryOp& node) {
    node.get_lhs()->accept(*this);
    node.get_rhs()->accept(*this);
}

void SymbolAnalysis::visit(UnaryOp& node) {
    node.get_expr()->accept(*this);
}

void SymbolAnalysis::visit(AccessExpr& node) {
    const log::Span span = log::Span(m_ast->get_file(), node.get_span());
    const std::string& name = node.get_name();

    node.get_base()->accept(*this);

    // Check that the base type is a struct.
    QualType base_type = node.get_base()->get_type();
    if (base_type->is_pointer())
        base_type = static_cast<const PointerType*>(base_type.get_type())->get_pointee();

    if (!base_type->is_struct())
        log::fatal("'.' base must be a struct or a pointer to one", span);

    // Resolve the struct definition from the base type.
    const StructDefn* struct_defn = static_cast<const StructType*>(
        base_type.get_type())->get_defn();

    // Resolve the target field from the struct definition.
    const FieldDefn* field = struct_defn->get_field(name);
    if (!field)
        log::fatal("field '" + name + "' does not exist", span);

    node.set_field(field);
    node.set_type(field->get_type());
}

void SymbolAnalysis::visit(CallExpr& node) {
    node.get_callee()->accept(*this);

    for (Expr* arg : node.get_args())
        arg->accept(*this);

    // @Todo: maybe propogate function return type here.
}

void SymbolAnalysis::visit(CastExpr& node) {
    node.get_expr()->accept(*this);

    if (!resolve_type(node.get_type()))
        log::fatal("unresolved type: " + node.get_type().to_string(), 
            log::Span(m_ast->get_file(), node.get_span()));
}

void SymbolAnalysis::visit(ParenExpr& node) {
    node.get_expr()->accept(*this);
}

void SymbolAnalysis::visit(RefExpr& node) {
    const log::Span span = log::Span(m_ast->get_file(), node.get_span());
    const std::string& name = node.get_name();

    NamedDefn* named_defn = m_scope->get(name);
    if (!named_defn)
        log::fatal("unresolved reference: " + name, span);

    ValueDefn* value_defn = dynamic_cast<ValueDefn*>(named_defn);
    if (!value_defn)
        log::fatal("invalid reference: " + name, span);

    node.set_defn(value_defn);
    node.set_type(value_defn->get_type());
}

void SymbolAnalysis::visit(SizeofExpr& node) {
    if (!resolve_type(node.get_target_type()))
        log::fatal("unresolved type: " + node.get_target_type().to_string(), 
            log::Span(m_ast->get_file(), node.get_span()));
}

void SymbolAnalysis::visit(SubscriptExpr& node) {
    node.get_base()->accept(*this);
    node.get_index()->accept(*this);
}

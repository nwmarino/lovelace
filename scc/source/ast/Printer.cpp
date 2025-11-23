//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "ast/Decl.hpp"
#include "ast/Expr.hpp"
#include "ast/Stmt.hpp"
#include "core/TranslationUnit.hpp"

#include <cstdint>
#include <ostream>
#include <string>

using namespace scc;

/// The current printer indentation level.
static uint32_t g_indent = 0;

/// Print the indentation level \c g_indent to the output stream \p os.
static void print_indent(std::ostream& os) {
    os << std::string(g_indent * 2, ' ');
}

/// Stringify \p span and print it to the output stream \p os.
static void print_span(std::ostream& os, const Span& span) {
    os << '<' << std::to_string(span.begin.line) << ':' 
       << std::to_string(span.begin.column) << '/' 
       << std::to_string(span.end.line) + ':'
       << std::to_string(span.end.column) + '>';
}

void TranslationUnit::print(std::ostream& os) const {
    for (const auto& decl : m_decls) decl->print(os);
}

void VariableDecl::print(std::ostream& os) const {
    print_indent(os);
    os << "Variable ";
    print_span(os, m_span);
    os << ' ';
    
    switch (m_storage) {
    case StorageClass::None:
        break;
    case StorageClass::Auto:
        os << "auto ";
        break;
    case StorageClass::Register:
        os << "register ";
        break;
    case StorageClass::Static:
        os << "static ";
        break;
    case StorageClass::Extern:
        os << "extern ";
        break;
    }

    os << m_name << " '" << m_type.to_string() << "'\n";

    if (has_initializer()) {
        ++g_indent;
        m_init->print(os);
        --g_indent;
    }
}

void ParameterDecl::print(std::ostream& os) const {
    print_indent(os);
    os << "Parameter ";
    print_span(os, m_span);
    os << ' ' << m_name << " '" << m_type.to_string() << "'\n";
}

void FunctionDecl::print(std::ostream& os) const {
    print_indent(os);
    os << "Function ";
    print_span(os, m_span);
    os << ' ';
    
    switch (m_storage) {
    case StorageClass::None:
        break;
    case StorageClass::Auto:
        os << "auto ";
        break;
    case StorageClass::Register:
        os << "register ";
        break;
    case StorageClass::Static:
        os << "static ";
        break;
    case StorageClass::Extern:
        os << "extern ";
        break;
    }

    os << m_name << " '" << m_type.to_string() << "'\n";

    if (has_params() || has_body()) {
        ++g_indent;
        for (const auto& param : m_params) param->print(os);
        if (has_body()) m_body->print(os);
        --g_indent;
    }
}

void CompoundStmt::print(std::ostream& os) const {
    print_indent(os);
    os << "Compound ";
    print_span(os, m_span);
    os << '\n';

    if (has_stmts()) {
        ++g_indent;
        for (const auto& stmt : m_stmts) stmt->print(os);
        --g_indent;
    }
}

void DeclStmt::print(std::ostream& os) const {
    m_decl->print(os);
}

void ExprStmt::print(std::ostream& os) const {
    m_expr->print(os);
}

void IfStmt::print(std::ostream& os) const {
    print_indent(os);
    os << "If ";
    print_span(os, m_span);
    os << '\n';

    ++g_indent;
    m_cond->print(os);
    m_then->print(os);
    if (has_else()) m_else->print(os);
    --g_indent;
}

void ReturnStmt::print(std::ostream& os) const {
    print_indent(os);
    os << "Return ";
    print_span(os, m_span);

    if (has_expr()) {
        os << '\n';

        ++g_indent;
        m_expr->print(os);
        --g_indent;
    } else {
        os << " void\n";
    }
}

void BreakStmt::print(std::ostream& os) const {
    print_indent(os);
    os << "Break ";
    print_span(os, m_span);
    os << '\n';
}

void ContinueStmt::print(std::ostream& os) const {
    print_indent(os);
    os << "Continue ";
    print_span(os, m_span);
    os << '\n';
}

void WhileStmt::print(std::ostream& os) const {
    print_indent(os);
    os << "While ";
    print_span(os, m_span);
    os << '\n';

    ++g_indent;
    m_cond->print(os);
    if (has_body()) m_body->print(os);
    --g_indent;
}

void ForStmt::print(std::ostream& os) const {
    print_indent(os);
    os << "For ";
    print_span(os, m_span);
    os << '\n';

    ++g_indent;
    if (has_init()) m_init->print(os);
    if (has_cond()) m_cond->print(os);
    if (has_step()) m_step->print(os);
    if (has_body()) m_body->print(os);
    --g_indent;
}

void IntegerLiteral::print(std::ostream& os) const {
    print_indent(os);
    os << "Integer ";
    print_span(os, m_span);
    os << ' ' << m_value << " '" << m_type.to_string() << "'\n";
}

void FPLiteral::print(std::ostream& os) const {
    print_indent(os);
    os << "Float ";
    print_span(os, m_span);
    os << ' ' << m_value << " '" << m_type.to_string() << "'\n";
}

void CharLiteral::print(std::ostream& os) const {
    print_indent(os);
    os << "Character ";
    print_span(os, m_span);
    os << ' ' << m_value << " '" << m_type.to_string() << "'\n"; 
}

void StringLiteral::print(std::ostream& os) const {
    print_indent(os);
    os << "String ";
    print_span(os, m_span);
    os << ' ' << m_value << " '" << m_type.to_string() << "'\n";
}

void BinaryExpr::print(std::ostream& os) const {
    print_indent(os);
    os << "Binop ";
    print_span(os, m_span);
    os << ' ' << to_string(m_operator) << " '" << m_type.to_string() << "'\n";

    ++g_indent;
    m_left->print(os);
    m_right->print(os);
    --g_indent;
}

void UnaryExpr::print(std::ostream& os) const {
    print_indent(os);
    os << "Unop ";
    print_span(os, m_span);
    os << ' ' << to_string(m_operator) << ' ' << (m_postfix ? "postfix" : "prefix") 
       << " '" << m_type.to_string() << "'\n";

    ++g_indent;
    m_expr->print(os);
    --g_indent;
}

void ParenExpr::print(std::ostream& os) const {
    print_indent(os);
    os << "Parentheses ";
    print_span(os, m_span);
    os << " '" << m_type.to_string() << "'\n";

    ++g_indent;
    m_expr->print(os);
    --g_indent;
}

void RefExpr::print(std::ostream& os) const {
    print_indent(os);
    os << "Ref ";
    print_span(os, m_span);
    os << ' ' << get_name() << " '" << m_type.to_string() << "'\n";
}

void CallExpr::print(std::ostream& os) const {
    print_indent(os);
    os << "Call ";
    print_span(os, m_span);
    os << " '" << m_type.to_string() << "'\n";

    ++g_indent;
    m_callee->print(os);
    for (const auto& arg : m_args) arg->print(os);
    --g_indent;
}

void CastExpr::print(std::ostream& os) const {
    print_indent(os);
    os << "Cast ";
    print_span(os, m_span);
    os << " '" << m_type.to_string() << "'\n";

    ++g_indent;
    m_expr->print(os);
    --g_indent;
}

void SizeofExpr::print(std::ostream& os) const {
    print_indent(os);
    os << "Sizeof ";
    print_span(os, m_span);
    os << ' ' << m_target.to_string() << " '" << m_type.to_string() << "'\n";
}

void SubscriptExpr::print(std::ostream& os) const {
    print_indent(os);
    os << "Subscript ";
    print_span(os, m_span);
    os << " '" << m_type.to_string() << "'\n";

    ++g_indent;
    m_base->print(os);
    m_index->print(os);
    --g_indent;
}

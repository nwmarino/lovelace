//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "ast/Decl.hpp"
#include "ast/Expr.hpp"
#include "ast/Stmt.hpp"

#include <cstdint>
#include <ostream>
#include <string>

using namespace scc;

/// The current printer indentation level.
static uint32_t g_indent = 0;

/// Print the indentation level \c g_indent to the output stream \p os.
static void print_indent(ostream& os) {
    os << string(g_indent * 2, ' ');
}

/// Stringify \p span and print it to the output stream \p os.
static void print_span(ostream& os, const SourceSpan& span) {
    os << '<' << std::to_string(span.start.line) << ':' 
       << std::to_string(span.start.column) << '/' 
       << std::to_string(span.end.line) + ':'
       << std::to_string(span.end.column) + '>';
}

void TranslationUnitDecl::print(ostream& os) const {
    for (const auto& decl : m_decls) 
        decl->print(os);
}

void VariableDecl::print(ostream& os) const {
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

    if (has_init()) {
        ++g_indent;
        
        m_init->print(os);

        --g_indent;
    }
}

void ParameterDecl::print(ostream& os) const {
    print_indent(os);
    os << "Parameter ";
    print_span(os, m_span);
    os << ' ' << m_name << " '" << m_type.to_string() << "'\n";
}

void FunctionDecl::print(ostream& os) const {
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

        for (const auto& param : m_params) 
            param->print(os);

        if (has_body()) 
            m_body->print(os);
        
        --g_indent;
    }
}

void TypedefDecl::print(ostream& os) const {
    print_indent(os);
    os << "Typedef ";
    print_span(os, m_span);
    os << ' ' << m_name << " '" << m_type->to_string() << "'\n";
}

void FieldDecl::print(ostream& os) const {
    print_indent(os);
    os << "Field ";
    print_span(os, m_span);
    os << ' ' << m_name << " '" << m_type->to_string() << "'\n";
}

void RecordDecl::print(ostream& os) const {
    print_indent(os);
    os << "Record ";
    print_span(os, m_span);
    os << ' ' << (is_struct() ? "struct " : "union ") << m_name << " '" 
       << m_type->to_string() << "'\n";

    ++g_indent;
    
    for (const auto& decl : m_decls) 
        decl->print(os);

    --g_indent;
}

void EnumVariantDecl::print(ostream& os) const {
    print_indent(os);
    os << "Variant ";
    print_span(os, m_span);
    os << ' ' << m_name << " '" << m_type.to_string() << "'\n";
}

void EnumDecl::print(ostream& os) const {
    print_indent(os);
    os << "Enum ";
    print_span(os, m_span);
    os << ' ' << m_name << " '" << m_type->to_string() << "'\n";

    ++g_indent;
    
    for (const auto& decl : m_decls) 
        decl->print(os);

    --g_indent;
}

void CompoundStmt::print(ostream& os) const {
    print_indent(os);
    os << "Compound ";
    print_span(os, m_span);
    os << '\n';

    if (!empty()) {
        ++g_indent;
        
        for (const auto& stmt : m_stmts) 
            stmt->print(os);
        
        --g_indent;
    }
}

void DeclStmt::print(ostream& os) const {
    for (const auto& decl : m_decls)
        decl->print(os);
}

void ExprStmt::print(ostream& os) const {
    m_expr->print(os);
}

void IfStmt::print(ostream& os) const {
    print_indent(os);
    os << "If ";
    print_span(os, m_span);
    os << '\n';

    ++g_indent;
    
    m_cond->print(os);
    m_then->print(os);
    if (has_else()) 
        m_else->print(os);
    
    --g_indent;
}

void ReturnStmt::print(ostream& os) const {
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

void BreakStmt::print(ostream& os) const {
    print_indent(os);
    os << "Break ";
    print_span(os, m_span);
    os << '\n';
}

void ContinueStmt::print(ostream& os) const {
    print_indent(os);
    os << "Continue ";
    print_span(os, m_span);
    os << '\n';
}

void WhileStmt::print(ostream& os) const {
    print_indent(os);
    os << "While ";
    print_span(os, m_span);
    os << '\n';

    ++g_indent;
    
    m_cond->print(os);
    if (has_body()) 
        m_body->print(os);
    
    --g_indent;
}

void ForStmt::print(ostream& os) const {
    print_indent(os);
    os << "For ";
    print_span(os, m_span);
    os << '\n';

    ++g_indent;
    
    if (has_init()) 
        m_init->print(os);
    if (has_cond()) 
        m_cond->print(os);
    if (has_step()) 
        m_step->print(os);
    if (has_body()) 
        m_body->print(os);
    
    --g_indent;
}

void CaseStmt::print(ostream& os) const {
    print_indent(os);
    os << "Case ";
    print_span(os, m_span);
    os << '\n';

    ++g_indent;

    m_match->print(os);
    m_body->print(os);

    --g_indent;
}

void SwitchStmt::print(ostream& os) const {
    print_indent(os);
    os << "Switch ";
    print_span(os, m_span);
    os << '\n';

    ++g_indent;
    
    for (const auto& c : m_cases) 
        c->print(os);

    if (has_default()) 
        m_default->print(os);
    
    --g_indent;
}

void IntegerLiteral::print(ostream& os) const {
    print_indent(os);
    os << "Integer ";
    print_span(os, m_span);
    os << ' ' << m_value << " '" << m_type.to_string() << "'\n";
}

void FPLiteral::print(ostream& os) const {
    print_indent(os);
    os << "Float ";
    print_span(os, m_span);
    os << ' ' << m_value << " '" << m_type.to_string() << "'\n";
}

void CharLiteral::print(ostream& os) const {
    print_indent(os);
    os << "Character ";
    print_span(os, m_span);
    os << ' ' << m_value << " '" << m_type.to_string() << "'\n"; 
}

void StringLiteral::print(ostream& os) const {
    print_indent(os);
    os << "String ";
    print_span(os, m_span);
    os << ' ' << m_value << " '" << m_type.to_string() << "'\n";
}

void BinaryExpr::print(ostream& os) const {
    print_indent(os);
    os << "Binop ";
    print_span(os, m_span);
    os << ' ' << to_string(m_operator) << " '" << m_type.to_string() << "'\n";

    ++g_indent;

    m_left->print(os);
    m_right->print(os);

    --g_indent;
}

void UnaryExpr::print(ostream& os) const {
    print_indent(os);
    os << "Unop ";
    print_span(os, m_span);
    os << ' ' << to_string(m_operator) << ' ' << (m_postfix ? "postfix" : "prefix") 
       << " '" << m_type.to_string() << "'\n";

    ++g_indent;

    m_expr->print(os);

    --g_indent;
}

void ParenExpr::print(ostream& os) const {
    print_indent(os);
    os << "Parentheses ";
    print_span(os, m_span);
    os << " '" << m_type.to_string() << "'\n";

    ++g_indent;

    m_expr->print(os);

    --g_indent;
}

void RefExpr::print(ostream& os) const {
    print_indent(os);
    os << "Ref ";
    print_span(os, m_span);
    os << ' ' << get_name() << " '" << m_type.to_string() << "'\n";
}

void CallExpr::print(ostream& os) const {
    print_indent(os);
    os << "Call ";
    print_span(os, m_span);
    os << " '" << m_type.to_string() << "'\n";

    ++g_indent;

    m_callee->print(os);
    for (const auto& arg : m_args) 
        arg->print(os);

    --g_indent;
}

void CastExpr::print(ostream& os) const {
    print_indent(os);
    os << "Cast ";
    print_span(os, m_span);
    os << " '" << m_type.to_string() << "'\n";

    ++g_indent;

    m_expr->print(os);

    --g_indent;
}

void SizeofExpr::print(ostream& os) const {
    print_indent(os);
    os << "Sizeof ";
    print_span(os, m_span);
    os << ' ' << m_target.to_string() << " '" << m_type.to_string() << "'\n";
}

void SubscriptExpr::print(ostream& os) const {
    print_indent(os);
    os << "Subscript ";
    print_span(os, m_span);
    os << " '" << m_type.to_string() << "'\n";

    ++g_indent;

    m_base->print(os);
    m_index->print(os);

    --g_indent;
}

void MemberExpr::print(ostream& os) const {
    print_indent(os);
    os << "Member ";
    print_span(os, m_span);
    os << (m_arrow ? "->" : ".") << m_member->get_name() << " '" 
       << m_type.to_string() << "'\n";

    ++g_indent;

    m_base->print(os);

    --g_indent;
}

void TernaryExpr::print(ostream& os) const {
    print_indent(os);
    os << "Ternary ";
    print_span(os, m_span);
    os << " '" << m_type.to_string() << "'\n";

    ++g_indent;

    m_cond->print(os);
    m_tval->print(os);
    m_fval->print(os);

    --g_indent;
}

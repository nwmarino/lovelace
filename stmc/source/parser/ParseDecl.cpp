//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/parser/Parser.hpp"
#include "stmc/tree/Decl.hpp"
#include "stmc/tree/Scope.hpp"
#include "stmc/tree/Type.hpp"
#include "stmc/types/SourceLocation.hpp"

using namespace stm;

Decl* Parser::parse_initial_declaration() {
    if (!match(Token::Identifier))
        m_diags.fatal("expected identifier", loc());

    if (expect("load"))
        return parse_load_declaration();

    const Token name = last();
    next();

    if (expect(Token::Path))
        return parse_binding_declaration(name);

    return nullptr;
}

Decl* Parser::parse_binding_declaration(const Token name) {
    if (expect(Token::SetParen)) {
        Scope* scope = enter_scope();

        vector<ParameterDecl*> params = {};
        params.reserve(2);

        while (!expect(Token::EndParen)) {
            if (!match(Token::Identifier))
                m_diags.fatal("expected parameter name", loc());

            const SourceLocation param_start = loc();
            string param_name = last().value;
            next();

            if (!expect(Token::Colon))
                m_diags.fatal("expected parameter type", loc());

            TypeUse param_type = parse_type();

            ParameterDecl* param = ParameterDecl::create(
                *m_context, since(param_start), param_name, param_type);

            m_scope->add(param);
            params.push_back(param);

            if (expect(Token::EndParen))
                break;

            if (!expect(Token::Comma))
                m_diags.fatal("expected ','", loc());
        }

        params.shrink_to_fit();

        if (!expect(Token::Arrow))
            m_diags.fatal("expected '->' after parameter list", loc());

        TypeUse ret_type = parse_type();

        BlockStmt* body = nullptr;
        SourceLocation end = loc();
        if (match(Token::SetBrace)) {
            body = static_cast<BlockStmt*>(parse_block_statement());
            end = body->get_span().end;
        } else if (!expect(Token::Semi)) {
            m_diags.fatal("expected function body", loc());
        }

        exit_scope();

        vector<TypeUse> param_types = {};
        param_types.reserve(params.size());
        for (uint32_t i = 0, e = params.size(); i != e; ++i)
            param_types.push_back(params[i]->get_type());

        FunctionDecl* FN = FunctionDecl::create(
            *m_context, 
            SourceSpan(name.loc, end), 
            name.value,
            {},
            FunctionType::get(*m_context, ret_type, param_types), 
            scope, 
            params, 
            body);

        m_scope->add(FN);
        return FN;
    }

    return nullptr;
}

Decl* Parser::parse_load_declaration() {
    return nullptr;
}

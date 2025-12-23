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
    } else if (expect("struct")) {
        if (!expect(Token::SetBrace))
            m_diags.fatal("expectd '{'", since(loc()));

        vector<FieldDecl*> fields = {};
        fields.reserve(2);

        SourceLocation end = loc();
        while (!expect(Token::EndBrace)) {
            if (!match(Token::Identifier))
                m_diags.fatal("expected field name", since(loc()));

            const Token field_name = last();
            next();

            if (!expect(Token::Colon))
                m_diags.fatal("expected ':'", since(loc()));

            TypeUse field_type = parse_type();

            FieldDecl* field = FieldDecl::create(*
                m_context, 
                since(field_name.loc), 
                field_name.value, 
                {}, 
                field_type);

            fields.push_back(field);

            if (match(Token::EndBrace)) {
                end = loc();
                next(); // '}'
                break;
            }

            if (!expect(Token::Comma))
                m_diags.fatal("expected ','", since(loc()));
        }

        fields.shrink_to_fit();
        
        StructDecl* decl = StructDecl::create(
            *m_context, SourceSpan(name.loc, end), name.value, {}, nullptr);

        const StructType* type = StructType::create(*m_context, decl);
        
        decl->set_type(type);
        decl->set_fields(fields);
        
        m_scope->add(decl);
        return decl;
    } else if (expect("enum")) {
        TypeUse underlying;
        if (match(Token::Identifier)) {
            underlying = parse_type();
        } else {
            underlying = BuiltinType::get(*m_context, BuiltinType::Int64);
        }

        EnumDecl* decl = EnumDecl::create(
            *m_context, name.loc, name.value, {}, underlying);

        const EnumType* type = EnumType::create(
            *m_context, underlying, decl);

        decl->set_type(type);
        
        if (!expect(Token::SetBrace))
            m_diags.fatal("expected '{'", SourceSpan(loc()));

        vector<VariantDecl*> variants = {};
        variants.reserve(4);

        int64_t value = 0;
        SourceLocation end = loc();
        while (!expect(Token::EndBrace)) {
            if (!match(Token::Identifier))
                m_diags.fatal("expected name", SourceSpan(loc()));

            const Token variant_name = last();
            next();

            if (expect(Token::Eq)) {
                bool neg = false;

                if (expect(Token::Minus))
                    neg = true;

                if (!match(Token::Integer))
                    m_diags.fatal("expected integer", SourceSpan(loc()));

                value = std::stoll(last().value);

                if (neg)
                    value = -value;

                next();
            }

            VariantDecl* variant = VariantDecl::create(
                *m_context, 
                since(variant_name.loc), 
                variant_name.value, 
                {}, 
                type, 
                value++);

            m_scope->add(variant);
            variants.push_back(variant);

            if (match(Token::EndBrace)) {
                end = loc();
                next(); // '}'
                break;
            }

            if (!expect(Token::Comma))
                m_diags.fatal("expected ','", SourceSpan(loc()));
        }

        variants.shrink_to_fit();

        decl->set_variants(variants);
        m_scope->add(decl);
        return decl;
    } else {
        TypeUse type = parse_type();

        Expr* init = nullptr;
        SourceLocation end = loc();
        
        if (expect(Token::Eq)) {
            init = parse_initial_expression();
            end = init->get_span().end;
        }

        // Semis are not strictly necessary, but are not disallowed either.
        while (expect(Token::Semi));

        VariableDecl* var = VariableDecl::create(
            *m_context, 
            SourceSpan(name.loc, end), 
            name.value, 
            {}, 
            type, 
            init, 
            true);

        m_scope->add(var);
        return var;
    }

    return nullptr;
}

Decl* Parser::parse_load_declaration() {
    return nullptr;
}

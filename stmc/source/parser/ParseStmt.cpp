//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/parser/Parser.hpp"
#include "stmc/tree/Decl.hpp"
#include "stmc/tree/Scope.hpp"
#include "stmc/tree/Stmt.hpp"
#include "stmc/types/SourceLocation.hpp"

#include <cassert>

using namespace stm;

Stmt* Parser::parse_initial_statement() {
    if (match(Token::SetBrace)) {
        return parse_block_statement();
    } else if (match("asm")) {
        return parse_inline_assembly_statement();
    } else if (match("let")) {
        return parse_declarative_statement();
    } else {
        return parse_control_statement();
    }
}

Stmt* Parser::parse_inline_assembly_statement() {
    const SourceLocation start = loc();
    next(); // 'asm'

    if (!expect(Token::SetParen))
        m_diags.fatal("expected '('", loc());

    string iasm;
    vector<Expr*> args = {};
    vector<string> outs = {};
    vector<string> ins = {};
    vector<string> clobbers = {};

    // Parse the assembly template (between '(' and the first ':').
    while (!expect(Token::Colon)) {
        if (!match(Token::String))
            m_diags.fatal("expected inline assembly string literal", loc());

        iasm += last().value;
        next();
    }

    // Parse the output constraints (between the first ':' and the second ':').
    while (!expect(Token::Colon)) {
        if (!match(Token::String))
            m_diags.fatal("expected output constraint string", loc());

        outs.push_back(last().value);
        next();

        if (!expect(Token::SetParen))
            m_diags.fatal("expected '('", loc());

        Expr* arg = parse_initial_expression();
        if (!arg)
            m_diags.fatal("expected expression", loc());

        args.push_back(arg);

        if (!expect(Token::EndParen))
            m_diags.fatal("expected ')'", loc());

        if (expect(Token::Colon))
            break;

        if (!expect(Token::Comma))
            m_diags.fatal("expected ','", loc());
    }

    // Parse the input constraints (between the second ':' and the third ':').
    while (!expect(Token::Colon)) {
        if (!match(Token::String))
            m_diags.fatal("expected input constraint string", loc());

        ins.push_back(last().value);
        next();

        if (!expect(Token::SetParen))
            m_diags.fatal("expected '('", loc());

        Expr* arg = parse_initial_expression();
        if (!arg)
            m_diags.fatal("expected expression", loc());

        args.push_back(arg);

        if (!expect(Token::EndParen))
            m_diags.fatal("expected ')'", loc());

        if (expect(Token::Colon))
            break;

        if (!expect(Token::Comma))
            m_diags.fatal("expected ','", loc());
    }

    // Parse the clobbers (between the third ':' and the ')').
    while (!match(Token::EndParen)) {
        if (!match(Token::String))
            m_diags.fatal("expected clobber string", loc());

        clobbers.push_back(last().value);
        next();

        if (match(Token::EndParen))
            break;

        if (!expect(Token::Comma))
            m_diags.fatal("expected ','", loc());
    }

    const SourceLocation end = loc();
    next(); // ')'

    return AsmStmt::create(
        *m_context, SourceSpan(start, end), iasm, outs, ins, args, clobbers);
}

Stmt* Parser::parse_block_statement() {
    SourceLocation start = loc();
    next(); // '{'

    vector<Stmt*> stmts = {};
    stmts.reserve(4);

    Scope* scope = enter_scope();

    while (!match(Token::EndBrace)) {
        Stmt* stmt = parse_initial_statement();
        while (expect(Token::Semi));
        stmts.push_back(stmt);
    }

    stmts.shrink_to_fit();
    exit_scope();

    SourceLocation end = loc();
    next(); // '}'

    return BlockStmt::create(*m_context, SourceSpan(start, end), scope, stmts);
}

Stmt* Parser::parse_control_statement() {
    const Token ctrl = last();
    
    if (expect("break")) {
        return BreakStmt::create(*m_context, since(ctrl.loc));
    } else if (expect("continue")) {
        return ContinueStmt::create(*m_context, since(ctrl.loc));
    } else if (expect("ret")) {
        Expr* expr = nullptr;
        if (!expect(Token::Semi)) {
            expr = parse_initial_expression();
            if (!expect(Token::Semi))
                m_diags.fatal("expected ';'", loc());
        }

        return RetStmt::create(*m_context, since(ctrl.loc), expr);
    } else if (expect("if")) {
        Expr* cond = parse_initial_expression();
        assert(cond && "unable to parse 'if' condition!");

        Stmt* then_body = parse_initial_statement();
        assert(then_body && "unable to parse 'if' then body!");

        Stmt* else_body = nullptr;
        if (expect("else")) {
            else_body = parse_initial_statement();
            assert(else_body && "unable to parse 'if' else body!");
        }

        return IfStmt::create(
            *m_context, since(ctrl.loc), cond, then_body, else_body);
    } else if (expect("while")) {
        Expr* cond = parse_initial_expression();
        assert(cond && "unable to parse 'while' statement!");

        Stmt* body = nullptr;
        if (!match(Token::Semi)) {
            body = parse_initial_statement();
            assert(body && "unable to parse 'while' body!");
        }

        return WhileStmt::create(*m_context, since(ctrl.loc), cond, body);
    }

    return parse_initial_expression();
}

Stmt* Parser::parse_declarative_statement() {
    // For now, just support local variable parsing.

    const SourceLocation start = loc();
    next(); // 'let'

    if (!match(Token::Identifier))
        m_diags.fatal("expected identifier", loc());

    const string name = last().value;
    next();

    if (!expect(Token::Colon))
        m_diags.fatal("expected ':'", loc());

    TypeUse type = parse_type();

    SourceLocation end = loc();
    Expr* init = nullptr;
    if (!expect(Token::Semi)) {
        if (!expect(Token::Eq))
            m_diags.fatal("expected '='", loc());

        init = parse_initial_expression();
        end = loc();
        if (!expect(Token::Semi))
            m_diags.fatal("expected ';'", loc());
    }

    VariableDecl* var =  VariableDecl::create(
        *m_context, 
        SourceSpan(start, end), 
        name, 
        {}, 
        type, 
        init,
        false);

    m_scope->add(var);
    return DeclStmt::create(*m_context, var);
}

//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "ast/Parser.hpp"
#include "ast/Decl.hpp"
#include "ast/Expr.hpp"
#include "ast/Stmt.hpp"
#include "ast/Type.hpp"
#include "core/Logger.hpp"
#include "core/Span.hpp"
#include "lexer/Token.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace scc;

Parser::Parser(const std::string& file) : m_file(file), m_lexer(file) {}

void Parser::parse(TranslationUnit& unit) {
    next();

    m_context = &unit.m_context;

    auto scope = enter_scope();
    unit.m_scope = std::move(scope);

    while (!m_lexer.is_eof()) {
        auto decl = parse_decl();
        assert(decl != nullptr && "could not parse declaration!");
        unit.m_decls.push_back(std::move(decl));
    }

    m_context = nullptr;
    m_scope = nullptr;
}

bool Parser::match(TokenKind kind) const {
    return m_lexer.last().kind == kind;
}

bool Parser::match(const char* kw) const {
    const Token& token = m_lexer.last();
    return token.kind == TokenKind::Identifier && kw == token.value;
}

void Parser::next() {
    m_lexer.lex();
}

void Parser::skip(uint32_t n) {
    for (uint32_t i = 0; i < n; ++i)
        m_lexer.lex();
}

Span Parser::since(const SourceLocation& loc) {
    return Span { .begin = loc, .end = m_lexer.last().loc };
}

std::unique_ptr<Scope> Parser::enter_scope() {
    auto scope = std::unique_ptr<Scope>(new Scope(m_scope));
    m_scope = scope.get();
    return scope;
}

void Parser::exit_scope() {
    if (m_scope != nullptr)
        m_scope = m_scope->get_parent();
}

BinaryExpr::Op Parser::get_binary_operator(TokenKind kind) const {
    switch (kind) {
    case TokenKind::Eq: 
        return BinaryExpr::Assign;
    case TokenKind::EqEq: 
        return BinaryExpr::Equals;
    case TokenKind::BangEq: 
        return BinaryExpr::NotEquals;
    case TokenKind::Plus: 
        return BinaryExpr::Add;
    case TokenKind::PlusEq: 
        return BinaryExpr::AddAssign;
    case TokenKind::Minus: 
        return BinaryExpr::Sub;
    case TokenKind::MinusEq: 
        return BinaryExpr::SubAssign;
    case TokenKind::Star: 
        return BinaryExpr::Mul;
    case TokenKind::StarEq: 
        return BinaryExpr::MulAssign;
    case TokenKind::Slash: 
        return BinaryExpr::Div;
    case TokenKind::SlashEq: 
        return BinaryExpr::DivAssign;
    case TokenKind::Percent: 
        return BinaryExpr::Mod;
    case TokenKind::PercentEq: 
        return BinaryExpr::ModAssign;
    case TokenKind::And: 
        return BinaryExpr::And;
    case TokenKind::AndAnd: 
        return BinaryExpr::LogicAnd;
    case TokenKind::AndEq: 
        return BinaryExpr::AndAssign;
    case TokenKind::Or: 
        return BinaryExpr::Or;
    case TokenKind::OrOr: 
        return BinaryExpr::LogicOr;
    case TokenKind::OrEq: 
        return BinaryExpr::OrAssign;
    case TokenKind::Xor: 
        return BinaryExpr::Xor;
    case TokenKind::XorEq: 
        return BinaryExpr::XorAssign;
    case TokenKind::Lt: 
        return BinaryExpr::LessThan;
    case TokenKind::LShift: 
        return BinaryExpr::LeftShift;
    case TokenKind::LShiftEq: 
        return BinaryExpr::LeftShiftAssign;
    case TokenKind::LtEq: 
        return BinaryExpr::LessThanEquals;
    case TokenKind::Gt: 
        return BinaryExpr::GreaterThan;
    case TokenKind::RShift: 
        return BinaryExpr::RightShift;
    case TokenKind::RShiftEq: 
        return BinaryExpr::RightShiftAssign;
    case TokenKind::GtEq: 
        return BinaryExpr::GreaterThanEquals;
    default: 
        return BinaryExpr::Unknown;
    }
}

UnaryExpr::Op Parser::get_unary_operator(TokenKind kind) const {
    switch (kind) {
    case TokenKind::Bang: 
        return UnaryExpr::Op::LogicNot;
    case TokenKind::PlusPlus: 
        return UnaryExpr::Op::Increment;
    case TokenKind::Minus: 
        return UnaryExpr::Op::Negate;
    case TokenKind::MinusMinus: 
        return UnaryExpr::Op::Decrement;
    case TokenKind::Star: 
        return UnaryExpr::Op::Dereference;
    case TokenKind::And: 
        return UnaryExpr::Op::AddressOf;
    case TokenKind::Tilde: 
        return UnaryExpr::Op::Not;
    default: 
        return UnaryExpr::Op::Unknown;
    }
}

int32_t Parser::get_binary_operator_precedence(TokenKind kind) const {
    switch (kind) {
    case TokenKind::Star:
    case TokenKind::Slash:
    case TokenKind::Percent:
        return 11;
    case TokenKind::Plus:
    case TokenKind::Minus:
        return 10;
    case TokenKind::LShift:
    case TokenKind::RShift:
        return 9;
    case TokenKind::Lt:
    case TokenKind::LtEq:
    case TokenKind::Gt:
    case TokenKind::GtEq:
        return 8;
    case TokenKind::EqEq:
    case TokenKind::BangEq:
        return 7;
    case TokenKind::And:
        return 6;
    case TokenKind::Xor:
        return 5;
    case TokenKind::Or:
        return 4;
    case TokenKind::AndAnd:
        return 3;
    case TokenKind::OrOr:
        return 2;
    case TokenKind::Eq:
    case TokenKind::PlusEq:
    case TokenKind::MinusEq:
    case TokenKind::StarEq:
    case TokenKind::SlashEq:
    case TokenKind::PercentEq:
    case TokenKind::AndEq:
    case TokenKind::OrEq:
    case TokenKind::XorEq:
    case TokenKind::LShiftEq:
    case TokenKind::RShiftEq:
        return 1;
    default: 
        return -1;
    }
}

bool Parser::is_reserved(const std::string& ident) const {
    static std::unordered_set<std::string> keywords = {
        "auto", "break", "case", "char",
        "const", "continue", "default", "do",
        "double", "else", "enum", "extern",
        "float", "for", "goto", "if",
        "int", "long", "register", "return",
        "short", "signed", "sizeof", "static",
        "struct", "switch", "typedef", "union",
        "unsigned", "void", "volatile", "while",
    };

    return keywords.contains(ident);
}

bool Parser::is_typedef(const std::string& ident) const {
    // No named types yet, so only have to consider primitives.

    static std::unordered_set<std::string> keywords = {
        "auto", "char", "const", "double",
        "enum", "float", "int", "long",
        "short", "signed", "struct", "typedef",
        "union", "unsigned", "void", "volatile",  
    };

    return keywords.contains(ident);
}

StorageClass Parser::parse_storage_class() {
    static std::unordered_map<std::string, StorageClass> classes = {
        { "auto", StorageClass::Auto },
        { "extern", StorageClass::Extern },
        { "register", StorageClass::Register },
        { "static", StorageClass::Static },
    };

    if (m_lexer.last().kind != TokenKind::Identifier)
        return StorageClass::None;

    const std::string& value = m_lexer.last().value;
    if (classes.contains(value)) {
        next();
        return classes[value];
    }

    return StorageClass::None;
}

bool Parser::parse_type(QualType& ty) {
    if (!match(TokenKind::Identifier))
        return false;
    
    if (m_lexer.last().value == "const") {
        ty.with_const();
        next();
    } else if (m_lexer.last().value == "volatile") {
        ty.with_volatile();
        next();
    }

    if (m_lexer.last().value == "volatile") {
        if (ty.is_volatile()) {
            Logger::warn("type already marked with 'volatile', ignoring", 
                since(m_lexer.last(1).loc));
        } else {
            ty.with_volatile();
        }

        next();
    }

    std::string base = "";

    while (match(TokenKind::Identifier) && is_reserved(m_lexer.last().value)) {
        base += m_lexer.last().value + ' ';
        next();
    }

    if (!base.empty())
        base = base.substr(0, base.size() - 1);

    static std::unordered_map<std::string, const Type*> primitives = {
        { "void", VoidType::get(*m_context) },
        { "char", IntegerType::get(*m_context, 8, true) },
        { "unsigned char", IntegerType::get(*m_context, 8, false) },
        { "short", IntegerType::get(*m_context, 16, true) },
        { "unsigned short", IntegerType::get(*m_context, 16, false) },
        { "int", IntegerType::get(*m_context, 32, true) },
        { "unsigned int", IntegerType::get(*m_context, 32, false) },
        { "long", IntegerType::get(*m_context, 64, true) },
        { "unsigned long", IntegerType::get(*m_context, 64, false) },
        { "long long", IntegerType::get(*m_context, 64, true) },
        { "unsigned long long", IntegerType::get(*m_context, 64, false) },
        { "float", FPType::get(*m_context, 32) },
        { "double", FPType::get(*m_context, 64) },  
    };

    const Type* pType = nullptr;
    if (primitives.count(base) != 0)
        pType = primitives[base];

    while (match(TokenKind::Star)) {
        pType = PointerType::get(*m_context, pType);
        next();
    }

    ty.set_type(pType);
    return true;
}

std::unique_ptr<Decl> Parser::parse_decl() {
    const SourceLocation start = m_lexer.last().loc;

    // Attempt to parse any preceeding storage class.
    StorageClass sclass = parse_storage_class();

    // Attempt to parse a declaration type.
    QualType ty {};
    if (!parse_type(ty))
        Logger::error("expected type", since(start));
    
    if (!match(TokenKind::Identifier))
        Logger::error("missing identifier after declaration type", since(start));

    std::string name = m_lexer.last().value;
    next(); // identifier

    if (is_reserved(name))
        Logger::error("identifier '" + name + "' is reserved", since(start));

    if (match(TokenKind::Semi) || match(TokenKind::Eq)) {
        // The identifier is followed by a ';' or '=', which means its a 
        // variable in the form of:
        //
        // <type> <ident> ';'
        //       or
        // <type> <ident> = ...
        return parse_variable(start, sclass, ty, name);
    } else if (match(TokenKind::SetParen)) {
        // The identifier is followed by a '(', which means its the beginning
        // of a function parameter list.
        return parse_function(start, sclass, ty, name);
    } else {
        // No declaration pattern matches.
        Logger::error("expected declaration after identifier", since(start));
    }
}

std::unique_ptr<Decl> Parser::parse_function(
        const SourceLocation& start, StorageClass sclass, const QualType& ty, 
        const std::string& name) {
    next(); // '('

    std::unique_ptr<Scope> scope = enter_scope();
    std::vector<std::unique_ptr<ParameterDecl>> params = {};

    // Reserve some space if there are going to be parameters.
    if (!match(TokenKind::EndParen))
        params.reserve(6);

    while (!match(TokenKind::EndParen)) {
        const SourceLocation pstart = m_lexer.last().loc;

        QualType pty {};
        if (!parse_type(pty))
            Logger::error("expected function parameter type");

        if (!match(TokenKind::Identifier))
            Logger::error("missing identifier after parameter type", since(start));

        std::string pname = m_lexer.last().value;
        next(); // identifier

        if (is_reserved(pname))
            Logger::error("identifier '" + pname + "' is reserved", since(start));

        auto param = std::unique_ptr<ParameterDecl>(
            new ParameterDecl(since(pstart), pname, pty));
        
        scope->add(param.get());
        params.push_back(std::move(param));

        if (match(TokenKind::EndParen))
            break;

        if (!match(TokenKind::Comma))
            Logger::error("expected ',' after function parameter", since(start));

        next(); // ','
    }

    next(); // ')'

    // Since we reserved extra space if there was at least one parameter, we
    // can shrink it now since it won't ever change.
    if (!params.empty())
        params.shrink_to_fit();

    const QualType& ret = ty;
    std::vector<QualType> param_types(params.size(), QualType {});
    for (uint32_t i = 0; i < params.size(); ++i)
        param_types[i] = params[i]->get_type();

    const QualType& function_type = 
        FunctionType::get(*m_context, ret, param_types);

    std::unique_ptr<Stmt> body = nullptr;
    if (match(TokenKind::Semi)) {
        next(); // ';'
    } else {
        body = parse_stmt();
        assert(body != nullptr && "could not parse function body!");
    }

    exit_scope();

    auto function = std::unique_ptr<FunctionDecl>(new FunctionDecl(
        sclass, 
        since(start), 
        name, 
        function_type, 
        params, 
        std::move(scope), 
        std::move(body)));

    m_scope->add(function.get());
    return function;
}

std::unique_ptr<Decl> Parser::parse_variable(
        const SourceLocation& start, StorageClass sclass, const QualType& ty, 
        const std::string& name) {
    std::unique_ptr<Expr> init = nullptr;
    if (match(TokenKind::Eq)) {
        next(); // '='
        init = parse_expr();
        assert(init != nullptr && "could not parse variable initializer!");
    }

    auto var = std::unique_ptr<VariableDecl>(new VariableDecl(
        sclass, since(start), name, ty, std::move(init)
    ));

    m_scope->add(var.get());
    return var;
}

std::unique_ptr<Expr> Parser::parse_expr() {
    return parse_primary();
}

std::unique_ptr<Expr> Parser::parse_primary() {
    if (match(TokenKind::Integer)) {
        return parse_integer();
    } else if (match(TokenKind::Identifier)) {
        return parse_ref();
    }
    
    return nullptr;
}

std::unique_ptr<Expr> Parser::parse_integer() {
    const Token& integer = m_lexer.last();
    const SourceLocation start = integer.loc;
    int64_t value = std::stoll(integer.value);
    next();

    const Type* ty = nullptr;
    if (!match(TokenKind::Identifier)) {
        // Default to 'int' type.
        ty = IntegerType::get(*m_context, 32, true);
    } else if (match("u") || match("U")) {
        ty = IntegerType::get(*m_context, 32, false);
    } else if (match("l") || match("L") || match("ll") || match("LL")) {
        ty = IntegerType::get(*m_context, 64, true);
    } else if (match("ul") || match("UL") || match("ull") || match("ULL")) {
        ty = IntegerType::get(*m_context, 64, false);
    }

    return std::unique_ptr<IntegerLiteral>(new IntegerLiteral(
        since(start), QualType(ty), value
    ));
}

std::unique_ptr<Expr> Parser::parse_float() {
    return nullptr;
}

std::unique_ptr<Expr> Parser::parse_character() {
    return nullptr;
}

std::unique_ptr<Expr> Parser::parse_string() {
    return nullptr;
}

std::unique_ptr<Expr> Parser::parse_binary() {
    return nullptr;
}

std::unique_ptr<Expr> Parser::parse_unary() {
    return nullptr;
}

std::unique_ptr<Expr> Parser::parse_ref() {
    const Token token = m_lexer.last();
    next(); // identifier

    Decl* decl = m_scope->get(token.value);
    if (!decl) {
        Logger::error("unresolved reference: '" + token.value + "'", 
            since(token.loc));
    }

    return std::unique_ptr<RefExpr>(new RefExpr(
        since(token.loc), decl->get_type(), decl
    ));
}

std::unique_ptr<Expr> Parser::parse_call() {
    return nullptr;
}

std::unique_ptr<Expr> Parser::parse_cast() {
    return nullptr;
}

std::unique_ptr<Expr> Parser::parse_sizeof() {
    return nullptr;
}

std::unique_ptr<Stmt> Parser::parse_stmt() {
    if (match(TokenKind::SetBrace)) {
        return parse_compound();
    }

    if (match("return")) {
        return parse_return();
    } else if (match("if")) {
        return parse_if();
    } else if (match("break")) {
        const SourceLocation start = m_lexer.last().loc;
        next(); // 'break'

        return std::unique_ptr<BreakStmt>(new BreakStmt(since(start)));
    } else if (match("continue")) {
        const SourceLocation start = m_lexer.last().loc;
        next(); // 'continue'

        return std::unique_ptr<ContinueStmt>(new ContinueStmt(since(start)));
    } else if (match(TokenKind::Identifier) && is_typedef(m_lexer.last().value)) {
        auto var = parse_decl();
        assert(var != nullptr && "could not parse variable declaration!");

        return std::unique_ptr<DeclStmt>(new DeclStmt(
            var->span(), std::move(var)
        ));
    }

    auto expr = parse_expr();
    assert(expr != nullptr && "could not parse expression statement!");

    return std::unique_ptr<ExprStmt>(new ExprStmt(
        expr->span(), std::move(expr)
    ));
}

std::unique_ptr<Stmt> Parser::parse_compound() {
    const SourceLocation start = m_lexer.last().loc;

    next(); // '{'

    auto scope = enter_scope();
    std::vector<std::unique_ptr<Stmt>> stmts = {};
    
    if (!match(TokenKind::EndBrace))
        stmts.reserve(4);

    while (!match(TokenKind::EndBrace)) {
        auto stmt = parse_stmt();
        assert(stmt != nullptr && "could not parse statement!");
        stmts.push_back(std::move(stmt));

        if (match(TokenKind::EndBrace))
            break;

        while (match(TokenKind::Semi))
            next(); // ';'
    }

    next(); // '}'

    if (!stmts.empty())
        stmts.shrink_to_fit();

    exit_scope();

    return std::unique_ptr<CompoundStmt>(new CompoundStmt(
        since(start), std::move(scope), stmts
    ));
}

std::unique_ptr<Stmt> Parser::parse_if() {
    const SourceLocation start = m_lexer.last().loc;
    next(); // 'if'

    std::unique_ptr<Expr> cond = nullptr;
    std::unique_ptr<Stmt> then = nullptr, els = nullptr;

    if (!match(TokenKind::SetParen))
        Logger::error("missing '(' after 'if' keyword", since(start));

    next(); // '('

    cond = parse_expr();
    assert(cond != nullptr && "could not parse if condition expression!");

    if (!match(TokenKind::EndParen))
        Logger::error("missing ')' after 'if' condition", since(start));

    next(); // ')'

    then = parse_stmt();
    assert(then != nullptr && "could not parse if-then statement!");

    if (match("else")) {
        next(); // 'else'

        els = parse_stmt();
        assert(els != nullptr && "could not parse if-else statement!");
    }

    return std::unique_ptr<IfStmt>(new IfStmt(
        since(start), std::move(cond), std::move(then), std::move(els)
    ));
}

std::unique_ptr<Stmt> Parser::parse_return() {
    const SourceLocation start = m_lexer.last().loc;
    next(); // 'return'

    std::unique_ptr<Expr> expr = nullptr;
    if (!match(TokenKind::Semi)) {
        expr = parse_expr();
        assert(expr != nullptr && "could not parse return expression!");
    }

    return std::unique_ptr<ReturnStmt>(new ReturnStmt(
        since(start), std::move(expr)
    ));
}

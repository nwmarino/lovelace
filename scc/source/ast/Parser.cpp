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

Parser::Parser(const std::string& file, const std::string& source) 
    : m_file(file), m_lexer(file, source) {}

void Parser::parse(TranslationUnit& unit) {
    next();

    m_unit = &unit;
    m_context = &unit.m_context;

    auto scope = enter_scope();
    unit.m_scope = std::move(scope);

    while (!m_lexer.is_eof()) {
        auto decl = parse_decl();
        //assert(decl != nullptr && "could not parse declaration!");

        if (decl != nullptr)
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
    return Span(loc, m_lexer.last().loc);
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

bool Parser::is_storage_class(const std::string& ident) const {
    static std::unordered_set<std::string> keywords = {
        "auto", "extern", "register", "static",
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
    if (!match(TokenKind::Identifier))
        return StorageClass::None;

    static std::unordered_map<std::string, StorageClass> classes = {
        { "auto", StorageClass::Auto },
        { "extern", StorageClass::Extern },
        { "register", StorageClass::Register },
        { "static", StorageClass::Static },
    };

    const std::string value = m_lexer.last().value;
    if (classes.contains(value)) {
        next();
        return classes[value];
    }

    return StorageClass::None;
}

bool Parser::parse_type(QualType& ty) {
    if (!match(TokenKind::Identifier))
        return false;

    const SourceLocation start = m_lexer.last().loc;
    
    if (match("const")) {
        ty.with_const();
        next();
    } else if (match("volatile")) {
        ty.with_volatile();
        next();
    }

    if (match("volatile")) {
        if (ty.is_volatile()) {
            Logger::warn("type already qualified with 'volatile', ignoring", 
                since(m_lexer.last(1).loc));
        } else {
            ty.with_volatile();
        }

        next();
    }

    std::string base = "";

    while (match(TokenKind::Identifier) && is_typedef(m_lexer.last().value)) {
        base += m_lexer.last().value + ' ';
        next();
    }

    if (!base.empty())
        base = base.substr(0, base.size() - 1);

    std::unordered_map<std::string, const Type*> primitives = {
        { "void", BuiltinType::get_void_type(*m_context) },
        { "char", BuiltinType::get_char_type(*m_context) },
        { "unsigned char", BuiltinType::get_uchar_type(*m_context) },
        { "short", BuiltinType::get_short_type(*m_context) },
        { "unsigned short", BuiltinType::get_ushort_type(*m_context) },
        { "int", BuiltinType::get_int_type(*m_context) },
        { "unsigned int", BuiltinType::get_uint_type(*m_context) },
        { "long", BuiltinType::get_long_type(*m_context) },
        { "unsigned long", BuiltinType::get_ulong_type(*m_context) },
        { "long long", BuiltinType::get_longlong_type(*m_context) },
        { "unsigned long long", BuiltinType::get_ulonglong_type(*m_context) },
        { "float", BuiltinType::get_float_type(*m_context) },
        { "double", BuiltinType::get_double_type(*m_context) },
        { "long double", BuiltinType::get_longdouble_type(*m_context) },
    };

    const Type* pType = nullptr;
    if (!base.empty()) {
        if (primitives.count(base) != 0) {
            pType = primitives.at(base);
        } else {
            Logger::error("expected type", since(start));
        }
    } else {
        const Decl* decl = m_scope->get(m_lexer.last().value);
        if (!decl) Logger::error("expected type", since(start));

        pType = decl->get_type().get_type();
        next(); // identifier
    }

    while (match(TokenKind::Star)) {
        pType = PointerType::get(*m_context, pType);
        next();
    }

    ty.set_type(pType);
    return true;
}

std::unique_ptr<Decl> Parser::parse_decl() {
    if (match("typedef")) return parse_typedef();

    const SourceLocation start = m_lexer.last().loc;

    // Attempt to parse any preceeding storage class.
    StorageClass sclass = parse_storage_class();

    // Attempt to parse a declaration type.
    QualType ty {};
    if (sclass != StorageClass::Auto && !parse_type(ty))
        Logger::error("expected type", since(start));
    
    if (!match(TokenKind::Identifier))
        Logger::error("missing identifier after declaration type", since(start));

    const std::string name = m_lexer.last().value;
    next(); // identifier

    if (is_reserved(name))
        Logger::error("identifier '" + name + "' is reserved", since(start));

    if (match(TokenKind::Semi) || match(TokenKind::Eq) || match(TokenKind::SetBrack)) {
        // The identifier is followed by a ';' or '=' or '[', which means its a 
        // variable in the form of:
        //
        // <type> <ident> ';'
        //       or
        // <type> <ident> = ...
        //       or
        // <type> <ident>[...]
        return parse_variable(start, sclass, ty, name);
    } else if (match(TokenKind::SetParen)) {
        // The identifier is followed by a '(', which means its the beginning
        // of a function parameter list.

        // TEMPORARY: Disallow auto classed functions.
        if (sclass == StorageClass::Auto)
            Logger::error("function cannot be marked with 'auto' keyword", since(start));

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
    if (!match(TokenKind::EndParen)) params.reserve(6);

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

        while (match(TokenKind::SetBrack)) {
            next(); // '['

            // TODO: Support automatic array size checks.
            if (!match(TokenKind::Integer))
                Logger::error("missing array size literal after '['", since(start));

            uint32_t size = std::stoul(m_lexer.last().value);
            next(); // size

            if (!match(TokenKind::EndBrack))
                Logger::error("missing ']' after array size");

            next(); // ']'

            pty = QualType(
                ArrayType::get(*m_context, pty, size)
            );
        }

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

    std::vector<QualType> param_types(params.size(), QualType {});
    for (uint32_t i = 0; i < params.size(); ++i)
        param_types[i] = params[i]->get_type();

    const FunctionType* function_type = 
        FunctionType::get(*m_context, ty, param_types);

    std::unique_ptr<Stmt> body = nullptr;
    if (match(TokenKind::Semi)) {
        next(); // ';'
    } else {
        body = parse_stmt();
        assert(body != nullptr && "could not parse function body!");
    }

    exit_scope();

    // Check for a previous declaration with the same name in the parent scope.
    const Decl* prev = m_scope->get(name);
    if (!prev) {
        // No previous declaration, so we can define this function as is.
        auto function = std::unique_ptr<FunctionDecl>(new FunctionDecl(
            sclass, 
            since(start), 
            name, 
            QualType(function_type), 
            params, 
            std::move(scope), 
            std::move(body)));

        m_scope->add(function.get());
        return function;
    }

    // A previous declaration exists, so now we have to check it to be both a
    // function and have the same signature defined here.
    const FunctionDecl* prev_fn = dynamic_cast<const FunctionDecl*>(prev);
    if (!prev_fn)
        Logger::error("redefinition of '" + name + "'", since(start));

    const FunctionType* prev_ty = static_cast<const FunctionType*>(
        prev_fn->get_type().get_type());

    if (prev->storage_class() != sclass) {
        Logger::error("conflicting storage classes for '" + name + "'", 
            since(start));
    }

    bool return_types_match = prev_ty->get_return_type() == ty;
    bool parameter_counts_match = prev_fn->num_params() == params.size();
    bool param_types_match = true;

    if (parameter_counts_match) {
        for (uint32_t i = 0; i < params.size(); ++i) {
            if (params[i]->get_type() != prev_ty->get_param_type(i)) {
                param_types_match = false;
                break;
            }
        }
    }

    if (!return_types_match || !parameter_counts_match || !param_types_match) {
        Logger::error("conflicting types for '" + name + "'; have '" + 
            prev_fn->get_type().to_string() + "'", start);
    }

    if (prev_fn->has_body() && body != nullptr)
        Logger::error("redefinition of '" + name + "'", start);

    // TODO: Cleanup. Using const cast here is smelly...
    FunctionDecl* cprev = const_cast<FunctionDecl*>(prev_fn);
    cprev->m_body = std::move(body);
    return nullptr;
}

std::unique_ptr<Decl> Parser::parse_variable(
        const SourceLocation& start, StorageClass sclass, const QualType& ty, 
        const std::string& name) {
    QualType var_type = ty;

    while (match(TokenKind::SetBrack)) {
        next(); // '['

        // TODO: Support automatic array size checks.
        if (!match(TokenKind::Integer))
            Logger::error("missing array size literal after '['", since(start));

        uint32_t size = std::stoul(m_lexer.last().value);
        next(); // size

        if (!match(TokenKind::EndBrack))
            Logger::error("missing ']' after array size");

        next(); // ']'

        var_type = QualType(
            ArrayType::get(*m_context, var_type, size)
        );
    }

    std::unique_ptr<Expr> init = nullptr;
    if (match(TokenKind::Eq)) {
        next(); // '='
        init = parse_expr();
        assert(init != nullptr && "could not parse variable initializer!");
    }

    if (sclass == StorageClass::Auto) {
        if (!init)
            Logger::error("variable marked 'auto' but requires initializer");

        if (ty.get_type() != nullptr)
            Logger::error("variable marked 'auto', but type provided");
    }

    auto var = std::unique_ptr<VariableDecl>(new VariableDecl(
        sclass, 
        since(start), 
        name, 
        sclass == StorageClass::Auto ? init->get_type() : var_type, 
        std::move(init)
    ));

    m_scope->add(var.get());
    return var;
}

std::unique_ptr<Decl> Parser::parse_typedef() {
    const SourceLocation start = m_lexer.last().loc;
    next(); // 'typedef'

    QualType underlying {};
    if (!parse_type(underlying))
        Logger::error("expected type after 'typedef'", since(start));

    if (!match(TokenKind::Identifier))
        Logger::error("expected identifier", since(start));

    std::string name = m_lexer.last().value;
    next(); // identifier

    if (is_reserved(name))
        Logger::error("identifier '" + name + "' is reserved", since(start));

    if (match(TokenKind::Semi)) {
        next(); // ';'
    } else {
        Logger::error("expected ';'", since(start));
    }

    std::unique_ptr<TypedefDecl> decl = std::make_unique<TypedefDecl>(
        since(start), name, QualType()
    );

    m_scope->add(decl.get());

    decl->get_type() = QualType(TypedefType::create(
        *m_context, decl.get(), underlying
    ));

    return decl;
}

std::unique_ptr<Expr> Parser::parse_expr() {
    auto base = parse_unary_prefix();
    assert(base != nullptr && "could not parse base expression!");
    base = parse_binary(std::move(base), 0);

    if (match(TokenKind::Question)) {
        return parse_ternary(std::move(base));
    } else {
        return base;
    }
}

std::unique_ptr<Expr> Parser::parse_primary() {
    if (match(TokenKind::Integer)) {
        return parse_integer();
    } else if (match(TokenKind::Float)) {
        return parse_float();
    } else if (match(TokenKind::Character)) {
        return parse_character();
    } else if (match(TokenKind::String)) {
        return parse_string();
    } else if (match("sizeof")) {
        return parse_sizeof();
    } else if (match(TokenKind::Identifier)) {
        return parse_ref();
    } else if (match(TokenKind::SetParen)) {
        const SourceLocation start = m_lexer.last().loc;
        next(); // '('

        std::unique_ptr<Expr> expr = nullptr;

        if (match(TokenKind::Identifier) && is_typedef(m_lexer.last().value)) {
            QualType ty {};
            if (!parse_type(ty))
                Logger::error("expected cast type", since(start));

            if (!match(TokenKind::EndParen))
                Logger::error("expected ')' after cast type", since(start));

            next(); // ')'

            expr = parse_expr();
            assert(expr != nullptr && "could not parse cast expression!");

            return std::make_unique<CastExpr>(since(start), ty, std::move(expr));
        } else {
            expr = parse_expr();
            assert(expr != nullptr && "could not parse parentheses expression!");

            if (!match(TokenKind::EndParen))
                Logger::error("expected ')' after expression", since(start));

            next(); // ')'
            return std::make_unique<ParenExpr>(
                since(start), expr->get_type(), std::move(expr)
            );
        }
    }
    
    return nullptr;
}

std::unique_ptr<Expr> Parser::parse_integer() {
    const Token integer = m_lexer.last();
    next(); // integer

    const Type* ty = nullptr;
    if (!match(TokenKind::Identifier)) {
        // Default to 'int' type.
        ty = BuiltinType::get_int_type(*m_context);
    } else if (match("u") || match("U")) {
        ty = BuiltinType::get_uint_type(*m_context);
        next(); // 'u' || 'U'
    } else if (match("l") || match("L")) {
        ty = BuiltinType::get_long_type(*m_context);
        next(); // 'l' || 'L'
    } else if (match("ul") || match("UL")) {
        ty = BuiltinType::get_ulong_type(*m_context);
        next(); // 'ul' || 'UL'
    } else if (match("ll") || match("LL")) {
        ty = BuiltinType::get_longlong_type(*m_context);
        next(); // 'll' || 'LL'
    } else if (match("ull") || match("ULL")) {
        ty = BuiltinType::get_ulonglong_type(*m_context);
        next(); // 'ull' || 'ULL'
    }

    return std::make_unique<IntegerLiteral>(
        since(integer.loc), QualType(ty), std::stoll(integer.value)
    );
}

std::unique_ptr<Expr> Parser::parse_float() {
    const Token fp = m_lexer.last();
    next(); // float

    const Type* ty = nullptr;
    if (!match(TokenKind::Identifier)) {
        // Default to 'double' type.
        ty = BuiltinType::get_double_type(*m_context);
    } else if (match("f") || match("F")) {
        ty = BuiltinType::get_float_type(*m_context);
        next(); // 'f' || 'F'
    }

    return std::make_unique<FPLiteral>(
        since(fp.loc), QualType(ty), std::stod(fp.value)
    );
}

std::unique_ptr<Expr> Parser::parse_character() {
    const Token ch = m_lexer.last();
    next(); // '...'

    return std::make_unique<CharLiteral>(
        since(ch.loc), 
        QualType(BuiltinType::get_char_type(*m_context)), 
        ch.value[0]
    );
}

std::unique_ptr<Expr> Parser::parse_string() {
    const Token str = m_lexer.last();
    next(); // "..."

    QualType ty(PointerType::get(
        *m_context, BuiltinType::get_char_type(*m_context)
    ));
    ty.with_const();

    return std::make_unique<StringLiteral>(since(str.loc), ty, str.value);
}

std::unique_ptr<Expr> Parser::parse_binary(std::unique_ptr<Expr> base, 
                                           int32_t precedence) {
    while (1) {
        const Token last = m_lexer.last();

        int32_t token_prec = get_binary_operator_precedence(last.kind);
        if (token_prec < precedence) break;

        BinaryExpr::Op op = get_binary_operator(last.kind);
        if (op == BinaryExpr::Unknown) break;
        next(); // operator
        
        std::unique_ptr<Expr> right = parse_unary_prefix();
        if (!right)
            Logger::error("expected right side expression", since(last.loc));

        int32_t next_prec = get_binary_operator_precedence(m_lexer.last().kind);
        if (token_prec < next_prec) {
            right = parse_binary(std::move(right), precedence + 1);
            if (!right)
                Logger::error("expected right side expression", since(last.loc));
        }

        base = std::make_unique<BinaryExpr>(
            Span(base->span().begin, right->span().end),
            base->get_type(),
            op,
            std::move(base),
            std::move(right)
        );
    }

    return base;
}

std::unique_ptr<Expr> Parser::parse_unary_prefix() {
    UnaryExpr::Op op = get_unary_operator(m_lexer.last().kind);
    
    if (UnaryExpr::is_prefix_op(op)) {
        const SourceLocation start = m_lexer.last().loc;
        next(); // operator

        std::unique_ptr<Expr> base = parse_unary_prefix();
        if (!base) Logger::error("expected expression", since(start));

        return std::make_unique<UnaryExpr>(
            since(start), base->get_type(), op, false, std::move(base)
        );
    } else return parse_unary_postfix();
}

std::unique_ptr<Expr> Parser::parse_unary_postfix() {
    std::unique_ptr<Expr> base = parse_primary();
    if (!base) Logger::error("expected expression", m_lexer.last().loc);

    while (1) {
        const SourceLocation start = m_lexer.last().loc;
        UnaryExpr::Op op = get_unary_operator(m_lexer.last().kind);

        if (UnaryExpr::is_postfix_op(op)) {
            next(); // operator

            base = std::make_unique<UnaryExpr>(
                since(start), base->get_type(), op, true, std::move(base)
            );
        } else if (match(TokenKind::SetParen)) {
            next(); // '('

            std::vector<std::unique_ptr<Expr>> args = {};
            if (!match(TokenKind::EndParen)) args.reserve(2);

            while (!match(TokenKind::EndParen)) {
                std::unique_ptr<Expr> arg = parse_expr();
                if (!arg) Logger::error("expected expression", since(start));
                
                args.push_back(std::move(arg));

                if (match(TokenKind::EndParen)) break;

                if (match(TokenKind::Comma)) {
                    next(); // ','
                } else {
                    Logger::error("expected ',' after function call argument", 
                        since(start));
                }
            }

            next(); // ')'

            if (!args.empty()) args.shrink_to_fit();

            base = std::make_unique<CallExpr>(
                since(start), base->get_type(), std::move(base), args
            );
        } else if (match(TokenKind::SetBrack)) {
            next(); // '['

            std::unique_ptr<Expr> index = parse_expr();
            if (!index) 
                Logger::error("expected expression after '['", since(start));

            if (match(TokenKind::EndBrack)) {
                next(); // ']'
            } else {
                Logger::error("expected ']'");
            }

            base = std::make_unique<SubscriptExpr>(
                since(start), 
                base->get_type(), 
                std::move(base), 
                std::move(index)
            );
        } else if (match(TokenKind::Dot) || match(TokenKind::Arrow)) {
            bool arrow = match(TokenKind::Arrow);
            next(); // '.' || '->'

            const Decl* member = nullptr;

            if (match(TokenKind::Identifier)) {
                const QualType& base_type = base->get_type();
                // TODO: Determine member based on base structure type.
            } else {
                Logger::error("expected identifier after '" + 
                    std::string(arrow ? "->" : ".") + "'", since(start));
            }

            base = std::make_unique<MemberExpr>(
                since(start),
                member->get_type(),
                std::move(base),
                member,
                arrow
            );
        } else {
            break;
        }
    }

    return base;
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

std::unique_ptr<Expr> Parser::parse_sizeof() {
    const SourceLocation start = m_lexer.last().loc;
    next(); // 'sizeof'

    if (!match(TokenKind::SetParen))
        Logger::error("missing '(' after 'sizeof' keyword");

    next(); // '('

    QualType ty {};
    if (!parse_type(ty))
        Logger::error("expected type for 'sizeof' operator");

    if (!match(TokenKind::EndParen))
        Logger::error("missing ')' after 'sizeof' type");

    next(); // ')'

    // Uses ulong for the sizeof result.
    return std::unique_ptr<SizeofExpr>(new SizeofExpr(
        since(start), QualType(BuiltinType::get_ulong_type(*m_context)), ty
    ));
}

std::unique_ptr<Expr> Parser::parse_ternary(std::unique_ptr<Expr> base) {
    next(); // '?'

    std::unique_ptr<Expr> tval = nullptr, fval = nullptr;

    // Parse the true value: '?' ... ':'
    if (!(tval = parse_expr())) Logger::error("expected expression");

    if (match(TokenKind::Colon)) {
        next(); // ':'
    } else {
        Logger::error("expected ':' after ternary specifier");
    }

    /// Parse the false value: ':' ...
    if (!(fval = parse_expr())) Logger::error("expected expression");

    return std::make_unique<TernaryExpr>(
        since(base->span().begin), 
        tval->get_type(), 
        std::move(base), 
        std::move(tval), 
        std::move(fval)
    );
}

std::unique_ptr<Stmt> Parser::parse_stmt() {
    if (match(TokenKind::SetBrace)) {
        return parse_compound();
    } else if (match("return")) {
        return parse_return();
    } else if (match("if")) {
        return parse_if();
    } else if (match("while")) {
        return parse_while();
    } else if (match("for")) {
        return parse_for();
    } else if (match("switch")) {
        return parse_switch();
    } else if (match("break")) {
        const SourceLocation start = m_lexer.last().loc;
        next(); // 'break'
        return std::make_unique<BreakStmt>(since(start));
    } else if (match("continue")) {
        const SourceLocation start = m_lexer.last().loc;
        next(); // 'continue'
        return std::make_unique<ContinueStmt>(since(start));
    } else if (match(TokenKind::Identifier) && 
      (is_storage_class(m_lexer.last().value) || is_typedef(m_lexer.last().value))) {
        std::unique_ptr<Decl> var = parse_decl();
        if (!var) Logger::error("expected variable declaration");

        return std::make_unique<DeclStmt>(var->span(), std::move(var));
    }

    // Fallback to an expression statement.
    std::unique_ptr<Expr> expr = parse_expr();
    if (!expr) Logger::error("expected expression");

    return std::make_unique<ExprStmt>(expr->span(), std::move(expr));
}

std::unique_ptr<Stmt> Parser::parse_compound() {
    const SourceLocation start = m_lexer.last().loc;
    next(); // '{'

    std::unique_ptr<Scope> scope = enter_scope();
    std::unique_ptr<Stmt> stmt = nullptr;
    std::vector<std::unique_ptr<Stmt>> stmts = {};
    if (!match(TokenKind::EndBrace)) stmts.reserve(4);

    while (!match(TokenKind::EndBrace)) {
        if (!(stmt = parse_stmt())) 
            Logger::error("expected statement", since(start));

        stmts.push_back(std::move(stmt));
        stmt = nullptr;

        if (match(TokenKind::EndBrace)) break;

        while (match(TokenKind::Semi)) next(); // ';'
    }

    next(); // '}'
    exit_scope();

    if (!stmts.empty()) stmts.shrink_to_fit();

    return std::make_unique<CompoundStmt>(
        since(start), std::move(scope), stmts
    );
}

std::unique_ptr<Stmt> Parser::parse_if() {
    const SourceLocation start = m_lexer.last().loc;
    next(); // 'if'

    std::unique_ptr<Expr> cond = nullptr;
    std::unique_ptr<Stmt> then = nullptr, els = nullptr;

    if (match(TokenKind::SetParen)) {
        next(); // '('
    } else {
        Logger::error("expected '(' after 'if'", since(start));
    }

    if (!(cond = parse_expr()))
        Logger::error("expected expression after '('", since(start));

    if (match(TokenKind::EndParen)) {
        next(); // ')'
    } else {
        Logger::error("missing ')' after 'if' condition", since(start));
    }

    if (!(then = parse_stmt())) 
        Logger::error("expected statement", since(start));

    if (match("else")) {
        next(); // 'else'

        if (!(els = parse_stmt()))
            Logger::error("expected statement after 'else'", since(start));
    }

    return std::make_unique<IfStmt>(
        since(start), std::move(cond), std::move(then), std::move(els)
    );
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

std::unique_ptr<Stmt> Parser::parse_while() {
    const SourceLocation start = m_lexer.last().loc;
    next(); // 'while'

    std::unique_ptr<Expr> cond = nullptr;
    std::unique_ptr<Stmt> body = nullptr;

    if (!match(TokenKind::SetParen))
        Logger::error("missing '(' after 'while' keyword", since(start));

    next(); // '('

    cond = parse_expr();
    assert(cond != nullptr && "could not parse while condition expression!");

    if (!match(TokenKind::EndParen))
        Logger::error("missing ')' after 'while' condition", since(start));

    next(); // ')'

    if (match(TokenKind::Semi)) {
        next(); // ';'
    } else {
        body = parse_stmt();
        assert(body != nullptr && "could not parse while body!");
    }

    return std::unique_ptr<WhileStmt>(new WhileStmt(
        since(start), std::move(cond), std::move(body)
    ));
}

std::unique_ptr<Stmt> Parser::parse_for() {
    const SourceLocation start = m_lexer.last().loc;
    next(); // 'while'

    std::unique_ptr<Stmt> init = nullptr, body = nullptr;
    std::unique_ptr<Expr> cond = nullptr, step = nullptr;

    if (match(TokenKind::SetParen)) {
        next(); // '('
    } else {
        Logger::error("expected '(' after 'for'", since(start));
    }

    // Parse the 'for' initializer: for (... ';'
    if (!match(TokenKind::Semi))
        if (!(init = parse_stmt())) Logger::error("expected statement");

    // Eat any semicolon after the initializer.
    if (match(TokenKind::Semi)) next(); // ';'

    // Parse the 'for' stop condition: ';' ... ';'
    if (!match(TokenKind::Semi))
        if (!(cond = parse_expr())) Logger::error("expected expresion");

    // Eat any semicolon after the stop condition.
    if (match(TokenKind::Semi)) next(); // ';'

    // Parse the 'for' step: ';' ... ')'
    if (!match(TokenKind::EndParen))
        if (!(step = parse_expr())) Logger::error("expected expression");

    if (match(TokenKind::EndParen)) {
        next(); // ')'
    } else {
        Logger::error("expected ')' after 'for' specifier");
    }

    if (!match(TokenKind::Semi))
        if (!(body = parse_stmt())) Logger::error("expected statement");

    return std::make_unique<ForStmt>(
        since(start), 
        std::move(init), 
        std::move(cond), 
        std::move(step), 
        std::move(body)
    );
}

std::unique_ptr<Stmt> Parser::parse_switch() {
    const SourceLocation start = m_lexer.last().loc;
    next(); // 'switch'

    std::unique_ptr<Expr> mtch = nullptr;
    std::unique_ptr<Stmt> def = nullptr;
    std::vector<std::unique_ptr<CaseStmt>> cases = {};

    if (match(TokenKind::SetParen)) {
        next(); // '('
    } else {
        Logger::error("expected '(' after 'switch'");
    }

    if (!(mtch = parse_expr())) Logger::error("expected expression");

    if (match(TokenKind::EndParen)) {
        next(); // ')'
    } else {
        Logger::error("expected ')' after 'switch' specifier");
    }

    if (match(TokenKind::SetBrace)) {
        next(); // '{'
    } else {
        Logger::error("expected '{' after 'switch' specifier");
    }

    cases.reserve(4);
    while (!match(TokenKind::EndBrace)) {
        if (match("case")) {
            const SourceLocation case_s = m_lexer.last().loc;
            next(); // 'case'

            std::unique_ptr<Expr> case_m = nullptr;
            std::unique_ptr<Stmt> case_b = nullptr;

            if (!(case_m = parse_expr()))
                Logger::error("expected expression after 'case'");

            if (match(TokenKind::Colon)) {
                next(); // ':'
            } else {
                Logger::error("expected ':' after case expression");
            }

            if (!(case_b = parse_stmt())) Logger::error("expected statement");

            cases.push_back(std::make_unique<CaseStmt>(
                since(case_s), std::move(case_m), std::move(case_b)
            ));
        } else if (match("default")) {
            if (def) Logger::error("more than one default statement in 'switch'");

            next(); // 'default'

            if (match(TokenKind::Colon)) {
                next(); // ':'
            } else {
                Logger::error("expected ':' after 'default'");
            }

            if (!(def = parse_stmt())) Logger::error("expected statement");
        }

        if (match(TokenKind::EndBrace)) break;

        while (match(TokenKind::Semi)) next(); // ';'
    }

    next(); // '}'

    if (!cases.empty()) cases.shrink_to_fit();
    
    return std::make_unique<SwitchStmt>(
        since(start), std::move(mtch), cases, std::move(def)
    );
}

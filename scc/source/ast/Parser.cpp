//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "ast/Parser.hpp"
#include "ast/Decl.hpp"
#include "ast/Expr.hpp"
#include "ast/Stmt.hpp"
#include "ast/Type.hpp"
#include "ast/TypeContext.hpp"
#include "core/Logger.hpp"
#include "core/SourceSpan.hpp"
#include "lexer/Token.hpp"

#include <string>
#include <unordered_set>

using namespace scc;

using std::unordered_set;

Parser::Parser(const string& file, const string& source) 
    : m_file(file), m_lexer(file, source) {}

TranslationUnitDecl* Parser::parse() {
    next();

    m_unit = new TranslationUnitDecl(m_file);
    m_tctx = &m_unit->get_context();
    m_dctx = m_unit;

    while (!m_lexer.is_eof())
        parse_decl();

    m_tctx = nullptr;
    m_dctx = nullptr;
    return m_unit;
}

bool Parser::match(TokenKind kind) const {
    return m_lexer.last().kind == kind;
}

bool Parser::match(const char* kw) const {
    const Token& token = m_lexer.last();
    return token.kind == TokenKind::Identifier && kw == token.value;
}

bool Parser::expect(TokenKind kind) {
    const Token& token = m_lexer.last();
    
    if (token.kind == kind) {
        next();
        return true;
    } else return false;
}

void Parser::next() {
    m_lexer.lex();
}

void Parser::skip(uint32_t n) {
    for (uint32_t i = 0; i < n; ++i)
        m_lexer.lex();
}

SourceSpan Parser::since(const SourceLocation& loc) const {
    return SourceSpan(loc, m_lexer.last().loc);
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

bool Parser::is_reserved(const string& ident) const {
    static unordered_set<string> keywords = {
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

void Parser::check_reserved(
        const SourceLocation& loc, const string& ident) const {
    if (is_reserved(ident))
        Logger::error("identifier '" + ident + "' is reserved", since(loc));
}

bool Parser::is_storage_class(const string& ident) const {
    static unordered_set<string> keywords = {
        "auto", "extern", "register", "static",
    };

    return keywords.contains(ident);
}

bool Parser::is_typedef(const string& ident) const {
    // No named types yet, so only have to consider primitives.

    static unordered_set<string> keywords = {
        "auto", "char", "const", "double",
        "enum", "float", "int", "long",
        "short", "signed", "struct", "typedef",
        "union", "unsigned", "void", "volatile",  
    };

    return keywords.contains(ident);
}

TagTypeDecl::TagKind Parser::get_tag_kind(const string& ident) const {
    if (ident == "struct") {
        return TagTypeDecl::Struct;
    } else if (ident == "union") {
        return TagTypeDecl::Union;
    } else if (ident == "enum") {
        return TagTypeDecl::Enum;
    }

    assert(false && "identifier is not a tag keyword!");
}

StorageClass Parser::parse_storage_class() {
    if (!match(TokenKind::Identifier))
        return StorageClass::None;

    static unordered_map<string, StorageClass> classes = {
        { "auto", StorageClass::Auto },
        { "extern", StorageClass::Extern },
        { "register", StorageClass::Register },
        { "static", StorageClass::Static },
    };

    const string value = m_lexer.last().value;
    if (classes.contains(value)) {
        next();
        return classes[value];
    }

    return StorageClass::None;
}

void Parser::parse_type(QualType& type) {
    SourceLocation start = curr().loc;
    
    if (match("const")) {
        type.with_const();
        next();
    } else if (match("volatile")) {
        type.with_volatile();
        next();
    }

    if (match("volatile")) {
        if (type.is_volatile()) {
            Logger::warn("type already qualified with 'volatile', ignoring", 
                since(m_lexer.last(1).loc));
        } else {
            type.with_volatile();
        }

        next();
    }

    if (!match(TokenKind::Identifier))
        Logger::error("expected identifier", since(start));

    const Type* unqual = nullptr;
    if (is_tag(curr().value)) {
        string tag = curr().value;
        next(); // tag

        if (!match(TokenKind::Identifier))
            Logger::error("expected identifier", since(start));

        string tag_name = curr().value;
        next(); // identifier

        if (tag_name == "enum")
            Logger::error("cannot forward declare 'enum'", since(start));

        RecordDecl* tag_decl = static_cast<RecordDecl*>(m_dctx->get_tag(tag_name));
        if (!tag_decl) {
            TagTypeDecl::TagKind kind = get_tag_kind(tag);

            tag_decl = new RecordDecl(
                m_unit, 
                since(start), 
                tag_name, 
                nullptr, 
                kind);

            tag_decl->set_type(RecordType::create(*m_tctx, tag_decl));
            unqual = tag_decl->get_type();
        }
    } else if (is_reserved(curr().value)) {
        string base = "";
        while (match(TokenKind::Identifier) && is_typedef(curr().value)) {
            base += curr().value + ' ';
            next();
        }

        if (!base.empty())
            base = base.substr(0, base.size() - 1);

        unordered_map<string, const Type*> primitives = {
            { "void", BuiltinType::get_void_type(*m_tctx) },
            { "char", BuiltinType::get_char_type(*m_tctx) },
            { "unsigned char", BuiltinType::get_uchar_type(*m_tctx) },
            { "short", BuiltinType::get_short_type(*m_tctx) },
            { "unsigned short", BuiltinType::get_ushort_type(*m_tctx) },
            { "int", BuiltinType::get_int_type(*m_tctx) },
            { "unsigned int", BuiltinType::get_uint_type(*m_tctx) },
            { "long", BuiltinType::get_long_type(*m_tctx) },
            { "unsigned long", BuiltinType::get_ulong_type(*m_tctx) },
            { "long long", BuiltinType::get_longlong_type(*m_tctx) },
            { "unsigned long long", BuiltinType::get_ulonglong_type(*m_tctx) },
            { "float", BuiltinType::get_float_type(*m_tctx) },
            { "double", BuiltinType::get_double_type(*m_tctx) },
            { "long double", BuiltinType::get_longdouble_type(*m_tctx) },
        };

        if (primitives.count(base) != 0) {
            unqual = primitives[base];
        } else {
            Logger::error("expected type", since(start));
        }
    } else {
        // Type identifier is some typedef.
        NamedDecl* nd = m_dctx->get_decl(curr().value);
        if (!nd)
            Logger::error("unresolved type: '" + curr().value + "'", since(start));

        TypeDecl* td = dynamic_cast<TypeDecl*>(nd);
        if (!td)
            Logger::error("expected type", since(start));

        unqual = td->get_type();
        next(); // identifier
    }

    while (match(TokenKind::Star)) {
        unqual = PointerType::get(*m_tctx, unqual);
        next();
    }

    type.set_type(unqual);
}

Decl* Parser::parse_decl() {
    if (match("typedef")) {
        return parse_typedef();
    } else if (match("struct") || match("union")) {
        return parse_record();
    } else if (match("enum")) {
        return parse_enum();
    }

    SourceLocation start = m_lexer.last().loc;
    StorageClass storage = None;
    string name;
    QualType type;

    // Attempt to parse any preceeding storage class.
    storage = parse_storage_class();

    // Attempt to parse a declaration type.
    if (storage != StorageClass::Auto)
        parse_type(type);
    
    if (!match(TokenKind::Identifier))
        Logger::error("missing identifier", since(start));

    name = m_lexer.last().value;
    next(); // identifier

    check_reserved(start, name);

    if (match(TokenKind::Semi) || match(TokenKind::Eq) || match(TokenKind::SetBrack)) {
        // The identifier is followed by a ';' or '=' or '[', which means its a 
        // variable in the form of:
        //
        // <type> <ident> ';'
        //       or
        // <type> <ident> = ...
        //       or
        // <type> <ident>[...]
        return parse_variable(start, storage, type, name);
    } else if (match(TokenKind::SetParen)) {
        // The identifier is followed by a '(', which means its the beginning
        // of a function parameter list.

        // TEMPORARY: Disallow auto classed functions.
        if (storage == StorageClass::Auto)
            Logger::error("function cannot be marked with 'auto' keyword", since(start));

        return parse_function(start, storage, type, name);
    } else {
        // No declaration pattern matches.
        Logger::error("expected declaration after identifier", since(start));
    }
}

Decl* Parser::parse_function(
        const SourceLocation& start, StorageClass storage, 
        QualType ret_type, const string& name) {
    next(); // '('

    FunctionDecl* decl = nullptr;
    bool existing = false;

    if (NamedDecl* nd = m_dctx->get_decl(name)) {
        decl = dynamic_cast<FunctionDecl*>(nd);
        if (!decl)
            Logger::error("redefiniton of '" + name + "'", since(start));

        if (decl->get_storage_class() != storage)
            Logger::error("conflicting storage classes for '" + name + "'", 
                since(start));

        existing = true;
    } else {
        decl = new FunctionDecl(m_dctx, start, name, QualType(), storage);
    }

    m_dctx = decl;

    vector<ParameterDecl*> params = {};

    // Reserve some space if there are going to be parameters.
    if (!match(TokenKind::EndParen)) 
        params.reserve(6);

    while (!match(TokenKind::EndParen)) {
        SourceLocation p_start = m_lexer.last().loc;
        string p_name;
        QualType p_type;

        parse_type(p_type);

        if (!match(TokenKind::Identifier))
            Logger::error("expected identifier", since(start));

        p_name = m_lexer.last().value;
        next(); // identifier

        check_reserved(p_start, p_name);

        while (expect(TokenKind::SetBrack)) {
            // TODO: Support automatic array size checks.
            if (!match(TokenKind::Integer))
                Logger::error("expected integer after '['", since(start));

            // TODO: Switch to signed and check for invalid negative sizes.
            uint32_t size = std::stoul(m_lexer.last().value);
            next(); // size

            if (!expect(TokenKind::EndBrack))
                Logger::error("expected ']'");

            p_type = QualType(ArrayType::get(*m_tctx, p_type, size));
        }

        params.push_back(new ParameterDecl(m_dctx, p_start, p_name, p_type));

        if (match(TokenKind::EndParen))
            break;

        if (!expect(TokenKind::Comma))
            Logger::error("expected ','", since(start));
    }

    next(); // ')'

    if (existing) {
        // If this is a redeclaration of a function, then we need to check that
        // the type signatures match.

        const FunctionType* f_type = static_cast<const FunctionType*>(
            decl->get_type().get_type());

        if (f_type->get_return_type() != ret_type)
            Logger::error("conflicting types for '" + name + "'; have '" + 
                decl->get_type().to_string() + "'", start);

        if (decl->num_params() != params.size())
            Logger::error("conflicting types for '" + name + "'; have '" + 
                decl->get_type().to_string() + "'", start);
        
        for (uint32_t i = 0; i < params.size(); ++i) {
            if (f_type->get_param_type(i) != params[i]->get_type())
                Logger::error("conflicting types for '" + name + "'; have '" + 
                    decl->get_type().to_string() + "'", start);
        }

        // The parameter set already exists, so we can delete the new one.
        // TODO: Potentially update the source locations to the new parameters.
        for (ParameterDecl* param : params)
            delete param;

        params.clear();
    } else {
        // Since we reserved extra space if there was at least one parameter, 
        // we can shrink it now since it won't ever change.
        if (!params.empty())
            params.shrink_to_fit();

        vector<QualType> p_types(params.size(), QualType());
        for (uint32_t i = 0; i < params.size(); ++i)
            p_types[i] = params[i]->get_type();

        decl->set_params(params);
        decl->get_type().set_type(FunctionType::get(*m_tctx, ret_type, p_types));
    }

    if (match(TokenKind::Semi)) {
        next(); // ';'
    } else if (existing && decl->has_body()) {
        Logger::error("redefinition of '" + name + "'", since(start));
    } else {
        Stmt* body = parse_stmt();
        if (!body)
            Logger::error("expected statement", since(start));
        
        decl->set_body(body);
    }

    m_dctx = m_dctx->get_parent();
    return decl;
}

Decl* Parser::parse_variable(
        const SourceLocation& start, StorageClass storage, QualType type, 
        const string& name) {
    while (match(TokenKind::SetBrack)) {
        next(); // '['

        // TODO: Support automatic array size checks.
        if (!match(TokenKind::Integer))
            Logger::error("expected integer", since(start));

        uint32_t size = std::stoul(m_lexer.last().value);
        next(); // integer

        if (!expect(TokenKind::EndBrack))
            Logger::error("expected ']'");

        type = QualType(ArrayType::get(*m_tctx, type, size));
    }

    Expr* init = nullptr;
    if (expect(TokenKind::Eq)) {
        if (!(init = parse_expr()))
            Logger::error("expected expression", since(start));
    }

    if (storage == StorageClass::Auto) {
        if (!init)
            Logger::error("variable marked 'auto' but missing initializer");

        if (type.get_type() != nullptr)
            Logger::error("variable marked 'auto', but type provided");

        type = init->get_type();
    }

    return new VariableDecl(
        m_dctx,
        start,
        name,
        type,
        storage, 
        !m_dctx->has_parent(),
        init);
}

Decl* Parser::parse_typedef() {
    SourceLocation start = m_lexer.last().loc;
    QualType underlying;
    string name;

    next(); // 'typedef'

    parse_type(underlying);

    if (!match(TokenKind::Identifier))
        Logger::error("expected identifier", since(start));

    name = m_lexer.last().value;
    next(); // identifier

    check_reserved(start, name);

    if (!expect(TokenKind::Semi))
        Logger::error("expected ';'", since(start));

    TypedefDecl* decl = new TypedefDecl(m_dctx, start, name, nullptr);
    decl->set_type(TypedefType::create(*m_tctx, decl, underlying));
    return decl;
}

Decl* Parser::parse_record() {
    RecordDecl* decl = nullptr;
    SourceLocation start = curr().loc;
    string name = "";
    vector<FieldDecl*> fields = {};
    bool is_struct = match("struct");
    bool empty = true;
    bool existing = false;

    next(); // 'struct' || 'union'
   
    if (match(TokenKind::Identifier)) {
        name = curr().value;
        check_reserved(start, name);
        next(); // identifier
    }

    // Check for an existing tag declaration with the same name.
    if (TagTypeDecl* nd = m_unit->get_tag(name)) {
        if (!(decl = dynamic_cast<RecordDecl*>(nd)))
            Logger::error("redefiniton of '" + name + "'", since(start));

        // There is an existing record with the same name, so we have to run
        // some special logic to see if theres no conflicts.
        existing = true;
    } else {
        decl = new RecordDecl(
            m_unit, 
            start, 
            name, 
            nullptr, 
            is_struct ? TagTypeDecl::Struct : TagTypeDecl::Union);
    }

    m_dctx = decl;

    // If the struct is opened with a '{', it should get closed at some point.
    if (expect(TokenKind::SetBrace)) {
        if (existing && !decl->empty())
            Logger::error("redefinition of '" + name + "'", since(start));

        fields.reserve(4);

        while (!match(TokenKind::EndBrace)) {
            SourceLocation f_start = curr().loc;
            string f_name;
            QualType f_type;

            parse_type(f_type);

            if (!match(TokenKind::Identifier))
                Logger::error("expected identifier", since(f_start));

            // Parse the field name and check that it's not reserved.
            f_name = curr().value;
            check_reserved(f_start, f_name);
            next(); // identifier

            fields.push_back(new FieldDecl(m_dctx, f_start, f_name, f_type));

            if (match(TokenKind::EndBrace)) 
                break;

            if (!expect(TokenKind::Semi))
                Logger::error("expected ';'", since(f_start));
        }

        if (!expect(TokenKind::EndBrace))
            Logger::error("expected '}'", since(start));
    }
    
    if (!expect(TokenKind::Semi))
        Logger::error("expected ';'", since(start));

    if (!fields.empty()) {
        fields.shrink_to_fit();
        
        decl->set_fields(fields);
        decl->set_type(RecordType::create(*m_tctx, decl));
    }

    m_dctx = m_dctx->get_parent();
    return decl;
}

Decl* Parser::parse_enum() {
    SourceLocation start = curr().loc;
    string name = "";
    vector<EnumVariantDecl*> variants = {};
    EnumDecl* decl = nullptr;

    next(); // 'enum'
   
    if (match(TokenKind::Identifier)) {
        name = curr().value;
        check_reserved(start, name);
        next(); // identifier
    }

    // Check for redefinition of an existing symbol, if this isn't an unnamed
    // enum.
    if (!name.empty() && m_dctx->get_decl(name))
        Logger::error("redefinition of '" + name + "'", since(start));

    decl = new EnumDecl(m_dctx, start, name, nullptr);
    decl->set_type(EnumType::create(*m_tctx, decl));

    m_dctx = decl;

    // Enums do not support forward declarations.
    if (!expect(TokenKind::SetBrace))
        Logger::error("expected '{'", since(start));

    if (!match(TokenKind::EndBrace)) 
        variants.reserve(4);

    int32_t value = 0;
    while (!match(TokenKind::EndBrace)) {
        SourceLocation v_start = curr().loc;
        string v_name;

        if (!match(TokenKind::Identifier))
            Logger::error("expected identifier", since(v_start));

        v_name = curr().value;
        check_reserved(v_start, v_name);
        next(); // identifier

        if (expect(TokenKind::Eq)) {
            bool neg = expect(TokenKind::Minus);

            if (match(TokenKind::Integer)) {
                value = std::stod(curr().value);
                next(); // integer
            } else {
                Logger::error("expected integer after '='", since(v_start));
            }

            if (neg)
                value = -value;
        }

        variants.push_back(new EnumVariantDecl(
            m_dctx, 
            v_start, 
            v_name, 
            name.empty() ? BuiltinType::get_int_type(*m_tctx) : decl->get_type(), 
            value++
        ));

        if (match(TokenKind::EndBrace)) 
            break;

        if (!expect(TokenKind::Comma))
            Logger::error("expected ','", since(v_start));
    }

    next(); // '}'

    if (variants.empty()) {
        Logger::warn("'enum' declaration is empty", since(start));
    } else {
        variants.shrink_to_fit();
    }

    decl->set_variants(variants);

    m_dctx = m_dctx->get_parent();
    return decl;
}

Expr* Parser::parse_expr() {
    Expr* base = parse_unary_prefix();
    if (!base)
        Logger::error("expected expression");
    
    base = parse_binary(base, 0);
    if (!base)
        Logger::error("expected expression");

    if (match(TokenKind::Question)) {
        return parse_ternary(base);
    } else {
        return base;
    }
}

Expr* Parser::parse_primary() {
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
        SourceLocation start = curr().loc;
        Expr* expr = nullptr;
        next(); // '('

        if (match(TokenKind::Identifier) && is_typedef(curr().value)) {
            QualType type;
            parse_type(type);

            if (!expect(TokenKind::EndParen))
                Logger::error("expected ')'", since(start));

            if (!(expr = parse_expr()))
                Logger::error("expected expression", since(start));

            return new CastExpr(start, type, expr);
        } else {
            if (!(expr = parse_expr()))
                Logger::error("expected expression", since(start));

            if (!expect(TokenKind::EndParen))
                Logger::error("expected ')'", since(start));

            return new ParenExpr(start, expr->get_type(), expr);
        }
    }
    
    return nullptr;
}

Expr* Parser::parse_integer() {
    Token integer = curr();
    next(); // integer

    const Type* type = nullptr;
    if (!match(TokenKind::Identifier)) {
        // Default to 'int' type.
        type = BuiltinType::get_int_type(*m_tctx);
    } else if (match("u") || match("U")) {
        type = BuiltinType::get_uint_type(*m_tctx);
        next(); // 'u' || 'U'
    } else if (match("l") || match("L")) {
        type = BuiltinType::get_long_type(*m_tctx);
        next(); // 'l' || 'L'
    } else if (match("ul") || match("UL")) {
        type = BuiltinType::get_ulong_type(*m_tctx);
        next(); // 'ul' || 'UL'
    } else if (match("ll") || match("LL")) {
        type = BuiltinType::get_longlong_type(*m_tctx);
        next(); // 'll' || 'LL'
    } else if (match("ull") || match("ULL")) {
        type = BuiltinType::get_ulonglong_type(*m_tctx);
        next(); // 'ull' || 'ULL'
    }

    return new IntegerLiteral(
        since(integer.loc), type, std::stol(integer.value));
}

Expr* Parser::parse_float() {
    Token fp = curr();
    next(); // float

    const Type* type = nullptr;
    if (!match(TokenKind::Identifier)) {
        // Default to 'double' type.
        type = BuiltinType::get_double_type(*m_tctx);
    } else if (match("f") || match("F")) {
        type = BuiltinType::get_float_type(*m_tctx);
        next(); // 'f' || 'F'
    }

    return new FPLiteral(since(fp.loc), type, std::stod(fp.value));
}

Expr* Parser::parse_character() {
    Token ch = curr();
    next(); // character

    return new CharLiteral(
        ch.loc, BuiltinType::get_char_type(*m_tctx), ch.value[0]);
}

Expr* Parser::parse_string() {
    Token str = curr();
    next(); // string

    // Get the 'const char*' type.
    QualType ty(PointerType::get(*m_tctx, BuiltinType::get_char_type(*m_tctx)));
    ty.with_const();

    return new StringLiteral(since(str.loc), ty, str.value);
}

Expr* Parser::parse_binary(Expr* base, int32_t precedence) {
    while (1) {
        Token last = curr();

        int32_t token_prec = get_binary_operator_precedence(last.kind);
        if (token_prec < precedence) 
            break;

        BinaryExpr::Op op = get_binary_operator(last.kind);
        if (op == BinaryExpr::Unknown) 
            break;

        next(); // operator
        
        Expr* right = parse_unary_prefix();
        if (!right)
            Logger::error("expected right side expression", since(last.loc));

        int32_t next_prec = get_binary_operator_precedence(m_lexer.last().kind);
        if (token_prec < next_prec) {
            right = parse_binary(right, precedence + 1);
            if (!right)
                Logger::error("expected right side expression", since(last.loc));
        }

        base = new BinaryExpr(
            SourceSpan(base->get_starting_loc(), right->get_ending_loc()),
            base->get_type(),
            op,
            base,
            right);
    }

    return base;
}

Expr* Parser::parse_unary_prefix() {
    UnaryExpr::Op op = get_unary_operator(m_lexer.last().kind);
    
    if (UnaryExpr::is_prefix_op(op)) {
        SourceLocation start = m_lexer.last().loc;
        next(); // operator

        Expr* base = parse_unary_prefix();
        if (!base) 
            Logger::error("expected expression", since(start));

        return new UnaryExpr(since(start), base->get_type(), op, false, base);
    } else {
        return parse_unary_postfix();
    }
}

Expr* Parser::parse_unary_postfix() {
    Expr* base = parse_primary();
    if (!base) 
        Logger::error("expected expression", since(curr().loc));

    while (1) {
        SourceLocation start = curr().loc;
        UnaryExpr::Op op = get_unary_operator(m_lexer.last().kind);

        if (UnaryExpr::is_postfix_op(op)) {
            next(); // operator
            base = new UnaryExpr(
                since(start), 
                base->get_type(), 
                op, 
                true, 
                base);
        } else if (expect(TokenKind::SetParen)) {
            vector<Expr*> args = {};
            if (!match(TokenKind::EndParen)) 
                args.reserve(2);

            while (!match(TokenKind::EndParen)) {
                Expr* arg = parse_expr();
                if (!arg)
                    Logger::error("expected expression", since(start));
                
                args.push_back(arg);

                if (match(TokenKind::EndParen)) 
                    break;

                if (!expect(TokenKind::Comma))
                    Logger::error("expected ','", since(start));
            }

            next(); // ')'

            if (!args.empty()) 
                args.shrink_to_fit();

            base = new CallExpr(since(start), base->get_type(), base, args);
        } else if (expect(TokenKind::SetBrack)) {
            Expr* index = parse_expr();
            if (!index) 
                Logger::error("expected expression after '['", since(start));

            if (!expect(TokenKind::EndBrack))
                Logger::error("expected ']'", since(start));

            base = new SubscriptExpr(since(start), base->get_type(), base, index);
        } else if (match(TokenKind::Dot) || match(TokenKind::Arrow)) {
            bool arrow = match(TokenKind::Arrow);
            next(); // '.' || '->'

            const ValueDecl* member = nullptr;

            if (match(TokenKind::Identifier)) {
                const QualType& base_type = base->get_type();
                // TODO: Determine member based on base structure type.
            } else {
                Logger::error("expected identifier after '" + 
                    std::string(arrow ? "->" : ".") + "'", since(start));
            }

            base = new MemberExpr(
                since(start), member->get_type(), base, member, arrow);
        } else {
            break;
        }
    }

    return base;
}

Expr* Parser::parse_ref() {
    Token ident = m_lexer.last();
    next(); // identifier

    const NamedDecl* nd = m_dctx->get_decl(ident.value);
    if (!nd)
        Logger::error("unresolved reference: '" + ident.value + "'", 
            since(ident.loc));

    const ValueDecl* vd = dynamic_cast<const ValueDecl*>(nd);
    if (!vd)
        Logger::error("expected value", since(ident.loc));

    return new RefExpr(since(ident.loc), vd->get_type(), vd);
}

Expr* Parser::parse_sizeof() {
    SourceLocation start = m_lexer.last().loc;
    next(); // 'sizeof'

    if (!expect(TokenKind::SetParen))
        Logger::error("missing '(' after 'sizeof' keyword");

    QualType type;
    parse_type(type);

    if (!expect(TokenKind::EndParen))
        Logger::error("missing ')' after 'sizeof' type");

    // Use 'unsigned long' for the sizeof result.
    return new SizeofExpr(
        since(start), BuiltinType::get_ulong_type(*m_tctx), type);
}

Expr* Parser::parse_ternary(Expr* base) {
    next(); // '?'

    Expr* tval = nullptr;
    Expr* fval = nullptr;

    // Parse the true value: '?' ... ':'
    if (!(tval = parse_expr())) 
        Logger::error("expected expression");

    if (!expect(TokenKind::Colon))
        Logger::error("expected ':' after ternary specifier");

    /// Parse the false value: ':' ...
    if (!(fval = parse_expr())) 
        Logger::error("expected expression");

    return new TernaryExpr(
        since(base->get_starting_loc()), tval->get_type(), base, tval, fval);
}

Stmt* Parser::parse_stmt() {
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
        SourceLocation start = curr().loc;
        next(); // 'break'
        return new BreakStmt(since(start));
    } else if (match("continue")) {
        SourceLocation start = curr().loc;
        next(); // 'continue'
        return new ContinueStmt(since(start));
    } else if (match(TokenKind::Identifier) && 
      (is_storage_class(curr().value) || is_typedef(curr().value))) {
        SourceLocation start = curr().loc;
        vector<const Decl*> decls = {};

        while (!match(TokenKind::Semi)) {
            Decl* decl = parse_decl();
            if (!decl)
                Logger::error("expected declaration", since(start));

            decls.push_back(decl);
        }

        next(); // ';'
        return new DeclStmt(start, decls);
    }

    // Fallback to an expression statement.
    SourceLocation start = curr().loc;
    Expr* expr = parse_expr();
    if (!expr) 
        Logger::error("expected expression", since(start));

    return new ExprStmt(expr->get_span(), expr);
}

Stmt* Parser::parse_compound() {
    SourceLocation start = curr().loc;
    vector<Stmt*> stmts = {};
    next(); // '{'
    
    // This compound won't be empty, so reserve some space.
    if (!match(TokenKind::EndBrace)) 
        stmts.reserve(4);

    while (!match(TokenKind::EndBrace)) {
        Stmt* stmt = parse_stmt();
        if (!stmt) 
            Logger::error("expected statement", since(start));

        stmts.push_back(stmt);

        if (match(TokenKind::EndBrace)) 
            break;

        while (expect(TokenKind::Semi));
    }

    next(); // '}'

    if (!stmts.empty()) 
        stmts.shrink_to_fit();

    return new CompoundStmt(since(start), stmts);
}

Stmt* Parser::parse_if() {
    SourceLocation start = curr().loc;
    next(); // 'if'

    Expr* cond = nullptr;
    Stmt* then = nullptr;
    Stmt* els = nullptr;

    if (!expect(TokenKind::SetParen))
        Logger::error("expected '(' after 'if'", since(start));

    if (!(cond = parse_expr()))
        Logger::error("expected expression after '('", since(start));

    if (!expect(TokenKind::EndParen))
        Logger::error("missing ')' after 'if' condition", since(start));

    if (!(then = parse_stmt())) 
        Logger::error("expected statement", since(start));

    if (match("else")) {
        next(); // 'else'

        if (!(els = parse_stmt()))
            Logger::error("expected statement after 'else'", since(start));
    }

    return new IfStmt(since(start), cond, then, els);
}

Stmt* Parser::parse_return() {
    SourceLocation start = curr().loc;
    next(); // 'return'

    Expr* expr = nullptr;
    if (!expect(TokenKind::Semi)) {
        expr = parse_expr();
        if (!expr)
            Logger::error("expected expression", since(start));

        if (!expect(TokenKind::Semi))
            Logger::error("expected ';'", since(start));
    }

    return new ReturnStmt(since(start), expr);
}

Stmt* Parser::parse_while() {
    SourceLocation start = curr().loc;
    next(); // 'while'

    Expr* cond = nullptr;
    Stmt* body = nullptr;

    if (!expect(TokenKind::SetParen))
        Logger::error("missing '(' after 'while' keyword", since(start));

    if (!(cond = parse_expr()))
        Logger::error("expected expression", since(start));

    if (!expect(TokenKind::EndParen))
        Logger::error("missing ')' after 'while' condition", since(start));

    if (!expect(TokenKind::Semi)) {
        if (!(body = parse_stmt()))
            Logger::error("expected statement", since(start));
    }

    return new WhileStmt(since(start), cond, body);
}

Stmt* Parser::parse_for() {
    SourceLocation start = curr().loc;
    next(); // 'while'

    Stmt* init = nullptr;
    Expr* cond = nullptr;
    Expr* step = nullptr;
    Stmt* body = nullptr;

    if (!expect(TokenKind::SetParen))
        Logger::error("expected '(' after 'for'", since(start));

    // Parse the 'for' initializer: for (... ';'
    if (!expect(TokenKind::Semi))
        if (!(init = parse_stmt()))
            Logger::error("expected statement", since(start));

    // Parse the 'for' stop condition: ';' ... ';'
    if (!match(TokenKind::Semi))
        if (!(cond = parse_expr())) 
            Logger::error("expected expresion", since(start));

    // Eat any semicolon after the stop condition.
    if (match(TokenKind::Semi)) 
        next(); // ';'

    // Parse the 'for' step: ';' ... ')'
    if (!match(TokenKind::EndParen))
        if (!(step = parse_expr())) 
            Logger::error("expected expression", since(start));

    if (!expect(TokenKind::EndParen))
        Logger::error("expected ')' after 'for' specifier", since(start));

    if (!match(TokenKind::Semi))
        if (!(body = parse_stmt())) 
            Logger::error("expected statement", since(start));

    return new ForStmt(since(start), init, cond, step, body);
}

Stmt* Parser::parse_switch() {
    SourceLocation start = curr().loc;
    Expr* expr = nullptr;
    Stmt* def = nullptr;
    vector<CaseStmt*> cases = {};
    next(); // 'switch'    

    if (!expect(TokenKind::SetParen))
        Logger::error("expected '(' after 'switch'", since(start));
    
    if (!(expr = parse_expr())) 
        Logger::error("expected expression", since(start));

    if (!expect(TokenKind::EndParen))
        Logger::error("expected ')' after 'switch' specifier", since(start));

    if (!expect(TokenKind::SetBrace))
        Logger::error("expected '{' after 'switch' specifier");

    cases.reserve(4);
    while (!match(TokenKind::EndBrace)) {
        if (match("case")) {
            SourceLocation c_start = curr().loc;
            Expr* c_match = nullptr;
            Stmt* c_body = nullptr;
            next(); // 'case'

            if (!(c_match = parse_expr()))
                Logger::error("expected expression", since(c_start));

            if (!expect(TokenKind::Colon))
                Logger::error("expected ':'", since(c_start));

            if (!(c_body = parse_stmt())) 
                Logger::error("expected statement", since(c_start));

            cases.push_back(new CaseStmt(since(c_start), c_match, c_body));
        } else if (match("default")) {
            if (def)
                Logger::error("multiple defaults in 'switch'", since(start));

            next(); // 'default'

            if (!expect(TokenKind::Colon)) 
                Logger::error("expected ':'", since(start));

            if (!(def = parse_stmt())) 
                Logger::error("expected statement", since(start));
        }

        if (match(TokenKind::EndBrace)) 
            break;

        while (expect(TokenKind::Semi));
    }

    next(); // '}'

    if (!cases.empty())
        cases.shrink_to_fit();
    
    return new SwitchStmt(since(start), expr, cases, def);
}

//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SCC_DECL_H_
#define SCC_DECL_H_

//
// This header file declares the base Decl class and all other recognized C
// declarations; both top-level and nested.
//

#include "ast/QualType.hpp"
#include "ast/Scope.hpp"
#include "ast/Type.hpp"
#include "core/Span.hpp"

#include <cassert>
#include <vector>

namespace scc {

using std::ostream;
using std::string;
using std::vector;

class Expr;
class Stmt;

class NamedDecl;

/// Possible kinds of storage classes in C.
enum StorageClass : uint32_t {
    None,
    Auto,
    Register,
    Static,
    Extern,
};

/// Base class for all declaration nodes in the abstract syntax tree.
class Decl {
public:
    /// Possible kinds of C declarations.
    enum Kind : uint32_t {
        TranslationUnit,
        Variable,
        Parameter,
        Function,
        Typedef,
        Field,
        Record,
        EnumVariant,
        Enum,
    };

protected:
    /// The kind of declaration of this is.
    const Kind m_kind;

    /// The span of source code this declaration covers.
    Span m_span;

    explicit Decl(Kind kind, const Span& span) : m_kind(kind), m_span(span) {}

public:
    virtual ~Decl() = default;

    /// Returns the kind of declaration this is.
    Kind get_kind() const { return m_kind; }
    
    /// Returns the span of source code this declaration covers.
    const Span& get_span() const { return m_span; }
    Span& get_span() { return m_span; }

    /// Pretty-print this declaration node to the output stream \p os.
    virtual void print(ostream& os) const = 0;
};

/// Base class for all declarations that may have a name.
class NamedDecl : public Decl {
protected:
    /// The name of this declaration.
    string m_name;

    explicit NamedDecl(Kind kind, const Span& span, const string& name)
        : Decl(kind, span), m_name(name) {}

public:
    virtual ~NamedDecl() = default;

    /// Returns the name of this declaration.
    const string& get_name() const { return m_name; }
    string& get_name() { return m_name; }
};

/// Base class for all named declarations that produce a value.
class ValueDecl : public NamedDecl {
protected:
    /// The type of this declaration.
    QualType m_type;

    explicit ValueDecl(Kind kind, const Span& span, const string& name, 
                       const QualType& type)
        : NamedDecl(kind, span, name), m_type(type) {}

public:
    virtual ~ValueDecl() = default;

    /// Returns the type of this declaration.
    const QualType& get_type() const { return m_type; }
    QualType& get_type() { return m_type; }
};

/// Base class for all contexts which may contain declarations.
class DeclContext {
protected:
    /// The parent context to this context, if there is one.
    DeclContext* m_parent;

    /// The declarations in this context.
    vector<Decl*> m_decls;

    DeclContext(DeclContext* parent = nullptr) : m_parent(parent), m_decls() {}

    DeclContext(DeclContext* parent, const vector<Decl*>& decls) 
        : m_parent(parent), m_decls(decls) {}

public:
    ~DeclContext();
    
    /// Returns true if this context has a parent context.
    bool has_parent() const { return m_parent != nullptr; }

    /// Returns the parent of this context, if there is one.
    const DeclContext* get_parent() const { return m_parent; }
    DeclContext* get_parent() { return m_parent; }

    /// Returns the number of declarations in this context.
    uint32_t num_decls() const { return m_decls.size(); }

    /// Returns true if this context does not contain any declarations.
    bool empty() const { return m_decls.empty(); }

    /// Returns the declarations in this context.
    const vector<Decl*>& get_decls() const { return m_decls; }
    vector<Decl*>& get_decls() { return m_decls; }

    /// Returns the declaration with the name \p name in this context if it
    /// there is one, and 'nullptr' otherwise. 
    const NamedDecl* get_decl(const string& name) const;
    NamedDecl* get_decl(const string& name) {
        return const_cast<NamedDecl*>(
            static_cast<const DeclContext*>(this)->get_decl(name));
    }

    /// Add a new declaration \p decl to this context.
    void add(Decl* decl) { m_decls.push_back(decl); }
};

/// Represents a translation unit declaration (a source file).
class TranslationUnitDecl final : public DeclContext, public Decl {
public:
    TranslationUnitDecl()
        : DeclContext(), Decl(Kind::TranslationUnit, Span()) {}

    TranslationUnitDecl(const TranslationUnitDecl&) = delete;
    TranslationUnitDecl& operator = (const TranslationUnitDecl&) = delete;

    void print(ostream& os) const;
};

/// Represents a variable declaration, either global or local.
class VariableDecl final : public ValueDecl {
    /// The storage class of this variable.
    StorageClass m_sclass;

    /// The initializing expression of this variable, if there is one.
    Expr* m_init = nullptr;

public:
    VariableDecl(const Span& span, const string& name, const QualType& type, 
                 StorageClass sclass, Expr* init)
        : ValueDecl(Kind::Variable, span, name, type), m_sclass(sclass), 
          m_init(init) {}

    VariableDecl(const VariableDecl&) = delete;
    VariableDecl& operator = (const VariableDecl&) = delete;

    ~VariableDecl();

    /// Returns the storage class of this variable.
    StorageClass get_storage_class() const { return m_sclass; }

    /// Returns true if this variable has an initializing expression.
    bool has_init() const { return m_init != nullptr; }

    /// Returns the initializing expression of this variable, if there is one.
    const Expr* get_init() const { return m_init; }
    Expr* get_init() { return m_init; }

    void print(ostream& os) const override;
};

/// Represents a parameter declaration within a function parameter list.
class ParameterDecl final : public ValueDecl {
public:
    ParameterDecl(const Span& span, const string& name, const QualType& type)
        : ValueDecl(Kind::Parameter, span, name, type) {}

    ParameterDecl(const ParameterDecl&) = delete;
    ParameterDecl& operator = (const ParameterDecl&) = delete;

    void print(ostream& os) const override;
};

/// Represents a function declaration.
class FunctionDecl final : public DeclContext, public ValueDecl {
    /// The storage class of this function.
    StorageClass m_sclass;

    /// The body of this function, if there is one.
    Stmt* m_body;

public:
    FunctionDecl(DeclContext* dctx, const Span& span, const string& name, 
                 const QualType& type, StorageClass sclass, Stmt* body)
        : DeclContext(dctx), ValueDecl(Kind::Function, span, name, type), 
          m_sclass(sclass), m_body(body) {}

    FunctionDecl(const FunctionDecl&) = delete;
    FunctionDecl& operator = (const FunctionDecl&) = delete;

    FunctionDecl();

    /// Returns the number of parameters this function has.
    uint32_t num_params() const { 
        return static_cast<const FunctionType*>(
            m_type.get_type())->num_params(); 
    }

    /// Returns true if this function has any parameters.
    bool has_params() const { return num_params() != 0; }

    /// Returns the parameter in this function named by \p name if it exists,
    /// and 'nullptr' otherwise.
    const ParameterDecl* get_param(const string& name) const {
        return dynamic_cast<const ParameterDecl*>(DeclContext::get_decl(name));
    }

    ParameterDecl* get_param(const string& name) {
        return const_cast<ParameterDecl*>(
            static_cast<const FunctionDecl*>(this)->get_param(name));
    }

    /// Returns the type of the parameter at position \p i of this function's 
    /// parameter list.
    const QualType& get_param_type(uint32_t i) const {
        return static_cast<const FunctionType*>(
            m_type.get_type())->get_param_type(i);
    }

    /// Returns true if this function declaration has a body.
    bool has_body() const { return m_body != nullptr; }

    /// Returns the body statement of this function declaration, if it exists.
    const Stmt* get_body() const { return m_body; }
    Stmt* get_body() { return m_body; }

    void print(ostream& os) const override;
};

/// Represents a field in a 'struct' declaration.
class FieldDecl final : public ValueDecl {
public:
    FieldDecl(const Span& span, const string& name, const QualType& type)
        : ValueDecl(Kind::Field, span, name, type) {}

    FieldDecl(const FieldDecl&) = delete;
    FieldDecl& operator = (const FieldDecl&) = delete;

    void print(ostream& os) const override;
};

/// Base class for all declarations that define a new type.
class TypeDecl : public NamedDecl {
protected:
    /// The type defined by this declaration.
    const Type* m_type;

    explicit TypeDecl(Kind kind, const Span& span, const string& name, 
                      const Type* type)
        : NamedDecl(kind, span, name), m_type(type) {}

public:
    virtual ~TypeDecl() = default;

    /// Returns the type that this declaration defines.
    const Type* get_type() const { return m_type; }

    /// Set the type that this declaration defines to \p type.
    void set_type(const Type* type) { m_type = type; }
};

/// Represents a 'typedef' declaration.
class TypedefDecl final : public TypeDecl {
public:
    TypedefDecl(const Span& span, const string& name, const Type* type)
        : TypeDecl(Kind::Typedef, span, name, type) {}

    TypedefDecl(const TypedefDecl&) = delete;
    TypedefDecl& operator = (const TypedefDecl&) = delete;

    void print(ostream& os) const override;
};

/// Represents a 'struct' or 'union' declaration.
class RecordDecl final : public DeclContext, public TypeDecl {
    /// If true, then this record is a 'struct' declaration, and a 'union'
    /// declaration otherwise.
    bool m_struct;

public:
    RecordDecl(DeclContext* dctx, const Span& span, const string& name, 
               const Type* type, bool is_struct)
        : DeclContext(dctx), TypeDecl(Kind::Record, span, name, type), 
          m_struct(is_struct) {}

    RecordDecl(const RecordDecl&) = delete;
    RecordDecl& operator = (const RecordDecl&) = delete;

    /// Returns true if this is a 'struct' record declaration.
    bool is_struct() const { return m_struct; }

    /// Returns true if this is a 'union' record declaration.
    bool is_union() const { return !m_struct; }

    /// Returns the field declaration in this record with the name \p name if
    /// there is one, and 'nullptr' otherwise.
    const FieldDecl* get_field(const string& name) const {
        return dynamic_cast<const FieldDecl*>(DeclContext::get_decl(name));
    }

    FieldDecl* get_field(const string& name) {
        return const_cast<FieldDecl*>(
            static_cast<const RecordDecl*>(this)->get_field(name));
    }

    void print(ostream& os) const override;
};

/// Represents a variant of an enumeration.
class EnumVariantDecl final : public ValueDecl {
    /// The value of this variant.
    const int32_t m_value;

public:
    EnumVariantDecl(const Span& span, const string& name, const QualType& type, 
                    int32_t value)
        : ValueDecl(Kind::EnumVariant, span, name, type), m_value(value) {}

    EnumVariantDecl(const EnumVariantDecl&) = delete;
    EnumVariantDecl& operator = (const EnumVariantDecl&) = delete;

    /// Returns the value of this variant.
    int32_t get_value() const { return m_value; }

    void print(ostream& os) const override;
};

/// Represents an 'enum' declaration.
class EnumDecl final : public DeclContext, public TypeDecl {
public:
    EnumDecl(DeclContext* dctx, const Span& span, const string& name, 
             const Type* type)
        : DeclContext(dctx), TypeDecl(Kind::Enum, span, name, type) {}

    EnumDecl(const EnumDecl&) = delete;
    EnumDecl& operator = (const EnumDecl&) = delete;

    /// Returns the number of variants in this enum.
    uint32_t num_variants() const { return DeclContext::num_decls(); }

    /// Returns true if this enum does not have any variants.
    bool empty() const { return DeclContext::empty(); }

    /// Returns the variant in this enum with name \p name if there is one, 
    /// and 'nullptr' otherwise.
    const EnumVariantDecl* get_variant(const string& name) const {
        return dynamic_cast<const EnumVariantDecl*>(DeclContext::get_decl(name));
    }

    EnumVariantDecl* get_variant(const string& name) {
        return const_cast<EnumVariantDecl*>(
            static_cast<const EnumDecl*>(this)->get_variant(name));
    }

    void print(ostream& os) const override;
};

} // namespace scc

#endif // SCC_DECL_H_

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
#include "ast/Visitor.hpp"
#include "core/SourceSpan.hpp"

#include <cassert>
#include <vector>

namespace scc {

using std::ostream;
using std::string;
using std::vector;

class Expr;
class Stmt;

class DeclContext;
class NamedDecl;
class TagTypeDecl;

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

    /// The span of source code that this declaration covers.
    SourceSpan m_span;

    explicit Decl(DeclContext* dctx, Kind kind, const SourceSpan& span);

public:
    virtual ~Decl() = default;

    /// Returns the kind of declaration this is.
    Kind get_kind() const { return m_kind; }

    /// Returns the span of source code that this declaration covers.
    const SourceSpan& get_span() const { return m_span; }
    SourceSpan& get_span() { return m_span; }
    
    /// Returns the location in source code that this declaration starts at.
    const SourceLocation& get_starting_loc() const { return m_span.start; }
    SourceLocation& get_starting_loc() { return m_span.start; }

    /// Returns the location in source code that this declaration ends at.
    const SourceLocation& get_ending_loc() const { return m_span.end; }
    SourceLocation& get_ending_loc() { return m_span.end; }

    /// Accept some visitor class \p visitor to access this node.
    virtual void accept(Visitor& visitor) = 0;

    /// Pretty-print this declaration node to the output stream \p os.
    virtual void print(ostream& os) const = 0;
};

/// Base class for all declarations that may have a name.
class NamedDecl : public Decl {
protected:
    /// The name of this declaration.
    string m_name;

    explicit NamedDecl(DeclContext* dctx, Kind kind, const SourceSpan& span, 
                       const string& name)
        : Decl(dctx, kind, span), m_name(name) {}

public:
    virtual ~NamedDecl() = default;

    /// Returns true if this declaration has a name.
    bool has_name() const { return !m_name.empty(); }

    /// Returns the name of this declaration.
    const string& get_name() const { return m_name; }
    string& get_name() { return m_name; }
};

/// Base class for all named declarations that produce a value.
class ValueDecl : public NamedDecl {
protected:
    /// The type of this declaration.
    QualType m_type;

    explicit ValueDecl(DeclContext* dctx, Kind kind, const SourceSpan& span, 
                       const string& name, const QualType& type)
        : NamedDecl(dctx, kind, span, name), m_type(type) {}

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
    DeclContext* m_parent = nullptr;

    /// The declarations in this context.
    vector<Decl*> m_decls = {};

    /// The tag type declarations in this context.
    vector<TagTypeDecl*> m_tags = {};

    DeclContext() = default;

    DeclContext(DeclContext* parent) : m_parent(parent) {}

public:
    ~DeclContext();
    
    /// Returns true if this context has a parent context.
    bool has_parent() const { return m_parent != nullptr; }

    /// Returns the parent of this context, if there is one.
    const DeclContext* get_parent() const { return m_parent; }
    DeclContext* get_parent() { return m_parent; }

    /// Returns the number of declarations in this context.
    uint32_t num_decls() const { return m_decls.size(); }

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

    /// Returns the number of tag declarations in this context.
    uint32_t num_tags() const { return m_tags.size(); }

    /// Returns the tag type declarations in this context.
    const vector<TagTypeDecl*>& get_tags() const { return m_tags; }
    vector<TagTypeDecl*>& get_tags() { return m_tags; }

    const TagTypeDecl* get_tag(const string& name) const;
    TagTypeDecl* get_tag(const string& name) {
        return const_cast<TagTypeDecl*>(
            static_cast<const DeclContext*>(this)->get_tag(name));
    }

    /// Add a new declaration \p decl to this context.
    void add(Decl* decl);
};

/// Represents a translation unit declaration (a source file).
class TranslationUnitDecl final : public DeclContext, public Decl {
    friend class Sema;
    friend class Codegen;

    /// The context of types for this translation unit.
    TypeContext* m_tctx;

public:
    TranslationUnitDecl(const string& file);

    TranslationUnitDecl(const TranslationUnitDecl&) = delete;
    TranslationUnitDecl& operator = (const TranslationUnitDecl&) = delete;

    ~TranslationUnitDecl();

    /// Returns the type context of this translation unit.
    const TypeContext& get_context() const { return *m_tctx; }
    TypeContext& get_context() { return *m_tctx; }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Represents a variable declaration, either global or local.
class VariableDecl final : public ValueDecl {
    friend class Sema;
    friend class Codegen;

    /// The storage class of this variable.
    StorageClass m_storage;

    /// The initializing expression of this variable, if there is one.
    Expr* m_init;

public:
    VariableDecl(DeclContext* dctx, const SourceSpan& span, const string& name, 
                 const QualType& type, StorageClass storage, Expr* init)
        : ValueDecl(dctx, Kind::Variable, span, name, type), m_storage(storage),
          m_init(init) {}

    VariableDecl(const VariableDecl&) = delete;
    VariableDecl& operator = (const VariableDecl&) = delete;

    ~VariableDecl();

    /// Returns the storage class of this variable.
    StorageClass get_storage_class() const { return m_storage; }

    /// Returns true if this variable has the 'extern' storage class.
    bool is_extern() const { return m_storage == Extern; }

    /// Returns true if this variable has the 'static' storage class.
    bool is_static() const { return m_storage == Static; }

    /// Returns true if this variable has an initializing expression.
    bool has_init() const { return m_init != nullptr; }

    /// Returns the initializing expression of this variable, if there is one.
    const Expr* get_init() const { return m_init; }
    Expr* get_init() { return m_init; }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Represents a parameter declaration within a function parameter list.
class ParameterDecl final : public ValueDecl {
    friend class Sema;
    friend class Codegen;

public:
    ParameterDecl(DeclContext* dctx, const SourceSpan& span, 
                  const string& name, const QualType& type)
        : ValueDecl(dctx, Kind::Parameter, span, name, type) {}

    ParameterDecl(const ParameterDecl&) = delete;
    ParameterDecl& operator = (const ParameterDecl&) = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Represents a function declaration.
class FunctionDecl final : public DeclContext, public ValueDecl {
    friend class Sema;
    friend class Codegen;

    /// The storage class of this function.
    StorageClass m_storage;

    /// The parameters of this function. These are borrowed from the 
    /// DeclContext super class.
    vector<ParameterDecl*> m_params = {};

    /// The body of this function, if there is one.
    Stmt* m_body = nullptr;

public:
    FunctionDecl(DeclContext* dctx, const SourceSpan& span, const string& name, 
                 const QualType& type, StorageClass storage)
        : DeclContext(dctx), ValueDecl(dctx, Kind::Function, span, name, type), 
          m_storage(storage) {}

    FunctionDecl(const FunctionDecl&) = delete;
    FunctionDecl& operator = (const FunctionDecl&) = delete;

    ~FunctionDecl();

    /// Returns the storage class of this function.
    StorageClass get_storage_class() const { return m_storage; }

    /// Returns true if this function has the 'extern' storage class.
    bool is_extern() const { return m_storage == Extern; }

    /// Returns true if this function has the 'static' storage class.
    bool is_static() const { return m_storage == Static; }

    /// Returns the number of parameters this function has.
    uint32_t num_params() const { return m_params.size(); }

    /// Returns true if this function has any parameters.
    bool has_params() const { return !m_params.empty(); }

    /// Returns the parameters of this function.
    const vector<ParameterDecl*>& get_params() const { return m_params; }
    vector<ParameterDecl*>& get_params() { return m_params; }

    /// Set the parameter list of this function to \p params.
    void set_params(const vector<ParameterDecl*>& params) {
        m_params = params;
    }
    
    /// Returns the parameter at position \p i of this function.
    const ParameterDecl* get_param(uint32_t i) const {
        assert(i < num_params() && "index out of bounds!");
        return m_params[i];
    }

    ParameterDecl* get_param(uint32_t i) {
        assert(i < num_params() && "index out of bounds!");
        return m_params[i];
    }

    /// Returns the parameter in this function named by \p name if it exists,
    /// and 'nullptr' otherwise.
    const ParameterDecl* get_param(const string& name) const {
        for (const auto& param : m_params)
            if (param->get_name() == name)
                return param;

        return nullptr;
    }

    ParameterDecl* get_param(const string& name) {
        return const_cast<ParameterDecl*>(
            static_cast<const FunctionDecl*>(this)->get_param(name));
    }

    /// Returns the type of the parameter at position \p i of this function's 
    /// parameter list.
    const QualType& get_param_type(uint32_t i) const {
        assert(i < num_params() && "index out of bounds!");
        return m_params[i]->get_type();
    }

    QualType& get_param_type(uint32_t i) {
        assert(i < num_params() && "index out of bounds!");
        return m_params[i]->get_type();
    }

    /// Returns true if this function declaration has a body.
    bool has_body() const { return m_body != nullptr; }

    /// Returns the body statement of this function declaration, if it exists.
    const Stmt* get_body() const { return m_body; }
    Stmt* get_body() { return m_body; }

    /// Set the body of this function to \p body.
    void set_body(Stmt* body) { m_body = body; }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Represents a field in a 'struct' declaration.
class FieldDecl final : public ValueDecl {
    friend class Sema;
    friend class Codegen;

public:
    FieldDecl(DeclContext* dctx, const SourceSpan& span, const string& name, 
              const QualType& type)
        : ValueDecl(dctx, Kind::Field, span, name, type) {}

    FieldDecl(const FieldDecl&) = delete;
    FieldDecl& operator = (const FieldDecl&) = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Base class for all declarations that define a new type.
class TypeDecl : public NamedDecl {
protected:
    /// The type defined by this declaration.
    const Type* m_type;

    explicit TypeDecl(DeclContext* dctx, Kind kind, const SourceSpan& span, 
                      const string& name, const Type* type)
        : NamedDecl(dctx, kind, span, name), m_type(type) {}

public:
    virtual ~TypeDecl() = default;

    /// Returns the type that this declaration defines.
    const Type* get_type() const { return m_type; }

    /// Set the type that this declaration defines to \p type.
    void set_type(const Type* type) { m_type = type; }
};

/// Represents a 'typedef' declaration.
class TypedefDecl final : public TypeDecl {
    friend class Sema;
    friend class Codegen;

public:
    TypedefDecl(DeclContext* dctx, const SourceSpan& span, const string& name, 
                const Type* type)
        : TypeDecl(dctx, Kind::Typedef, span, name, type) {}

    TypedefDecl(const TypedefDecl&) = delete;
    TypedefDecl& operator = (const TypedefDecl&) = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Represents a tagged type 'struct', 'union', or 'enum' declaration.
class TagTypeDecl : public DeclContext, public TypeDecl {
public:
    /// Possible kinds of tag types.
    enum TagKind : uint32_t {
        Struct,
        Union,
        Enum,
    };

protected:
    /// The kind of tag type this is.
    TagKind m_tag_kind;

    TagTypeDecl(DeclContext* dctx, Kind kind, const SourceSpan& span, 
                const string& name, const Type* type, TagKind tag_kind)
        : DeclContext(dctx), TypeDecl(dctx, kind, span, name, type), 
          m_tag_kind(tag_kind) {}

public:
    virtual ~TagTypeDecl() = default;

    /// Returns the kind fo tag type this is.
    TagKind get_tag_kind() const { return m_tag_kind; }
};

/// Represents a 'struct' or 'union' declaration.
class RecordDecl final : public TagTypeDecl {
    friend class Sema;
    friend class Codegen;

    /// The fields of this record. These are borrowed from the DeclContext
    /// super class.
    vector<FieldDecl*> m_fields = {};

public:
    RecordDecl(DeclContext* dctx, const SourceSpan& span, const string& name, 
               const Type* type, TagKind tag_kind)
        : TagTypeDecl(dctx, Kind::Record, span, name, type, tag_kind) {}

    RecordDecl(const RecordDecl&) = delete;
    RecordDecl& operator = (const RecordDecl&) = delete;

    /// Returns true if this is a 'struct' record declaration.
    bool is_struct() const { return m_tag_kind == Struct; }

    /// Returns true if this is a 'union' record declaration.
    bool is_union() const { return m_tag_kind == Union; }

    /// Returns the number of fields in this record.
    uint32_t num_fields() const { return m_fields.size(); }

    /// Returns true if this record does not have any fields in it.
    bool empty() const { return m_fields.empty(); }

    /// Returns the fields in this record.
    const vector<FieldDecl*>& get_fields() const { return m_fields; }
    vector<FieldDecl*>& get_fields() { return m_fields; }

    /// Set the list of fields in this record to \p fields.
    void set_fields(const vector<FieldDecl*>& fields) { m_fields = fields; }

    /// Returns the field at position \p i of this record.
    const FieldDecl* get_field(uint32_t i) const {
        assert(i < num_fields() && "index out of bounds!");
        return m_fields[i];
    }

    FieldDecl* get_field(uint32_t i) {
        assert(i < num_fields() && "index out of bounds!");
        return m_fields[i];
    }

    /// Returns the field declaration in this record with the name \p name if
    /// there is one, and 'nullptr' otherwise.
    const FieldDecl* get_field(const string& name) const {
        for (const FieldDecl* field : m_fields)
            if (field->get_name() == name)
                return field;

        return nullptr;
    }

    FieldDecl* get_field(const string& name) {
        return const_cast<FieldDecl*>(
            static_cast<const RecordDecl*>(this)->get_field(name));
    }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Represents a variant of an enumeration.
class EnumVariantDecl final : public ValueDecl {
    friend class Sema;
    friend class Codegen;

    /// The value of this variant.
    const int32_t m_value;

public:
    EnumVariantDecl(DeclContext* dctx, const SourceSpan& span, 
                    const string& name, const QualType& type, int32_t value)
        : ValueDecl(dctx, Kind::EnumVariant, span, name, type), 
          m_value(value) {}

    EnumVariantDecl(const EnumVariantDecl&) = delete;
    EnumVariantDecl& operator = (const EnumVariantDecl&) = delete;

    /// Returns the value of this variant.
    int32_t get_value() const { return m_value; }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

/// Represents an 'enum' declaration.
class EnumDecl final : public TagTypeDecl {
    friend class Sema;
    friend class Codegen;
    
    /// The variants of this enum.
    vector<EnumVariantDecl*> m_variants = {};

public:
    EnumDecl(DeclContext* dctx, const SourceSpan& span, const string& name, 
             const Type* type)
        : TagTypeDecl(dctx, Kind::Enum, span, name, type, TagKind::Enum) {}

    EnumDecl(const EnumDecl&) = delete;
    EnumDecl& operator = (const EnumDecl&) = delete;

    /// Returns the number of variants in this enum.
    uint32_t num_variants() const { return DeclContext::num_decls(); }

    /// Returns true if this enum does not have any variants.
    bool empty() const { return m_variants.empty(); }

    /// Returns the variants of this 'enum' declaration.
    const vector<EnumVariantDecl*>& get_variants() const { return m_variants; }
    vector<EnumVariantDecl*>& get_variants() { return m_variants; }

    /// Set the variants list of this enum to \p variants.
    void set_variants(const vector<EnumVariantDecl*>& variants) {
        m_variants = variants;
    }

    /// Returns the varient at position \p i of this enum.
    const EnumVariantDecl* get_variant(uint32_t i) const {
        assert(i < num_variants() && "index out of bounds!");
        return m_variants[i];
    }

    EnumVariantDecl* get_variant(uint32_t i) {
        assert(i < num_variants() && "index out of bounds!");
        return m_variants[i];
    }

    /// Returns the variant in this enum with name \p name if there is one, 
    /// and 'nullptr' otherwise.
    const EnumVariantDecl* get_variant(const string& name) const {
        for (const EnumVariantDecl* variant : m_variants)
            if (variant->get_name() == name)
                return variant;

        return nullptr;
    }

    EnumVariantDecl* get_variant(const string& name) {
        return const_cast<EnumVariantDecl*>(
            static_cast<const EnumDecl*>(this)->get_variant(name));
    }

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    void print(ostream& os) const override;
};

} // namespace scc

#endif // SCC_DECL_H_

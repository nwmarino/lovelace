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
#include "core/Span.hpp"

#include <cassert>
#include <memory>
#include <vector>

namespace scc {

class Expr;
class Stmt;

/// Possible kinds of storage classes in C.
enum class StorageClass : uint32_t {
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
        Variable,
        Parameter,
        Function,
        Typedef,
        Field,
        Struct,
        Variant,
        Enum,
    };

protected:
    /// The kind of declaration of this is.
    const Kind m_kind;

    /// The storage class of this declaration, if any.
    const StorageClass m_storage;

    /// The span of source code this declaration covers.
    const Span m_span;

    /// The name of this declaration, if it has one.
    const std::string m_name;

    /// The type of this declaration.
    QualType m_type;

public:
    explicit Decl(Kind kind, StorageClass storage, const Span& span, 
                  const std::string& name, const QualType& ty) 
        : m_kind(kind), m_storage(storage), m_span(span), m_name(name), 
          m_type(ty) {}

    Decl(const Decl&) = delete;
    Decl& operator = (const Decl&) = delete;

    virtual ~Decl() = default;

    /// Returns the kind of declaration this is.
    Kind kind() const { return m_kind; }

    /// Returns the storage class of this declaration.
    StorageClass storage_class() const { return m_storage; }
    
    /// Returns the span of source code this declaration covers.
    const Span& span() const { return m_span; }

    /// Returns the name of this declaration.
    const std::string& name() const { return m_name; }

    /// Returns the type of this variable.
    const QualType& get_type() const { return m_type; }
    QualType& get_type() { return m_type; }

    /// Pretty-print this declaration node to the output stream \p os.
    virtual void print(std::ostream& os) const = 0;
};

/// Represents a variable declaration, either global or local.
class VariableDecl final : public Decl {
    /// The initializing expression of this variable, if there is one.
    std::unique_ptr<Expr> m_init = nullptr;

public:
    explicit VariableDecl(StorageClass storage, const Span& span, 
                          const std::string& name, const QualType& ty, 
                          std::unique_ptr<Expr> init);

    VariableDecl(const VariableDecl&) = delete;
    VariableDecl& operator = (const VariableDecl&) = delete;

    /// Returns the initializing expression of this variable, if there is one.
    const Expr* get_initializer() const { return m_init.get(); }
    Expr* get_initializer() { return m_init.get(); }

    /// Returns true if this variable has an initializing expression.
    bool has_initializer() const { return m_init != nullptr; }

    void print(std::ostream& os) const override;
};

/// Represents a parameter declaration within a function parameter list.
class ParameterDecl final : public Decl {
public:
    ParameterDecl(const Span& span, const std::string& name, const QualType& ty);

    ParameterDecl(const ParameterDecl&) = delete;
    ParameterDecl& operator = (const ParameterDecl&) = delete;

    void print(std::ostream& os) const override;
};

/// Represents a top-level function declaration.
class FunctionDecl final : public Decl {
public:
    using ParameterList = std::vector<std::unique_ptr<ParameterDecl>>;

    ParameterList m_params;
    std::unique_ptr<Scope> m_scope;
    std::unique_ptr<Stmt> m_body;

public:
    explicit FunctionDecl(StorageClass storage, const Span& span, 
                          const std::string& name, const QualType& ty,
                          ParameterList& params, std::unique_ptr<Scope> scope, 
                          std::unique_ptr<Stmt> body);

    FunctionDecl(const FunctionDecl&) = delete;
    FunctionDecl& operator = (const FunctionDecl&) = delete;

    /// Returns the number of parameters this function has.
    uint32_t num_params() const { return m_params.size(); }

    /// Returns true if this function has any parameters.
    bool has_params() const { return !m_params.empty(); }

    /// Returns the parameter in this function named by \p name if it exists,
    /// and 'nullptr' otherwise.
    const ParameterDecl* get_param(const std::string& name) const {
        for (const auto& field : m_params)
            if (field->name() == name) return field.get();

        return nullptr;
    }

    ParameterDecl* get_param(const std::string& name) {
        return const_cast<ParameterDecl*>(
            static_cast<const FunctionDecl*>(this)->get_param(name));
    }

    /// Returns the parameter at position \p i of this function's parameter 
    /// list.
    const ParameterDecl* get_param(uint32_t i) const {
        assert(i < m_params.size() && "index out of bounds!");
        return m_params[i].get();
    }

    ParameterDecl* get_param(uint32_t i) {
        return const_cast<ParameterDecl*>(
            static_cast<const FunctionDecl*>(this)->get_param(i));
    }

    /// Returns the scope of this function declaration.
    const Scope* get_scope() const { return m_scope.get(); }
    Scope* get_scope() { return m_scope.get(); }

    /// Returns the body statement of this function declaration, if it exists.
    const Stmt* get_body() const { return m_body.get(); }
    Stmt* get_body() { return m_body.get(); }

    /// Returns true if this function declaration has a body.
    bool has_body() const { return m_body != nullptr; }

    void print(std::ostream& os) const override;
};

/// Represents a 'typedef' declaration.
class TypedefDecl final : public Decl {
public:
    TypedefDecl(const Span& span, const std::string& name, 
                const QualType& ty);

    TypedefDecl(const TypedefDecl&) = delete;
    TypedefDecl& operator = (const TypedefDecl&) = delete;

    void print(std::ostream& os) const override;
};

/// Represents a field of a structure.
class FieldDecl final : public Decl {
public:
    FieldDecl(const Span& span, const std::string& name, const QualType& ty);

    FieldDecl(const FieldDecl&) = delete;
    FieldDecl& operator = (const FieldDecl&) = delete;

    void print(std::ostream& os) const override;
};

/// Represents a 'struct' declaration.
class StructDecl final : public Decl {
    /// The fields of this structure.
    std::vector<std::unique_ptr<FieldDecl>> m_fields;

public:
    StructDecl(const Span& span, const std::string& name, const QualType& ty, 
               std::vector<std::unique_ptr<FieldDecl>>& fields);

    StructDecl(const StructDecl&) = delete;
    StructDecl& operator = (const StructDecl&) = delete;

    /// Returns the number of fields in this structure.
    uint32_t num_fields() const { return m_fields.size(); }

    /// Returns true if this structure has any fields.
    bool has_fields() const { return !m_fields.empty(); }

    /// Returns true if this structure does not have any fields.
    bool empty() const { return m_fields.empty(); }

    /// Returns the field in this structure named by \p name if it exists,
    /// and 'nullptr' otherwise.
    const FieldDecl* get_field(const std::string& name) const {
        for (const auto& field : m_fields)
            if (field->name() == name) return field.get();

        return nullptr;
    }

    FieldDecl* get_field(const std::string& name) {
        return const_cast<FieldDecl*>(
            static_cast<const StructDecl*>(this)->get_field(name));
    }

    /// Returns the field at position \p i of this structure.
    const FieldDecl* get_field(uint32_t i) const {
        assert(i < num_fields() && "index out of bounds!");
        return m_fields[i].get();
    }

    FieldDecl* get_field(uint32_t i) {
        return const_cast<FieldDecl*>(
            static_cast<const StructDecl*>(this)->get_field(i));
    }

    void print(std::ostream& os) const override;
};

/// Represents a variant of an enumeration.
class VariantDecl final : public Decl {
    /// The value of this variant.
    const int32_t m_value;

public:
    VariantDecl(const Span& span, const std::string& name, const QualType& ty, 
                int32_t value);

    VariantDecl(const VariantDecl&) = delete;
    VariantDecl& operator = (const VariantDecl&) = delete;

    /// Returns the value of this variant.
    int32_t get_value() const { return m_value; }

    void print(std::ostream& os) const override;
};

/// Represents an 'enum' declaration.
class EnumDecl final : public Decl {
    /// The variants of this enumeration.
    std::vector<std::unique_ptr<VariantDecl>> m_variants;

public:
    EnumDecl(const Span& span, const std::string& name, const QualType& ty, 
             std::vector<std::unique_ptr<VariantDecl>>& variants);

    EnumDecl(const EnumDecl&) = delete;
    EnumDecl& operator = (const EnumDecl&) = delete;

    /// Returns the number of variants in this enum.
    uint32_t num_variants() const { return m_variants.size(); }

    /// Returns true if this enum has any variants.
    bool has_variants() const { return !m_variants.empty(); }

    /// Returns true if this enum does not have any variants.
    bool empty() const { return m_variants.empty(); }

    /// Returns the variant in this enum named by \p name if it exists, and
    /// 'nullptr' otherwise.
    const VariantDecl* get_variant(const std::string& name) const {
        for (const auto& variant : m_variants)
            if (variant->name() == name) return variant.get();

        return nullptr;
    }

    VariantDecl* get_variant(const std::string& name) {
        return const_cast<VariantDecl*>(
            static_cast<const EnumDecl*>(this)->get_variant(name));
    }

    /// Returns the variant at position \p i of this enum.
    const VariantDecl* get_variant(uint32_t i) const {
        assert(i < m_variants.size() && "index out of bounds!");
        return m_variants[i].get();
    }

    VariantDecl* get_variant(uint32_t i) {
        return const_cast<VariantDecl*>(
            static_cast<const EnumDecl*>(this)->get_variant(i));
    }

    void print(std::ostream& os) const override;
};

} // namespace scc

#endif // SCC_DECL_H_

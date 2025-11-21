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

#include "ast/Scope.hpp"
#include "core/Span.hpp"

#include <cassert>
#include <memory>
#include <vector>

namespace scc {

class Expr;
class Stmt;
class Type;
class FunctionType;

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
    };

protected:
    /// The kind of declaration of this is.
    const Kind m_kind;

    /// The storage class of this declaration, if any.
    const StorageClass m_storage = StorageClass::None;

    /// The span of source code this declaration covers.
    const Span m_span;

    /// The name of this declaration, if it has one.
    const std::string m_name;

public:
    explicit Decl(Kind kind, StorageClass storage, const Span& span, 
                  const std::string& name) 
        : m_kind(kind), m_storage(storage), m_span(span), m_name(name) {}

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
};

/// Represents a variable declaration, either global or local.
class VariableDecl final : public Decl {
    /// The type of this variable.
    const Type* m_type = nullptr;

    /// The initializing expression of this variable, if there is one.
    std::unique_ptr<Expr> m_init = nullptr;

public:
    explicit VariableDecl(StorageClass storage, const Span& span, 
                          const std::string& name, const Type* ty, 
                          std::unique_ptr<Expr> init)
        : Decl(Kind::Variable, storage, span, name), m_type(ty), 
          m_init(std::move(init)) {}

    VariableDecl(const VariableDecl&) = delete;
    VariableDecl& operator = (const VariableDecl&) = delete;

    /// Returns the type of this variable.
    const Type* get_type() const { return m_type; }

    /// Returns the initializing expression of this variable, if there is one.
    const Expr* get_initializer() const { return m_init.get(); }
    Expr* get_initializer() { return m_init.get(); }

    /// Returns true if this variable has an initializing expression.
    bool has_initializer() const { return m_init != nullptr; }
};

/// Represents a parameter declaration within a function parameter list.
class ParameterDecl final : public Decl {
    /// The type of this parameter.
    const Type* m_type;

public:
    ParameterDecl(const Span& span, const std::string& name, const Type* ty)
        : Decl(Kind::Parameter, StorageClass::None, span, name), m_type(ty) {}

    ParameterDecl(const ParameterDecl&) = delete;
    ParameterDecl& operator = (const ParameterDecl&) = delete;

    /// Returns the type of this variable.
    const Type* get_type() const { return m_type; }
};

/// Represents a top-level function declaration.
class FunctionDecl final : public Decl {
public:
    using ParameterList = std::vector<std::unique_ptr<ParameterDecl>>;

    const FunctionType* m_type;
    ParameterList m_params;
    std::unique_ptr<Scope> m_scope;
    std::unique_ptr<Stmt> m_body;

public:
    explicit FunctionDecl(StorageClass storage, const Span& span, 
                          const std::string& name, const FunctionType* ty,
                          ParameterList& params, 
                          std::unique_ptr<Scope> scope, 
                          std::unique_ptr<Stmt> body)
        : Decl(Kind::Function, storage, span, name), m_type(ty), 
          m_params(std::move(params)), m_scope(std::move(scope)), 
          m_body(std::move(body)) {}

    FunctionDecl(const FunctionDecl&) = delete;
    FunctionDecl& operator = (const FunctionDecl&) = delete;

    /// Returns the type of this function.
    const FunctionType* get_type() const { return m_type; }

    /// Returns the parameter in this function named by \p name if it exists,
    /// and 'nullptr' otherwise.
    const ParameterDecl* get_field(const std::string& name) const {
        for (const auto& field : m_params)
            if (field->name() == name) return field.get();

        return nullptr;
    }

    ParameterDecl* get_field(const std::string& name) {
        return const_cast<ParameterDecl*>(
            static_cast<const FunctionDecl*>(this)->get_field(name));
    }

    /// Returns the parameter at the given index of this function.
    const ParameterDecl* get_field(uint32_t index) const {
        assert(index < m_params.size() && "index out of bounds!");
        return m_params[index].get();
    }

    ParameterDecl* get_field(uint32_t index) {
        return const_cast<ParameterDecl*>(
            static_cast<const FunctionDecl*>(this)->get_field(index));
    }

    /// Returns the scope of this function.
    const Scope* get_scope() const { return m_scope.get(); }
    Scope* get_scope() { return m_scope.get(); }

    /// Returns the body statement of this function, if it exists.
    const Stmt* get_body() const { return m_body.get(); }
    Stmt* get_body() { return m_body.get(); }

    /// Returns true if this function has a body.
    bool has_body() const { return m_body != nullptr; }
};

/*

/// Represents a `typedef` type declaration.
class TypedefDecl final : public Decl {
    /// The type that this typedef defines. It is owned by the AST context.
    std::shared_ptr<Type> m_type;

public:
    TypedefDecl(const Span& span, const std::string& name, 
                std::shared_ptr<Type> type);

    TypedefDecl(const TypedefDecl&) = delete;
    TypedefDecl& operator = (const TypedefDecl&) = delete;

    ~TypedefDecl() override = default;

    /// Returns the type of this variable.
    const Type* get_type() const { return m_type.get(); }
    Type* get_type() { return m_type.get(); }
};

/// Represents a field of a structure.
class FieldDecl final : public Decl {
    std::shared_ptr<Type> m_type;

public:
    FieldDecl(const Span& span, const std::string& name, 
              std::shared_ptr<Type> type);

    FieldDecl(const FieldDecl&) = delete;
    FieldDecl& operator = (const FieldDecl&) = delete;

    ~FieldDecl() override = default;

    /// Returns the type of this variable.
    const Type* get_type() const { return m_type.get(); }
    Type* get_type() { return m_type.get(); }
};

/// Represents a `struct` type declaration.
class StructDecl final : public Decl {
    /// The type that this structure defines. It is owned by the AST context. 
    std::shared_ptr<Type> m_type;

    /// The fields in this structure.
    std::vector<std::unique_ptr<FieldDecl>> m_fields;

public:
    StructDecl(const Span& span, const std::string& name, 
               std::shared_ptr<Type> type, 
               const std::vector<std::unique_ptr<FieldDecl>>& fields);

    StructDecl(const StructDecl&) = delete;
    StructDecl& operator = (const StructDecl&) = delete;

    ~StructDecl() override = default;

    /// Returns the type of this variable.
    const Type* get_type() const { return m_type.get(); }
    Type* get_type() { return m_type.get(); }

    /// Returns the field in this structure with the given name if it exists,
    /// and `nullptr` otherwise.
    const FieldDecl* get_field(const std::string& name) const {
        for (const auto& field : m_fields)
            if (field->get_name() == name)
                return field.get();

        return nullptr;
    }

    FieldDecl* get_field(const std::string& name) {
        return const_cast<FieldDecl*>(
            static_cast<const StructDecl*>(this)->get_field(name));
    }

    /// Returns the field at the given index of this structure.
    const FieldDecl* get_field(uint32_t index) const {
        assert(index < m_fields.size() && "index out of bounds!");
        return m_fields[index].get();
    }

    FieldDecl* get_field(uint32_t index) {
        return const_cast<FieldDecl*>(
            static_cast<const StructDecl*>(this)->get_field(index));
    }
};

/// Represents a variant of an enumeration.
class VariantDecl final : public Decl {
    /// The type of this enum variant - this will be the same as the type of
    /// its parent enumeration.
    std::shared_ptr<Type> m_type;

    /// The value of this enum variant.
    const int32_t m_value;

public:
    VariantDecl(const Span& span, const std::string& name, 
                    std::shared_ptr<Type> type, int32_t value);

    VariantDecl(const VariantDecl&) = delete;
    VariantDecl& operator = (const VariantDecl&) = delete;

    ~VariantDecl() override = default;

    /// Returns the type of this variable.
    const Type* get_type() const { return m_type.get(); }
    Type* get_type() { return m_type.get(); }

    /// Returns the value of this variant.
    int32_t get_value() const { return m_value; }
};

/// Represents an `enum` type declaration.
class EnumDecl final : public Decl {
    /// The type that this enumeration defines. This is owned by the AST 
    /// context.
    std::shared_ptr<Type> m_type;

    /// The variants in this enumeration.
    std::vector<std::unique_ptr<VariantDecl>> m_variants;

public:
    EnumDecl(const Span& span, const std::string& name,
             std::shared_ptr<Type> type, 
             const std::vector<std::unique_ptr<VariantDecl>>& variants);

    EnumDecl(const EnumDecl&) = delete;
    EnumDecl& operator = (const EnumDecl&) = delete;

    ~EnumDecl() override = default;

    /// Returns the type of this variable.
    const Type* get_type() const { return m_type.get(); }
    Type* get_type() { return m_type.get(); }

    /// Returns the variant in this enum with the given name if it exists, and 
    /// `nullptr` otherwise.
    const VariantDecl* get_variant(const std::string& name) const {
        for (const auto& variant : m_variants)
            if (variant->get_name() == name)
                return variant.get();

        return nullptr;
    }

    VariantDecl* get_variant(const std::string& name) {
        return const_cast<VariantDecl*>(
            static_cast<const EnumDecl*>(this)->get_variant(name));
    }

    /// Returns the variant at the given index of this enum.
    const VariantDecl* get_variant(uint32_t index) const {
        assert(index < m_variants.size() && "index out of bounds!");
        return m_variants[index].get();
    }

    VariantDecl* get_variant(uint32_t index) {
        return const_cast<VariantDecl*>(
            static_cast<const EnumDecl*>(this)->get_variant(index));
    }
};

*/

} // namespace scc

#endif // SCC_DECL_H_

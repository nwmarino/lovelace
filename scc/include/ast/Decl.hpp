#ifndef SCC_DECL_H_
#define SCC_DECL_H_

#include "Scope.hpp"
#include "../core/Span.hpp"

#include <cassert>
#include <memory>
#include <string>
#include <vector>

namespace scc {

class Expr;
class Stmt;
class Type;

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
    enum Kind : uint32_t {
        Variable,
        Parameter,
        Function,
        Typedef,
        Struct,
        Enum,
        Union,
    };

protected:
    const Kind m_kind;
    const StorageClass m_storage;
    const Span m_span;
    std::string m_name;

public:
    Decl(Kind kind, StorageClass storage, const Span& span, 
         const std::string& name);

    Decl(const Decl&) = delete;
    Decl& operator = (const Decl&) = delete;

    virtual ~Decl() = 0;

    /// Returns the kind of declaration this is.
    Kind get_kind() const { return m_kind; }

    /// Returns the storage class of this declaration.
    StorageClass get_storage_class() const { return m_storage; }
    
    /// Returns the span of source code this declaration covers.
    const Span& get_span() const { return m_span; }

    /// Returns the name of this declaration.
    const std::string& get_name() const { return m_name; }
    std::string& get_name() { return m_name; }

    /// Rename this declaration to |name|.
    void set_name(const std::string& name) { m_name = name; }    
};

/// Represents a variable declaration, either global or local.
class VariableDecl final : public Decl {
    std::shared_ptr<Type> m_type;
    std::unique_ptr<Expr> m_init;

public:
    VariableDecl(StorageClass storage, const Span& span, 
                 const std::string& name, std::shared_ptr<Type> type, 
                 std::unique_ptr<Expr> init);

    VariableDecl(const VariableDecl&) = delete;
    VariableDecl& operator = (const VariableDecl&) = delete;

    ~VariableDecl() override = default;

    /// Returns the type of this variable.
    const Type* get_type() const { return m_type.get(); }
    Type* get_type() { return m_type.get(); }

    /// Returns the initializing expression of this variable, if there is one.
    const Expr* get_initializer() const { return m_init.get(); }
    Expr* get_initializer() { return m_init.get(); }

    /// Returns true if this variable has an initializing expression.
    bool has_initializer() const { return m_init != nullptr; }
};

/// Represents a parameter declaration within a function parameter list.
class ParameterDecl final : public Decl {
    std::shared_ptr<Type> m_type;

public:
    ParameterDecl(const Span& span, const std::string& name, 
                  std::shared_ptr<Type> type);

    ParameterDecl(const ParameterDecl&) = delete;
    ParameterDecl& operator = (const ParameterDecl&) = delete;
    
    ~ParameterDecl() override = default;

    /// Returns the type of this variable.
    const Type* get_type() const { return m_type.get(); }
    Type* get_type() { return m_type.get(); }
};

/// Represents a top-level function declaration.
class FunctionDecl final : public Decl {
    std::shared_ptr<Type> m_type;
    std::vector<std::unique_ptr<ParameterDecl>> m_params;
    std::unique_ptr<Scope> m_scope;
    std::unique_ptr<Stmt> m_body;

public:
    FunctionDecl(StorageClass storage, const Span& span, 
                 const std::string& name, std::shared_ptr<Type> type,
                 const std::vector<std::unique_ptr<ParameterDecl>>& params,
                 std::unique_ptr<Scope> scope, std::unique_ptr<Stmt> body);

    FunctionDecl(const FunctionDecl&) = delete;
    FunctionDecl& operator = (const FunctionDecl&) = delete;

    ~FunctionDecl() override = default;

    /// Returns the type of this variable.
    const Type* get_type() const { return m_type.get(); }
    Type* get_type() { return m_type.get(); }

    /// Returns the parameter in this function with the given name if it exists,
    /// and `nullptr` otherwise.
    const ParameterDecl* get_field(const std::string& name) const {
        for (const auto& field : m_params)
            if (field->get_name() == name)
                return field.get();

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

} // namespace scc

#endif // SCC_DECL_H_

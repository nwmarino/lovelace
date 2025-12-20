//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef STATIM_DECL_H_
#define STATIM_DECL_H_

#include "stmc/tree/Context.hpp"
#include "stmc/tree/Rune.hpp"
#include "stmc/tree/Type.hpp"
#include "stmc/tree/TypeUse.hpp"
#include "stmc/tree/Visitor.hpp"
#include "stmc/types/SourceSpan.hpp"

#include <cassert>
#include <string>
#include <vector>

namespace stm {

using std::string;
using std::vector;

class BlockStmt;
class Expr;
class Rune;
class Scope;
class Stmt;
class Type;

/// Base class for all declaration nodes in the abstract syntax tree (AST).
class Decl {
protected:
    // The span of source code which this declaration covers.
    const SourceSpan m_span;

    Decl(SourceSpan span) : m_span(span) {}

public:
    Decl(const Decl&) = delete;
    void operator=(const Decl&) = delete;

    Decl(Decl&&) noexcept = delete;
    void operator=(Decl&&) noexcept = delete;

    virtual ~Decl() = default;

    virtual void accept(Visitor& visitor) = 0;

    SourceSpan get_span() const { return m_span; }
};

/// Represents a top-level 'load' declaration.
class LoadDecl : public Decl {
    // The path to the file being specified by this load.
    const string m_path;

    LoadDecl(SourceSpan span, const string& path) : Decl(span), m_path(path) {}

public:
    static LoadDecl* create(Context& ctx, SourceSpan span, const string& path);

    ~LoadDecl() = default;

    LoadDecl(const LoadDecl&) = delete;
    void operator=(const LoadDecl&) = delete;

    LoadDecl(LoadDecl&&) noexcept = delete;
    void operator=(LoadDecl&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    const string& get_path() const { return m_path; }
};

/// Representats an input source file i.e. translation unit.
class TranslationUnitDecl final : public Decl {
    // The corresponding input file.
    string m_file;

    Context m_context = {};
    
    // All of the top-level declarations within this translation unit.
    //
    // These are all owned by the parent unit, but may be borrowed for scoping,
    // named references, etc.
    vector<Decl*> m_decls = {};

    // The global scope of this translation unit. This borrows all named
    // declarations of this unit.
    Scope* m_scope;

    TranslationUnitDecl(const string& file);

public:
    static TranslationUnitDecl* create(const string& file);

    ~TranslationUnitDecl() override;

    TranslationUnitDecl(const TranslationUnitDecl&) = delete;
    void operator=(const TranslationUnitDecl&) = delete;

    TranslationUnitDecl(TranslationUnitDecl&&) noexcept = delete;
    void operator=(TranslationUnitDecl&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    const string& get_file() const { return m_file; }

    const Context& get_context() const { return m_context; }
    Context& get_context() { return m_context; }

    uint32_t num_decls() const { return m_decls.size(); }
    bool has_decls() const { return !m_decls.empty(); }

    const vector<Decl*>& get_decls() const { return m_decls; }
    vector<Decl*>& get_decls() { return m_decls; }

    const Decl* get_decl(uint32_t i) const {
        assert(i < num_decls() && "index out of bounds!");
        return m_decls[i];
    }

    Decl* get_decl(uint32_t i) {
        assert(i < num_decls() && "index out of bounds!");
        return m_decls[i];
    }

    const Scope* get_scope() const { return m_scope; }
    Scope* get_scope() { return m_scope; }
};

/// Base class for all declarations with a name and possible rune set.
class NamedDecl : public Decl {
protected:
    string m_name;
    RuneVec m_runes;

    NamedDecl(SourceSpan span, const string& name, const RuneVec& runes)
      : Decl(span), m_name(name), m_runes(runes) {}

public:
    virtual ~NamedDecl() override;

    NamedDecl(const NamedDecl&) = delete;
    void operator=(const NamedDecl&) = delete;

    NamedDecl(NamedDecl&&) noexcept = delete;
    void operator=(NamedDecl&&) noexcept = delete;

    void set_name(const string& name) { m_name = name; }
    const string& get_name() const { return m_name; }
    string& get_name() { return m_name; }

    uint32_t num_runes() const { return m_runes.size(); }
    bool has_runes() const { return !m_runes.empty(); }

    const RuneVec& get_runes() const { return m_runes; }
    RuneVec& get_runes() { return m_runes; }
};

/// Base class for all named declarations that are typed and produce a value.
class ValueDecl : public NamedDecl {
protected:
    // The type of this declaration.
    TypeUse m_type;

    ValueDecl(SourceSpan span, const string& name, const RuneVec& runes, 
              const TypeUse& type)
      : NamedDecl(span, name, runes), m_type(type) {}

public:
    virtual ~ValueDecl() = default;

    ValueDecl(const ValueDecl&) = delete;
    void operator=(const ValueDecl&) = delete;

    ValueDecl(ValueDecl&&) noexcept = delete;
    void operator=(ValueDecl&&) noexcept = delete;

    const TypeUse& get_type() const { return m_type; }
    TypeUse& get_type() { return m_type; }
};

/// Represents a variable declaration, either local or global.
class VariableDecl final : public ValueDecl {
    friend class SemanticAnalysis;

    // The initializing expression of this variable, if there is one.
    Expr* m_init;

    VariableDecl(SourceSpan span, const string& name, const RuneVec& runes, 
                 const TypeUse& type, Expr* init)
      : ValueDecl(span, name, runes, type), m_init(init) {}

public:
    static VariableDecl* create(Context& ctx, SourceSpan span, 
                                const string& name, const RuneVec& runes, 
                                const TypeUse& type, Expr* init);

    ~VariableDecl() override;

    VariableDecl(const VariableDecl&) = delete;
    void operator=(const VariableDecl&) = delete;

    VariableDecl(VariableDecl&&) noexcept = delete;
    void operator=(VariableDecl&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    bool has_init() const { return m_init != nullptr; }

    const Expr* get_init() const { return m_init; }
    Expr* get_init() { return m_init; }
};

/// Represents a function parameter declaration.
class ParameterDecl final : public ValueDecl {
    ParameterDecl(SourceSpan span, const string& name, const TypeUse& type)
      : ValueDecl(span, name, {}, type) {}
      
public:
    static ParameterDecl* create(Context& ctx, SourceSpan span, 
                                 const string& name, const TypeUse& type);

    ~ParameterDecl() = default;

    ParameterDecl(const ParameterDecl&) = delete;
    void operator=(const ParameterDecl&) = delete;

    ParameterDecl(ParameterDecl&&) noexcept = delete;
    void operator=(ParameterDecl&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }
};

/// Represents a function declaration.
class FunctionDecl final : public ValueDecl {
public:
    using Params = vector<ParameterDecl*>;

private:
    // The scope of this function.
    //
    // This is different from the scope of its body. This scope contains 
    // symbols tied directly to the function i.e. named parameters.
    Scope* m_scope;

    // The list of parameter declarations of this function.
    Params m_params = {};

    // The body statement of this function, if it has one, and null otherwise.
    BlockStmt* m_body = nullptr;

    FunctionDecl(SourceSpan span, const string& name, const RuneVec& runes, 
                 const TypeUse& type, Scope* scope, const Params& params, 
                 BlockStmt* body)
      : ValueDecl(span, name, runes, type), m_scope(scope), m_params(params), 
        m_body(body) {}

public:
    static FunctionDecl* create(Context& ctx, SourceSpan span, 
                                const string& name, const RuneVec& runes, 
                                const TypeUse& type, Scope* scope, 
                                const Params& params, BlockStmt* body = nullptr);

    ~FunctionDecl() override;

    FunctionDecl(const FunctionDecl&) = delete;
    void operator=(const FunctionDecl&) = delete;

    FunctionDecl(FunctionDecl&&) noexcept = delete;
    void operator=(FunctionDecl&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    const TypeUse& get_return_type() const {
        return static_cast<const FunctionType*>(
            m_type.get_type())->get_return_type();
    }

    /// Test if this function returns 'void'.
    bool returns_void() const {
        return static_cast<const FunctionType*>(
            m_type.get_type())->get_return_type()->is_void();
    }

    const Scope* get_scope() const { return m_scope; }
    Scope* get_scope() { return m_scope; }

    uint32_t num_params() const { return m_params.size(); }
    bool has_params() const { return !m_params.empty(); }

    void setParams(const Params& params) { m_params = params; }
    const Params& get_params() const { return m_params; }
    Params& get_params() { return m_params; }

    const ParameterDecl* get_param(uint32_t i) const {
        assert(i < num_params() && "index out of bounds!");
        return m_params[i];
    }

    ParameterDecl* get_param(uint32_t i) {
        assert(i < num_params() && "index out of bounds!");
        return m_params[i];
    }

    bool has_body() const { return m_body != nullptr; }

    void set_body(BlockStmt* body) { m_body = body; }
    const BlockStmt* get_body() const { return m_body; }
    BlockStmt* get_body() { return m_body; }
};

/// Represents a field declaration within a structure.
class FieldDecl final : public ValueDecl {
    FieldDecl(SourceSpan span, const string& name, const RuneVec& runes, 
              const TypeUse& type)
      : ValueDecl(span, name, runes, type) {}

public:
    static FieldDecl* create(Context& ctx, SourceSpan span, const string& name,
                             const RuneVec& runes, const TypeUse& type);

    ~FieldDecl() = default;

    FieldDecl(const FieldDecl&) = delete;
    void operator=(const FieldDecl&) = delete;

    FieldDecl(FieldDecl&&) noexcept = delete;
    void operator=(FieldDecl&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }
};

/// Represents an enum variant declaration.
class VariantDecl final : public ValueDecl {
    const int64_t m_value;

    VariantDecl(SourceSpan span, const string& name, const RuneVec& runes, 
                const TypeUse& type, int64_t value)
      : ValueDecl(span, name, runes, type), m_value(value) {}

public:
    static VariantDecl* create(Context& ctx, SourceSpan span, 
                               const string& name, const RuneVec& runes, 
                               const TypeUse& type, int64_t value);

    ~VariantDecl() = default;

    VariantDecl(const VariantDecl&) = delete;
    void operator=(const VariantDecl&) = delete;

    VariantDecl(VariantDecl&&) noexcept = delete;
    void operator=(VariantDecl&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }
    
    int64_t get_value() const { return m_value; }
};

/// Base class for all named declarations that define a new type.
class TypeDecl : public NamedDecl {
protected:
    // The type defined by this declaration.
    const Type* m_type;

    TypeDecl(SourceSpan span, const string& name, const RuneVec& runes, 
             const Type* type)
      : NamedDecl(span, name, runes), m_type(type) {}

public:
    virtual ~TypeDecl() = default;

    TypeDecl(const TypeDecl&) = delete;
    void operator=(const TypeDecl&) = delete;

    TypeDecl(TypeDecl&&) noexcept = delete;
    void operator=(TypeDecl&&) noexcept = delete;

    void set_type(const Type* type) { m_type = type; }
    const Type* get_type() const { return m_type; }
};

/// Represents a type alias declaration.
class AliasDecl final : public TypeDecl {
    AliasDecl(SourceSpan span, const string& name, const RuneVec& runes, 
              const Type* type)
      : TypeDecl(span, name, runes, type) {}

public:
    static AliasDecl* create(Context& ctx, SourceSpan span, const string& name, 
                             const RuneVec& runes, const Type* type);

    ~AliasDecl() = default;

    AliasDecl(const AliasDecl&) = delete;
    void operator=(const AliasDecl&) = delete;

    AliasDecl(AliasDecl&&) noexcept = delete;
    void operator=(AliasDecl&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }
};

/// Represents a structure type declaration.
class StructDecl final : public TypeDecl {
public:
    using Fields = vector<FieldDecl*>;
    
private:
    Fields m_fields = {};

    StructDecl(SourceSpan span, const string& name, const RuneVec& runes, 
               const Type* type)
      : TypeDecl(span, name, runes, type) {}
      
public:
    static StructDecl* create(Context& ctx, SourceSpan span, const string& name, 
                              const RuneVec& runes, const Type* type);

    ~StructDecl() override;

    StructDecl(const StructDecl&) = delete;
    void operator=(const StructDecl&) = delete;

    StructDecl(StructDecl&&) noexcept = delete;
    void operator=(StructDecl&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }
   
    uint32_t num_fields() const { return m_fields.size(); }
    bool has_fields() const { return !m_fields.empty(); }

    void set_fields(const Fields& fields) { m_fields = fields; }
    const Fields& get_fields() const { return m_fields; }
    Fields& get_fields() { return m_fields; }

    const FieldDecl* get_field(const string& name) const {
        for (const auto& field : m_fields)
            if (field->get_name() == name)
                return field;

        return nullptr;
    }

    FieldDecl* get_field(const string& name) {
        return const_cast<FieldDecl*>(
            static_cast<const StructDecl*>(this)->get_field(name));
    }
};

/// Represents an enumeration type declaration.
class EnumDecl final : public TypeDecl {
public:
    using Variants = vector<VariantDecl*>;

private:
    Variants m_variants = {};

    EnumDecl(SourceSpan span, const string& name, const RuneVec& runes, 
             const Type* type)
      : TypeDecl(span, name, runes, type) {}

public:
    static EnumDecl* create(Context& ctx, SourceSpan span, const string& name,
                            const RuneVec& runes, const Type* type);

    ~EnumDecl() override;

    EnumDecl(const EnumDecl&) = delete;
    void operator=(const EnumDecl&) = delete;

    EnumDecl(EnumDecl&&) noexcept = delete;
    void operator=(EnumDecl&&) noexcept = delete;

    void accept(Visitor& visitor) override { visitor.visit(*this); }

    uint32_t num_variants() const { return m_variants.size(); }
    bool has_variants() const { return !m_variants.empty(); }

    void set_variants(const Variants& variants) { m_variants = variants; }
    const Variants& get_variants() const { return m_variants; }
    Variants& get_variants() { return m_variants; }
};

} // namespace stm

#endif // STATIM_DECL_H_

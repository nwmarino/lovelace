//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "ast/Decl.hpp"
#include "ast/Expr.hpp"
#include "ast/Stmt.hpp"

using namespace scc;

VariableDecl::VariableDecl(StorageClass storage, const Span& span, 
                           const std::string& name, const QualType& ty, 
                           std::unique_ptr<Expr> init)
    : Decl(Kind::Variable, storage, span, name, ty), m_init(std::move(init)) {}

ParameterDecl::ParameterDecl(const Span& span, const std::string& name, 
							 const QualType& ty)
	: Decl(Kind::Parameter, StorageClass::None, span, name, ty) {}

FunctionDecl::FunctionDecl(StorageClass storage, const Span& span, 
                           const std::string& name, const QualType& ty,
                           ParameterList& params, std::unique_ptr<Scope> scope, 
                           std::unique_ptr<Stmt> body)
    : Decl(Kind::Function, storage, span, name, ty), m_params(std::move(params)), 
      m_scope(std::move(scope)), m_body(std::move(body)) {}

TypedefDecl::TypedefDecl(const Span& span, const std::string& name, 
						 const QualType& ty)
	: Decl(Kind::Typedef, StorageClass::None, span, name, ty) {}

VariantDecl::VariantDecl(const Span& span, const std::string& name, 
						 const QualType& ty, int32_t value)
	: Decl(Kind::Variant, StorageClass::None, span, name, ty), m_value(value) {}

EnumDecl::EnumDecl(const Span& span, const std::string& name, 
                   const QualType& ty, 
                   std::vector<std::unique_ptr<VariantDecl>>& variants)
    : Decl(Kind::Enum, StorageClass::None, span, name, ty),
	  m_variants(std::move(variants)) {}

FieldDecl::FieldDecl(const Span& span, const std::string& name, 
                     const QualType& ty)
    : Decl(Kind::Field, StorageClass::None, span, name, ty) {}

StructDecl::StructDecl(const Span& span, const std::string& name, 
                       const QualType& ty, 
                       std::vector<std::unique_ptr<FieldDecl>>& fields)
    : Decl(Kind::Struct, StorageClass::None, span, name, ty), 
      m_fields(std::move(fields)) {}

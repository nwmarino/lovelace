//
//  Copyright (c) 2026 Nick Marino
//  All rights reserved.
//

#include "lace/codegen/LLVMCodegen.hpp"
#include "lace/core/Diagnostics.hpp"
#include "lace/tree/Type.hpp"

#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Verifier.h>

using namespace lace;

LLVMCodegen::LLVMCodegen(const Options& options, const AST* ast, 
                         llvm::Module* mod)
        : options(options), ast(ast), mod(mod), builder(mod->getContext()) {
    for (Defn* defn : ast->get_loaded())
		emit_initial_definition(defn);

	for (Defn* defn : ast->get_defns())
		emit_initial_definition(defn);

	for (Defn* defn : ast->get_loaded()) {
		if (StructDefn* structure = dynamic_cast<StructDefn*>(defn))
			fill_existing_structure(structure);
	}

	for (Defn* defn : ast->get_defns())
		fill_existing_definition(defn);
}

void LLVMCodegen::finalize() {
	if (llvm::verifyModule(*mod, &llvm::errs()))
		log::fatal("module verification failed", 
			log::Location(ast->get_file(), { 0, 0 }));
}

llvm::Type* LLVMCodegen::to_llvm_type(const QualType& type) {
	switch (type->get_class()) {
        case Type::Alias:
			return to_llvm_type(static_cast<const AliasType*>(
				type.get_type())->get_underlying());

        case Type::Array: {
			const ArrayType* array = 
				static_cast<const ArrayType*>(type.get_type());

			return llvm::ArrayType::get(
				to_llvm_type(array->get_element_type()), 
				array->get_size());
		}

        case Type::Builtin: {
			const BuiltinType* builtin = 
				static_cast<const BuiltinType*>(type.get_type());

			switch (builtin->get_kind()) {
				case BuiltinType::Void:
					return llvm::Type::getVoidTy(mod->getContext());
				case BuiltinType::Bool:
				case BuiltinType::Char:
				case BuiltinType::Int8:
				case BuiltinType::UInt8:
					return llvm::Type::getInt8Ty(mod->getContext());
				case BuiltinType::Int16:
				case BuiltinType::UInt16:
					return llvm::Type::getInt16Ty(mod->getContext());
				case BuiltinType::Int32:
				case BuiltinType::UInt32:
					return llvm::Type::getInt32Ty(mod->getContext());
				case BuiltinType::Int64:
				case BuiltinType::UInt64:
					return llvm::Type::getInt64Ty(mod->getContext());
				case BuiltinType::Float32:
					return llvm::Type::getFloatTy(mod->getContext());
				case BuiltinType::Float64:
					return llvm::Type::getDoubleTy(mod->getContext());
			}
		}

        case Type::Deferred:
			__builtin_unreachable(); // No deferred types should reach this.
        
		case Type::Enum:
			return to_llvm_type(
					static_cast<const EnumType*>(type.get_type())->get_underlying());
        
		case Type::Function: {
			const FunctionType* sig = 
				static_cast<const FunctionType*>(type.get_type());

			std::vector<llvm::Type*> args(sig->num_params(), nullptr);
			for (uint32_t i = 0; i < sig->num_params(); ++i)
				args[i] = to_llvm_type(sig->get_param(i));

			return llvm::FunctionType::get(
				to_llvm_type(sig->get_return_type()), args, false);
		}

        case Type::Pointer:
			return llvm::PointerType::getUnqual(mod->getContext());

        case Type::Struct:
			return llvm::StructType::getTypeByName(
				mod->getContext(), type.get_type()->to_string());
	}

	__builtin_unreachable();
}

llvm::Value* LLVMCodegen::inject_comparison(llvm::Value* value) {
	llvm::Type* type = value->getType();

	if (type->isIntegerTy(1)) {
		return value;
	} else if (type->isIntegerTy()) {
		return builder.CreateICmpNE(value, llvm::ConstantInt::get(type, 0));
	} else if (type->isFloatingPointTy()) {
		return builder.CreateFCmpONE(value, llvm::ConstantFP::getZero(type));
	} else if (type->isPointerTy()) {
		return builder.CreateICmpNE(
			value, 
			llvm::ConstantPointerNull::get(dyn_cast<llvm::PointerType>(type)));
	}

	assert(false && "value cannot be reduced to a boolean!");
}

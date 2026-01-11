//
//  Copyright (c) 2026 Nick Marino
//  All rights reserved.
//

#include "lace/codegen/LLVMCodegen.hpp"
#include "lace/core/Diagnostics.hpp"
#include "lace/tree/Visitor.hpp"
#include "lace/core/Diagnostics.hpp"

#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/raw_ostream.h>

using namespace lace;

void LLVMCodegen::emit_initial_definition(const Defn* defn) {
    switch (defn->get_kind()) {
        case Defn::Function:
            emit_initial_function(static_cast<const FunctionDefn*>(defn));
            break;
        case Defn::Struct:
            emit_structure_shell(static_cast<const StructDefn*>(defn));
            break;
        case Defn::Variable:
            emit_initial_global(static_cast<const VariableDefn*>(defn));
            break;
        default:
            break;
    }
}

void LLVMCodegen::fill_existing_definition(const Defn* defn) {
    switch (defn->get_kind()) {
        case Defn::Function:
            fill_existing_function(static_cast<const FunctionDefn*>(defn));
            break;
        case Defn::Struct:
            fill_existing_structure(static_cast<const StructDefn*>(defn));
            break;
        case Defn::Variable:
            fill_existing_global(static_cast<const VariableDefn*>(defn));
            break;
        default:
            break;
    }
}

llvm::Function* LLVMCodegen::emit_initial_function(const FunctionDefn* defn) {
    auto linkage = llvm::Function::LinkageTypes::InternalLinkage;
    if (defn->has_rune(Rune::Public))
        linkage = llvm::Function::LinkageTypes::ExternalLinkage;

    llvm::FunctionType* type = llvm::dyn_cast<llvm::FunctionType>(
        to_llvm_type(defn->get_type()));

   llvm::Function* fn = llvm::Function::Create(type, linkage, defn->get_name(), mod);

   fn->addFnAttr(llvm::Attribute::UWTable);
   fn->addFnAttr(llvm::Attribute::NoUnwind);
   fn->setUWTableKind(llvm::UWTableKind::Default);

   fn->addFnAttr("frame-pointer", "all");
   fn->addFnAttr("target-cpu", "x86-64");
   return fn;
}

llvm::Function* LLVMCodegen::fill_existing_function(const FunctionDefn* defn) {
    llvm::Function* fn = mod->getFunction(defn->get_name());
    if (!defn->has_body())
        return fn;

    func = fn;

    llvm::BasicBlock* entry = llvm::BasicBlock::Create(
        mod->getContext(), "entry", func);
    builder.SetInsertPoint(entry);

    assert(defn->num_params() == func->arg_size());
    for (uint32_t i = 0; i < func->arg_size(); ++i) {
        const ParameterDefn* param = defn->get_param(i);
        assert(param);

        std::string name = param->get_name();
        if (name == "_")
            continue;
        
        llvm::Argument* arg = func->getArg(i);
        arg->setName(name);
        
        llvm::IRBuilder<> tmp(entry);
        llvm::AllocaInst* alloca = tmp.CreateAlloca(arg->getType());
        allocas.emplace(name, alloca);

        builder.CreateStore(arg, alloca);
    }

    emit_statement(defn->get_body());

    if (!builder.GetInsertBlock()->getTerminator()) {
        if (defn->get_return_type()->is_void()) {
            builder.CreateRetVoid();
        } else {
            log::fatal("function does not always return", 
                log::Span(ast->get_file(), defn->get_span().end));
        }
    }

    if (llvm::verifyFunction(*func, &llvm::errs()))
        log::fatal("function verification failed", 
            log::Location(ast->get_file(), defn->get_span().start));

    func = nullptr;
    builder.ClearInsertionPoint();
    allocas.clear();
    return fn;
}

llvm::StructType* LLVMCodegen::emit_structure_shell(const StructDefn* defn) {
    return llvm::StructType::create(mod->getContext(), defn->get_name());
}

llvm::StructType* LLVMCodegen::fill_existing_structure(const StructDefn* defn) {
    llvm::StructType* type = llvm::StructType::getTypeByName(
        mod->getContext(), defn->get_name());
    assert(type && "structure type does not exist!");

    std::vector<llvm::Type*> body(defn->num_fields(), nullptr);
    for (uint32_t i = 0; i < defn->num_fields(); ++i)
        body[i] = to_llvm_type(defn->get_field(i)->get_type());

    type->setBody(body);
    return type;
}

llvm::GlobalVariable* LLVMCodegen::emit_initial_global(const VariableDefn* defn) {
    auto linkage = llvm::GlobalVariable::LinkageTypes::InternalLinkage;
    if (defn->has_rune(Rune::Public))
        linkage = llvm::GlobalVariable::LinkageTypes::ExternalLinkage;

    llvm::GlobalVariable* global = new llvm::GlobalVariable(
        to_llvm_type(defn->get_type()), // type
        !defn->get_type().is_mut(), // constant / read-only 
        linkage,
        nullptr, // initalizer (waiting until fill-in)
        defn->get_name() // name
    );

    mod->insertGlobalVariable(global);
    return global;
}

llvm::GlobalVariable* LLVMCodegen::fill_existing_global(const VariableDefn* defn) {
    llvm::GlobalVariable* global = mod->getGlobalVariable(defn->get_name(), true);
    assert(global && "global variable does not exist!");

    if (!defn->has_init())
        return global;

    llvm::Value* init = emit_valued_expression(defn->get_init());
    llvm::Constant* constant = llvm::dyn_cast<llvm::Constant>(init);
    assert(constant && "global initializer must be constant!");

    global->setInitializer(constant);
    return global;
}

llvm::AllocaInst* LLVMCodegen::emit_local(const VariableDefn* defn) {
    llvm::BasicBlock& entry = func->getEntryBlock();
    llvm::IRBuilder<> tmp(&entry, entry.begin());
    
    llvm::AllocaInst* alloca = tmp.CreateAlloca(
        to_llvm_type(defn->get_type()), nullptr, defn->get_name());

    allocas.emplace(defn->get_name(), alloca);
    return alloca;
}

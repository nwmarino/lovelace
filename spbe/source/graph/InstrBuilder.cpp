//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "spbe/graph/CFG.hpp"
#include "spbe/graph/InstrBuilder.hpp"
#include "spbe/graph/Constant.hpp"
#include "spbe/graph/Instruction.hpp"
#include "spbe/target/Target.hpp"

using namespace spbe;

void InstrBuilder::insert(Instruction* inst) {
    if (m_insert != nullptr) {
        switch (m_mode) {
        case InsertMode::Prepend:
            m_insert->push_front(inst);
            break;

        case InsertMode::Append:
            m_insert->push_back(inst);
            break;
        }
    }
}

Instruction* InstrBuilder::insert(Opcode op, uint32_t result, const Type* type, 
                                 const std::vector<Value*>& operands) {
    Instruction* inst = new Instruction(result, type, op, nullptr, operands);
    insert(inst);
    return inst;
}

Instruction* InstrBuilder::build_nop() {
    return insert(INST_OP_NOP);
}

Instruction* InstrBuilder::build_const(Constant* constant) {
    assert(constant && "constant cannot be null");

    return insert(
        INST_OP_CONSTANT, 
        m_cfg.get_def_id(), 
        constant->get_type(), 
        { constant });
}

Instruction* InstrBuilder::build_string(ConstantString* string) {
    assert(string && "string constant cannot be null");

    return insert(
        INST_OP_STRING,
        m_cfg.get_def_id(),
        string->get_type(),
        { string });
}

Instruction* InstrBuilder::build_load(const Type* type, Value* src) {
    assert(type && "type cannot be null");

    return build_aligned_load(
        type, src, m_cfg.get_target().get_type_align(type));
}

Instruction* InstrBuilder::build_aligned_load(const Type* type, Value* src, 
                                              uint16_t align) {
    assert(type && "type cannot be null");
    assert(src && "src cannot be null");
    assert(src->get_type()->is_pointer_type() &&
        "src type must be a pointer");
    
    Instruction* inst = insert(INST_OP_LOAD, m_cfg.get_def_id(), type, { src });
    inst->data() = align;
    return inst;
}

Instruction* InstrBuilder::build_store(Value* value, Value* dst) {
    assert(value && "value cannot be null");

    return build_aligned_store(
        value, dst, m_cfg.get_target().get_type_align(value->get_type()));
}

Instruction* InstrBuilder::build_aligned_store(Value* value, Value* dst, 
                                               uint16_t align) {
    assert(value && "value cannot be null");
    assert(dst && "dst cannot be null");
    assert(dst->get_type()->is_pointer_type() &&
        "dst type must be a pointer");
    
    Instruction* inst = insert(INST_OP_STORE, 0, nullptr, { value, dst });
    inst->data() = align;
    return inst;
}

Instruction* InstrBuilder::build_ap(const Type* type, Value* src, Value* idx) {
    assert(type && "type cannot be null");
    assert(src && "src cannot be null");
    assert(idx && "idx cannot be null");
    assert(type->is_pointer_type() &&
        "type must be a pointer");
    assert(src->get_type()->is_pointer_type() &&
        "src type must be a pointer");
    assert(idx->get_type()->is_integer_type() &&
        "idx type must be an integer");
    
    return insert(INST_OP_ACCESS_PTR, m_cfg.get_def_id(), type, { src, idx });
}

Instruction* InstrBuilder::build_select(Value* cond, Value* tvalue, 
                                       Value* fvalue) {
    assert(cond && "cond cannot be null");
    assert(tvalue && "tvalue cannot be null");
    assert(fvalue && "fvalue cannot be null");
    assert(cond->get_type()->is_integer_type(1) && "cond type must be i1");
    assert(*tvalue->get_type() == *fvalue->get_type() &&
        "tvalue and fvalue must have the same type");
    
    return insert(
        INST_OP_SELECT, m_cfg.get_def_id(), tvalue->get_type(), { cond, tvalue, fvalue });
}

Instruction* InstrBuilder::build_brif(Value* cond, BasicBlock* tdst, 
                                     BasicBlock* fdst) {
    assert(cond && "cond cannot be null");
    assert(tdst && "tdst cannot be null");
    assert(fdst && "fdst cannot be null");
    assert(cond->get_type()->is_integer_type(1) && "cond type must be i1");
    
    if (m_insert) {
        // Add tdst, fdst as successors to the current block.
        m_insert->succs().push_back(tdst);
        m_insert->succs().push_back(fdst);

        // Add the current block as a predecessor to tdst, fdst.
        tdst->preds().push_back(m_insert);
        fdst->preds().push_back(m_insert);
    }

    return insert(
        INST_OP_BRANCH_IF,
        0,
        nullptr,
        {
            cond,
            BlockAddress::get(m_cfg, tdst),
            BlockAddress::get(m_cfg, fdst),
        });
}

Instruction* InstrBuilder::build_jmp(BasicBlock* dst) {
    assert(dst && "dst cannot be null");

    if (m_insert) {
        // Add dst as a successor to the current block.
        m_insert->succs().push_back(dst);

        // Add the current block as a predecessor to dst.
        dst->preds().push_back(m_insert);
    }

    return insert(INST_OP_JUMP, 0, nullptr, { BlockAddress::get(m_cfg, dst) });
}

Instruction* InstrBuilder::build_phi(const Type* type) {
    return insert(INST_OP_PHI, m_cfg.get_def_id(), type);
}

Instruction* InstrBuilder::build_ret(Value* value) {
    return insert(INST_OP_RETURN, 0, nullptr, { value });
}

Instruction* InstrBuilder::build_abort() {
    return insert(INST_OP_ABORT);
}

Instruction* InstrBuilder::build_unreachable() {
    return insert(INST_OP_UNREACHABLE);
}

Instruction* InstrBuilder::build_call(const FunctionType* type, Value* callee, 
                                     const std::vector<Value*>& args) {
    assert(type && "type cannot be null");
    assert(callee && "callee cannot be null");

    std::vector<Value*> operands = { callee };
    for (auto arg : args)
        operands.push_back(arg);

    return insert(
        INST_OP_CALL,
        type->has_return_type() ? m_cfg.get_def_id() : 0,
        type->has_return_type() ? type->get_return_type() : nullptr,
        operands);
}

Instruction* InstrBuilder::build_cmp_ieq(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert((lhs->get_type()->is_integer_type() || 
        lhs->get_type()->is_pointer_type()) && 
        "lhs type must be an integer or a pointer");
    assert((rhs->get_type()->is_integer_type() ||
        rhs->get_type()->is_pointer_type()) && 
        "rhs type must be an integer or a pointer");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(
        INST_OP_CMP_IEQ, m_cfg.get_def_id(), Type::get_i1_type(m_cfg), { lhs, rhs });
}

Instruction* InstrBuilder::build_cmp_ine(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert((lhs->get_type()->is_integer_type() || 
        lhs->get_type()->is_pointer_type()) && 
        "lhs type must be an integer or a pointer");
    assert((rhs->get_type()->is_integer_type() ||
        rhs->get_type()->is_pointer_type()) && 
        "rhs type must be an integer or a pointer");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(
        INST_OP_CMP_INE, m_cfg.get_def_id(), Type::get_i1_type(m_cfg), { lhs, rhs });
}

Instruction* InstrBuilder::build_cmp_oeq(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert(lhs->get_type()->is_floating_point_type() && 
        "lhs type must be a floating point type");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(
        INST_OP_CMP_OEQ, m_cfg.get_def_id(), Type::get_i1_type(m_cfg), { lhs, rhs });
}

Instruction* InstrBuilder::build_cmp_one(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert(lhs->get_type()->is_floating_point_type() && 
        "lhs type must be a floating point type");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(
        INST_OP_CMP_ONE, m_cfg.get_def_id(), Type::get_i1_type(m_cfg), { lhs, rhs });
}

Instruction* InstrBuilder::build_cmp_uneq(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert(lhs->get_type()->is_floating_point_type() && 
        "lhs type must be a floating point type");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(
        INST_OP_CMP_UNEQ, m_cfg.get_def_id(), Type::get_i1_type(m_cfg), { lhs, rhs });
}

Instruction* InstrBuilder::build_cmp_unne(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert(lhs->get_type()->is_floating_point_type() && 
        "lhs type must be a floating point type");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(
        INST_OP_CMP_UNNE, m_cfg.get_def_id(), Type::get_i1_type(m_cfg), { lhs, rhs });
}

Instruction* InstrBuilder::build_cmp_slt(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert((lhs->get_type()->is_integer_type() || 
        lhs->get_type()->is_pointer_type()) && 
        "lhs type must be an integer or a pointer");
    assert((rhs->get_type()->is_integer_type() ||
        rhs->get_type()->is_pointer_type()) && 
        "rhs type must be an integer or a pointer");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(
        INST_OP_CMP_SLT, m_cfg.get_def_id(), Type::get_i1_type(m_cfg), { lhs, rhs });
}

Instruction* InstrBuilder::build_cmp_sle(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert((lhs->get_type()->is_integer_type() || 
        lhs->get_type()->is_pointer_type()) && 
        "lhs type must be an integer or a pointer");
    assert((rhs->get_type()->is_integer_type() ||
        rhs->get_type()->is_pointer_type()) && 
        "rhs type must be an integer or a pointer");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(
        INST_OP_CMP_SLE, m_cfg.get_def_id(), Type::get_i1_type(m_cfg), { lhs, rhs });
}

Instruction* InstrBuilder::build_cmp_sgt(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert((lhs->get_type()->is_integer_type() || 
        lhs->get_type()->is_pointer_type()) && 
        "lhs type must be an integer or a pointer");
    assert((rhs->get_type()->is_integer_type() ||
        rhs->get_type()->is_pointer_type()) && 
        "rhs type must be an integer or a pointer");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(
        INST_OP_CMP_SGT, m_cfg.get_def_id(), Type::get_i1_type(m_cfg), { lhs, rhs });
}

Instruction* InstrBuilder::build_cmp_sge(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert((lhs->get_type()->is_integer_type() || 
        lhs->get_type()->is_pointer_type()) && 
        "lhs type must be an integer or a pointer");
    assert((rhs->get_type()->is_integer_type() ||
        rhs->get_type()->is_pointer_type()) && 
        "rhs type must be an integer or a pointer");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(
        INST_OP_CMP_SGE, m_cfg.get_def_id(), Type::get_i1_type(m_cfg), { lhs, rhs });
}

Instruction* InstrBuilder::build_cmp_ult(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert((lhs->get_type()->is_integer_type() || 
        lhs->get_type()->is_pointer_type()) && 
        "lhs type must be an integer or a pointer");
    assert((rhs->get_type()->is_integer_type() ||
        rhs->get_type()->is_pointer_type()) && 
        "rhs type must be an integer or a pointer");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(
        INST_OP_CMP_ULT, m_cfg.get_def_id(), Type::get_i1_type(m_cfg), { lhs, rhs });
}

Instruction* InstrBuilder::build_cmp_ule(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert((lhs->get_type()->is_integer_type() || 
        lhs->get_type()->is_pointer_type()) && 
        "lhs type must be an integer or a pointer");
    assert((rhs->get_type()->is_integer_type() ||
        rhs->get_type()->is_pointer_type()) && 
        "rhs type must be an integer or a pointer");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(
        INST_OP_CMP_ULE, m_cfg.get_def_id(), Type::get_i1_type(m_cfg), { lhs, rhs });
}

Instruction* InstrBuilder::build_cmp_ugt(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert((lhs->get_type()->is_integer_type() || 
        lhs->get_type()->is_pointer_type()) && 
        "lhs type must be an integer or a pointer");
    assert((rhs->get_type()->is_integer_type() ||
        rhs->get_type()->is_pointer_type()) && 
        "rhs type must be an integer or a pointer");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(
        INST_OP_CMP_UGT, m_cfg.get_def_id(), Type::get_i1_type(m_cfg), { lhs, rhs });
}

Instruction* InstrBuilder::build_cmp_uge(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert((lhs->get_type()->is_integer_type() || 
        lhs->get_type()->is_pointer_type()) && 
        "lhs type must be an integer or a pointer");
    assert((rhs->get_type()->is_integer_type() ||
        rhs->get_type()->is_pointer_type()) && 
        "rhs type must be an integer or a pointer");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(
        INST_OP_CMP_UGT, m_cfg.get_def_id(), Type::get_i1_type(m_cfg), { lhs, rhs });
}

Instruction* InstrBuilder::build_cmp_olt(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert(lhs->get_type()->is_floating_point_type() && 
        "lhs type must be a floating point type");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(
        INST_OP_CMP_OLT, m_cfg.get_def_id(), Type::get_i1_type(m_cfg), { lhs, rhs });
}

Instruction* InstrBuilder::build_cmp_ole(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert(lhs->get_type()->is_floating_point_type() && 
        "lhs type must be a floating point type");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(
        INST_OP_CMP_OLE, m_cfg.get_def_id(), Type::get_i1_type(m_cfg), { lhs, rhs });
}

Instruction* InstrBuilder::build_cmp_ogt(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert(lhs->get_type()->is_floating_point_type() && 
        "lhs type must be a floating point type");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(
        INST_OP_CMP_OGT, m_cfg.get_def_id(), Type::get_i1_type(m_cfg), { lhs, rhs });
}

Instruction* InstrBuilder::build_cmp_oge(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert(lhs->get_type()->is_floating_point_type() && 
        "lhs type must be a floating point type");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(
        INST_OP_CMP_OGE, m_cfg.get_def_id(), Type::get_i1_type(m_cfg), { lhs, rhs });
}

Instruction* InstrBuilder::build_cmp_unlt(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert(lhs->get_type()->is_floating_point_type() && 
        "lhs type must be a floating point type");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(
        INST_OP_CMP_UNLT, m_cfg.get_def_id(), Type::get_i1_type(m_cfg), { lhs, rhs });
}

Instruction* InstrBuilder::build_cmp_unle(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert(lhs->get_type()->is_floating_point_type() && 
        "lhs type must be a floating point type");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(
        INST_OP_CMP_UNLE, m_cfg.get_def_id(), Type::get_i1_type(m_cfg), { lhs, rhs });
}

Instruction* InstrBuilder::build_cmp_ungt(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert(lhs->get_type()->is_floating_point_type() && 
        "lhs type must be a floating point type");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(
        INST_OP_CMP_UNGT, m_cfg.get_def_id(), Type::get_i1_type(m_cfg), { lhs, rhs });
}

Instruction* InstrBuilder::build_cmp_unge(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert(lhs->get_type()->is_floating_point_type() && 
        "lhs type must be a floating point type");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(
        INST_OP_CMP_UNGE, m_cfg.get_def_id(), Type::get_i1_type(m_cfg), { lhs, rhs });
}

Instruction* InstrBuilder::build_iadd(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert((lhs->get_type()->is_integer_type() || 
        lhs->get_type()->is_pointer_type()) && 
        "lhs type must be an integer or a pointer");
    assert((rhs->get_type()->is_integer_type() ||
        rhs->get_type()->is_pointer_type()) && 
        "rhs type must be an integer or a pointer");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(INST_OP_IADD, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* InstrBuilder::build_fadd(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert(lhs->get_type()->is_floating_point_type() && 
        "lhs type must be a floating point type");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(INST_OP_FADD, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* InstrBuilder::build_isub(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert((lhs->get_type()->is_integer_type() || 
        lhs->get_type()->is_pointer_type()) && 
        "lhs type must be an integer or a pointer");
    assert((rhs->get_type()->is_integer_type() ||
        rhs->get_type()->is_pointer_type()) && 
        "rhs type must be an integer or a pointer");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(INST_OP_ISUB, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* InstrBuilder::build_fsub(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert(lhs->get_type()->is_floating_point_type() && 
        "lhs type must be a floating point type");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(INST_OP_FSUB, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* InstrBuilder::build_smul(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert((lhs->get_type()->is_integer_type() || 
        lhs->get_type()->is_pointer_type()) && 
        "lhs type must be an integer or a pointer");
    assert((rhs->get_type()->is_integer_type() ||
        rhs->get_type()->is_pointer_type()) && 
        "rhs type must be an integer or a pointer");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(INST_OP_SMUL, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* InstrBuilder::build_umul(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert((lhs->get_type()->is_integer_type() || 
        lhs->get_type()->is_pointer_type()) && 
        "lhs type must be an integer or a pointer");
    assert((rhs->get_type()->is_integer_type() ||
        rhs->get_type()->is_pointer_type()) && 
        "rhs type must be an integer or a pointer");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(INST_OP_UMUL, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* InstrBuilder::build_fmul(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert(lhs->get_type()->is_floating_point_type() && 
        "lhs type must be a floating point type");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(INST_OP_FMUL, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* InstrBuilder::build_sdiv(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert((lhs->get_type()->is_integer_type() || 
        lhs->get_type()->is_pointer_type()) && 
        "lhs type must be an integer or a pointer");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(INST_OP_SDIV, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* InstrBuilder::build_udiv(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert((lhs->get_type()->is_integer_type() || 
        lhs->get_type()->is_pointer_type()) && 
        "lhs type must be an integer or a pointer");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(INST_OP_UDIV, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* InstrBuilder::build_fdiv(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert(lhs->get_type()->is_floating_point_type() && 
        "lhs type must be a floating point type");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(INST_OP_FDIV, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* InstrBuilder::build_srem(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert((lhs->get_type()->is_integer_type() || 
        lhs->get_type()->is_pointer_type()) && 
        "lhs type must be an integer or a pointer");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(INST_OP_SREM, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* InstrBuilder::build_urem(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert((lhs->get_type()->is_integer_type() || 
        lhs->get_type()->is_pointer_type()) && 
        "lhs type must be an integer or a pointer");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(INST_OP_UREM, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* InstrBuilder::build_and(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert((lhs->get_type()->is_integer_type() || 
        lhs->get_type()->is_pointer_type()) && 
        "lhs type must be an integer or a pointer");
    assert((rhs->get_type()->is_integer_type() ||
        rhs->get_type()->is_pointer_type()) && 
        "rhs type must be an integer or a pointer");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(INST_OP_AND, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* InstrBuilder::build_or(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert((lhs->get_type()->is_integer_type() || 
        lhs->get_type()->is_pointer_type()) && 
        "lhs type must be an integer or a pointer");
    assert((rhs->get_type()->is_integer_type() ||
        rhs->get_type()->is_pointer_type()) && 
        "rhs type must be an integer or a pointer");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(INST_OP_OR, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* InstrBuilder::build_xor(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert((lhs->get_type()->is_integer_type() || 
        lhs->get_type()->is_pointer_type()) && 
        "lhs type must be an integer or a pointer");
    assert((rhs->get_type()->is_integer_type() ||
        rhs->get_type()->is_pointer_type()) && 
        "rhs type must be an integer or a pointer");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(INST_OP_XOR, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* InstrBuilder::build_shl(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert((lhs->get_type()->is_integer_type() || 
        lhs->get_type()->is_pointer_type()) && 
        "lhs type must be an integer or a pointer");
    assert((rhs->get_type()->is_integer_type() ||
        rhs->get_type()->is_pointer_type()) && 
        "rhs type must be an integer or a pointer");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(INST_OP_SHL, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* InstrBuilder::build_shr(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert((lhs->get_type()->is_integer_type() || 
        lhs->get_type()->is_pointer_type()) && 
        "lhs type must be an integer or a pointer");
    assert((rhs->get_type()->is_integer_type() ||
        rhs->get_type()->is_pointer_type()) && 
        "rhs type must be an integer or a pointer");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(INST_OP_SHR, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* InstrBuilder::build_sar(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null");
    assert(rhs && "rhs cannot be null");
    assert((lhs->get_type()->is_integer_type() || 
        lhs->get_type()->is_pointer_type()) && 
        "lhs type must be an integer or a pointer");
    assert((rhs->get_type()->is_integer_type() ||
        rhs->get_type()->is_pointer_type()) && 
        "rhs type must be an integer or a pointer");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type");

    return insert(INST_OP_SAR, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* InstrBuilder::build_not(Value* value) {
    assert(value && "value cannot be null");
    assert(value->get_type()->is_integer_type() && 
        "value type must be an integer");

    return insert(INST_OP_NOT, m_cfg.get_def_id(), value->get_type(), { value });
}

Instruction* InstrBuilder::build_ineg(Value* value) {
    assert(value && "value cannot be null");
    assert(value->get_type()->is_integer_type() && 
        "value type must be an integer");

    return insert(INST_OP_INEG, m_cfg.get_def_id(), value->get_type(), { value });
}

Instruction* InstrBuilder::build_fneg(Value* value) {
    assert(value && "value cannot be null");
    assert(value->get_type()->is_floating_point_type() && 
        "value type must be a floating point type");

    return insert(INST_OP_FNEG, m_cfg.get_def_id(), value->get_type(), { value });
}

Instruction* InstrBuilder::build_sext(const Type* type, Value* value) {
    assert(value && "value cannot be null");
    assert(value->get_type()->is_integer_type() && 
        "value type must be an integer");
    assert(type->is_integer_type() && "type must be an integer");

    return insert(INST_OP_SEXT, m_cfg.get_def_id(), type, { value });
}

Instruction* InstrBuilder::build_zext(const Type* type, Value* value) {
    assert(value && "value cannot be null");
    assert(value->get_type()->is_integer_type() && 
        "value type must be an integer");
    assert(type->is_integer_type() && "type must be an integer");

    return insert(INST_OP_ZEXT, m_cfg.get_def_id(), type, { value });
}

Instruction* InstrBuilder::build_fext(const Type* type, Value* value) {
    assert(value && "value cannot be null");
    assert(value->get_type()->is_floating_point_type() && 
        "value type must be a floating point type");
    assert(type->is_floating_point_type() &&
        "type must be a floating point type");

    return insert(INST_OP_FEXT, m_cfg.get_def_id(), type, { value });
}

Instruction* InstrBuilder::build_itrunc(const Type* type, Value* value) {
    assert(value && "value cannot be null");
    assert(value->get_type()->is_integer_type() && 
        "value type must be an integer");
    assert(type->is_integer_type() && "type must be an integer");

    return insert(INST_OP_ITRUNC, m_cfg.get_def_id(), type, { value });
}

Instruction* InstrBuilder::build_ftrunc(const Type* type, Value* value) {
    assert(value && "value cannot be null");
    assert(value->get_type()->is_floating_point_type() && 
        "value type must be a floating point type");
    assert(type->is_floating_point_type() &&
        "type must be a floating point type");

    return insert(INST_OP_FTRUNC, m_cfg.get_def_id(), type, { value });
}

Instruction* InstrBuilder::build_si2fp(const Type* type, Value* value) {
    assert(value && "value cannot be null");
    assert(value->get_type()->is_integer_type() && 
        "value type must be an integer");
    assert(type->is_floating_point_type() &&
        "type must be a floating point type");

    return insert(INST_OP_SI2FP, m_cfg.get_def_id(), type, { value });
}

Instruction* InstrBuilder::build_ui2fp(const Type* type, Value* value) {
    assert(value && "value cannot be null");
    assert(value->get_type()->is_integer_type() && 
        "value type must be an integer");
    assert(type->is_floating_point_type() &&
        "type must be a floating point type");

    return insert(INST_OP_UI2FP, m_cfg.get_def_id(), type, { value });
}

Instruction* InstrBuilder::build_fp2si(const Type* type, Value* value) {
    assert(value && "value cannot be null");
    assert(value->get_type()->is_floating_point_type() && 
        "value type must be a floating point type");
    assert(type->is_integer_type() &&
        "type must be an integer");

    return insert(INST_OP_FP2SI, m_cfg.get_def_id(), type, { value });
}

Instruction* InstrBuilder::build_fp2ui(const Type* type, Value* value) {
    assert(value && "value cannot be null");
    assert(value->get_type()->is_floating_point_type() && 
        "value type must be a floating point type");
    assert(type->is_integer_type() &&
        "type must be an integer");

    return insert(INST_OP_FP2UI, m_cfg.get_def_id(), type, { value });
}

Instruction* InstrBuilder::build_p2i(const Type* type, Value* value) {
    assert(value && "value cannot be null");
    assert(value->get_type()->is_pointer_type() && 
        "value type must be a pointer type");
    assert(type->is_integer_type() &&
        "type must be an integer");

    return insert(INST_OP_P2I, m_cfg.get_def_id(), type, { value });
}

Instruction* InstrBuilder::build_i2p(const Type* type, Value* value) {
    assert(value && "value cannot be null");
    assert(value->get_type()->is_integer_type() && 
        "value type must be an integer");
    assert(type->is_pointer_type() &&
        "type must be a pointer type");

    return insert(INST_OP_I2P, m_cfg.get_def_id(), type, { value });
}

Instruction* InstrBuilder::build_reint(const Type* type, Value* value) {
    assert(value && "value cannot be null");
    assert(value->get_type()->is_pointer_type() || value->get_type()->is_array_type());
    assert(type->is_pointer_type() &&
        "type must be a pointer type");

    return insert(INST_OP_REINTERPET, m_cfg.get_def_id(), type, { value });
}

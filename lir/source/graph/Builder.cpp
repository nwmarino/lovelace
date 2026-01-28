//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lir/graph/CFG.hpp"
#include "lir/graph/Builder.hpp"
#include "lir/graph/Constant.hpp"
#include "lir/graph/Instruction.hpp"

using namespace lir;

void Builder::insert(Instruction* inst) {
    if (!m_insert)
        return;

    switch (m_mode) {
        case InsertMode::Prepend:
            m_insert->prepend(inst);
            break;

        case InsertMode::Append:
            m_insert->append(inst);
            break;
    }
}

Instruction* Builder::insert(Mnemonic op, uint32_t def, Type* type, 
                             const std::vector<Value*>& ops,
                             Descriptor desc) {
    Instruction* inst = new Instruction(type, op, nullptr, def, desc, ops);
    insert(inst);
    return inst;
}

Instruction* Builder::build_string(String* string) {
    assert(string && "string cannot be null!");
    return insert(OP_STRING, m_cfg.get_def_id(), string->get_type(), { string });
}

Instruction* Builder::build_load(Type* type, Value* source) {
    assert(type && "type cannot be null!");
    assert(source && "source cannot be null!");
    assert(source->get_type()->is_pointer_type() && "source must be a pointer!");

    uint16_t alignment = m_cfg.get_machine().get_align(type);

    return insert(
        OP_LOAD, 
        m_cfg.get_def_id(), 
        type, 
        { source }, 
        { .alignment = alignment });
}

Instruction* Builder::build_store(Value* value, Value* dest) {
    assert(value && "value cannot be null!");
    assert(dest && "dst cannot be null!");
    assert(dest->get_type()->is_pointer_type() && "dest must be a pointer!");
    
    uint16_t alignment = m_cfg.get_machine().get_align(value->get_type());

    return insert(
        OP_STORE, 
        0, 
        VoidType::get(m_cfg), 
        { value, dest },
        { .alignment = alignment });
}

Instruction* Builder::build_pwalk(Type* type, Value* source, 
                                  const std::vector<Value*>& indices) {
    assert(type && "type cannot be null!");
    assert(type->is_pointer_type() && "result must be a pointer!");
    assert(source && "source cannot be null!");
    assert(source->get_type()->is_pointer_type() && "source must be a pointer!");
    assert(!indices.empty() && "index list cannot be empty!");

    std::vector<Value*> ops(indices.size() + 1, nullptr);
    ops[0] = source;
    for (uint32_t i = 0; i < indices.size(); ++i)
        ops[i + 1] = indices[i];

    return insert(OP_PWALK, m_cfg.get_def_id(), type, ops);
}

Instruction* Builder::build_jif(Value* cond, BasicBlock* true_dest, 
                                const std::vector<Value*>& true_args,
                                BasicBlock* false_dest,
                                const std::vector<Value*>& false_args) {
    assert(cond && "cond cannot be null!");
    assert(true_dest && "true_dest cannot be null!");
    assert(false_dest && "false_dest cannot be null!");
    assert(true_dest->num_args() == true_args.size() && "argument count mismatch!");
    assert(false_dest->num_args() == false_args.size() && "argument count mismatch!");
    assert(cond->get_type()->is_integer_type(1) && "cond must be a boolean!");
    
    if (m_insert) {
        // Add |true_dest|, |false_dest| as successors to the current block.
        m_insert->get_succs().push_back(true_dest);
        m_insert->get_succs().push_back(false_dest);

        // Add the current block as a predecessor to |true_dest|, |false_dest|.
        true_dest->get_preds().push_back(m_insert);
        false_dest->get_preds().push_back(m_insert);
    }

    std::vector<Value*> ops = { cond };
    ops.push_back(BlockAddress::get(m_cfg, true_dest));
    for (Value* arg : true_args)
        ops.push_back(arg);

    ops.push_back(BlockAddress::get(m_cfg, false_dest));
    for (Value* arg : false_args)
        ops.push_back(arg);

    return insert(OP_JIF, 0, VoidType::get(m_cfg), ops);
}

Instruction* Builder::build_jmp(BasicBlock* dest, const std::vector<Value*>& args) {
    assert(dest && "dest cannot be null!");
    assert(dest->num_args() == args.size() && "argument count mismatch!");

    if (m_insert) {
        // Add |dest| as a successor to the current block.
        m_insert->get_succs().push_back(dest);

        // Add the current block as a predecessor to |dest|.
        dest->get_preds().push_back(m_insert);
    }

    std::vector<Value*> ops = { BlockAddress::get(m_cfg, dest) };
    for (Value* arg : args)
        ops.push_back(arg);

    return insert(OP_JMP, 0, VoidType::get(m_cfg), ops);
}

Instruction* Builder::build_ret(Value* value) {
    return insert(OP_RET, 0, VoidType::get(m_cfg), { value });
}

Instruction* Builder::build_abort() {
    return insert(OP_ABORT, 0, VoidType::get(m_cfg));
}

Instruction* Builder::build_unreachable() {
    return insert(OP_UNREACHABLE, 0, VoidType::get(m_cfg));
}

Instruction* Builder::build_call(FunctionType* type, Value* callee, 
                                 const std::vector<Value*>& args) {
    assert(type && "type cannot be null!");
    assert(callee && "callee cannot be null!");

    std::vector<Value*> operands = { callee };
    for (Value* arg : args)
        operands.push_back(arg);

    // @Todo: figure out what to do with multiple function result values.
    return insert(
        OP_CALL,
        type->has_results() ? m_cfg.get_def_id() : 0,
        type->get_result(0),
        operands);
}

Instruction* Builder::build_cmp_ieq(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type!");

    return insert(
        OP_CMP, 
        m_cfg.get_def_id(), 
        Type::get_i1_type(m_cfg), 
        { lhs, rhs },
        { .cmp = CMP_IEQ });
}

Instruction* Builder::build_cmp_ine(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type!");

    return insert(
        OP_CMP, 
        m_cfg.get_def_id(), 
        Type::get_i1_type(m_cfg), 
        { lhs, rhs },
        { .cmp = CMP_INE });
}

Instruction* Builder::build_cmp_oeq(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type!");

    return insert(
        OP_CMP, 
        m_cfg.get_def_id(), 
        Type::get_i1_type(m_cfg), 
        { lhs, rhs },
        { .cmp = CMP_OEQ });
}

Instruction* Builder::build_cmp_one(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type!");

    return insert(
        OP_CMP, 
        m_cfg.get_def_id(), 
        Type::get_i1_type(m_cfg), 
        { lhs, rhs },
        { .cmp = CMP_ONE });
}

Instruction* Builder::build_cmp_slt(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type!");

    return insert(
        OP_CMP, 
        m_cfg.get_def_id(), 
        Type::get_i1_type(m_cfg), 
        { lhs, rhs },
        { .cmp = CMP_SLT });
}

Instruction* Builder::build_cmp_sle(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type!");

    return insert(
        OP_CMP, 
        m_cfg.get_def_id(), 
        Type::get_i1_type(m_cfg), 
        { lhs, rhs },
        { .cmp = CMP_SLE });
}

Instruction* Builder::build_cmp_sgt(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type!");

    return insert(
        OP_CMP, 
        m_cfg.get_def_id(), 
        Type::get_i1_type(m_cfg), 
        { lhs, rhs },
        { .cmp = CMP_SGT });
}

Instruction* Builder::build_cmp_sge(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type!");

    return insert(
        OP_CMP, 
        m_cfg.get_def_id(), 
        Type::get_i1_type(m_cfg), 
        { lhs, rhs },
        { .cmp = CMP_SGE });
}

Instruction* Builder::build_cmp_ult(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type!");

    return insert(
        OP_CMP, 
        m_cfg.get_def_id(), 
        Type::get_i1_type(m_cfg), 
        { lhs, rhs },
        { .cmp = CMP_ULT });
}

Instruction* Builder::build_cmp_ule(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type!");

    return insert(
        OP_CMP, 
        m_cfg.get_def_id(), 
        Type::get_i1_type(m_cfg), 
        { lhs, rhs },
        { .cmp = CMP_ULE });
}

Instruction* Builder::build_cmp_ugt(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type!");

    return insert(
        OP_CMP, 
        m_cfg.get_def_id(), 
        Type::get_i1_type(m_cfg), 
        { lhs, rhs },
        { .cmp = CMP_UGT });
}

Instruction* Builder::build_cmp_uge(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type!");

    return insert(
        OP_CMP, 
        m_cfg.get_def_id(), 
        Type::get_i1_type(m_cfg), 
        { lhs, rhs },
        { .cmp = CMP_UGE });
}

Instruction* Builder::build_cmp_olt(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type!");

    return insert(
        OP_CMP, 
        m_cfg.get_def_id(), 
        Type::get_i1_type(m_cfg), 
        { lhs, rhs },
        { .cmp = CMP_OLT });
}

Instruction* Builder::build_cmp_ole(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type!");

    return insert(
        OP_CMP, 
        m_cfg.get_def_id(), 
        Type::get_i1_type(m_cfg), 
        { lhs, rhs },
        { .cmp = CMP_OLE });
}

Instruction* Builder::build_cmp_ogt(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type!");

    return insert(
        OP_CMP, 
        m_cfg.get_def_id(), 
        Type::get_i1_type(m_cfg), 
        { lhs, rhs },
        { .cmp = CMP_OGT });
}

Instruction* Builder::build_cmp_oge(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "lhs and rhs must have the same type!");

    return insert(
        OP_CMP, 
        m_cfg.get_def_id(), 
        Type::get_i1_type(m_cfg), 
        { lhs, rhs },
        { .cmp = CMP_OGE });
}

Instruction* Builder::build_iadd(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "both operands must have the same type!");

    return insert(OP_IADD, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* Builder::build_isub(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "both operands must have the same type!");

    return insert(OP_ISUB, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* Builder::build_imul(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "both operands must have the same type!");

    return insert(OP_IMUL, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* Builder::build_sdiv(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "both operands must have the same type!");

    return insert(OP_SDIV, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* Builder::build_udiv(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "both operands must have the same type!");

    return insert(OP_UDIV, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* Builder::build_smod(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "both operands must have the same type!");

    return insert(OP_SMOD, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* Builder::build_umod(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "both operands must have the same type!");

    return insert(OP_UMOD, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* Builder::build_fadd(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(lhs->get_type()->is_float_type() && "lhs must be a float!");
    assert(rhs->get_type()->is_float_type() && "rhs must be a float!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "both operands must have the same type!");

    return insert(OP_FADD, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* Builder::build_fsub(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(lhs->get_type()->is_float_type() && "lhs must be a float!");
    assert(rhs->get_type()->is_float_type() && "rhs must be a float!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "both operands must have the same type!");

    return insert(OP_FSUB, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* Builder::build_fmul(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(lhs->get_type()->is_float_type() && "lhs must be a float!");
    assert(rhs->get_type()->is_float_type() && "rhs must be a float!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "both operands must have the same type!");

    return insert(OP_FMUL, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* Builder::build_fdiv(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(lhs->get_type()->is_float_type() && "lhs must be a float!");
    assert(rhs->get_type()->is_float_type() && "rhs must be a float!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "both operands must have the same type!");

    return insert(OP_FDIV, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* Builder::build_and(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "both operands must have the same type!");

    return insert(OP_AND, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* Builder::build_or(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "both operands must have the same type!");

    return insert(OP_OR, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* Builder::build_xor(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "both operands must have the same type!");

    return insert(OP_XOR, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* Builder::build_shl(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "both operands must have the same type!");

    return insert(OP_SHL, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* Builder::build_shr(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "both operands must have the same type!");

    return insert(OP_SHR, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* Builder::build_sar(Value* lhs, Value* rhs) {
    assert(lhs && "lhs cannot be null!");
    assert(rhs && "rhs cannot be null!");
    assert(lhs->get_type()->is_integer_type() && "lhs must be an integer!");
    assert(rhs->get_type()->is_integer_type() && "rhs must be an integer!");
    assert(*lhs->get_type() == *rhs->get_type() && 
        "both operands must have the same type!");

    return insert(OP_SAR, m_cfg.get_def_id(), lhs->get_type(), { lhs, rhs });
}

Instruction* Builder::build_not(Value* value) {
    assert(value && "value cannot be null!");
    assert(value->get_type()->is_integer_type() && "value must be an integer!");

    return insert(OP_NOT, m_cfg.get_def_id(), value->get_type(), { value });
}

Instruction* Builder::build_ineg(Value* value) {
    assert(value && "value cannot be null!");
    assert(value->get_type()->is_integer_type() && "value must be an integer!");
    
    return insert(OP_INEG, m_cfg.get_def_id(), value->get_type(), { value });
}

Instruction* Builder::build_fneg(Value* value) {
    assert(value && "value cannot be null!");
    assert(value->get_type()->is_float_type() && "value must be a float!");
    
    return insert(OP_FNEG, m_cfg.get_def_id(), value->get_type(), { value });
}

Instruction* Builder::build_sext(Type* type, Value* value) {
    assert(value && "value cannot be null!");
    assert(value->get_type()->is_integer_type() && "value must be an integer!");
    assert(type->is_integer_type() && "destination type must be an integer!");

    return insert(OP_SEXT, m_cfg.get_def_id(), type, { value });
}

Instruction* Builder::build_zext(Type* type, Value* value) {
    assert(value && "value cannot be null!");
    assert(value->get_type()->is_integer_type() && "value must be an integer!");
    assert(type->is_integer_type() && "destination type must be an integer!");

    return insert(OP_ZEXT, m_cfg.get_def_id(), type, { value });
}

Instruction* Builder::build_fext(Type* type, Value* value) {
    assert(value && "value cannot be null!");
    assert(value->get_type()->is_float_type() && "value must be a float!");
    assert(type->is_float_type() && "destination type must be a float!");

    return insert(OP_FEXT, m_cfg.get_def_id(), type, { value });
}

Instruction* Builder::build_itrunc(Type* type, Value* value) {
    assert(value && "value cannot be null!");
    assert(value->get_type()->is_integer_type() && "value must be an integer!");
    assert(type->is_integer_type() && "destination type must be an integer!");

    return insert(OP_ITRUNC, m_cfg.get_def_id(), type, { value });
}

Instruction* Builder::build_ftrunc(Type* type, Value* value) {
    assert(value && "value cannot be null!");
    assert(value->get_type()->is_float_type() && "value must be a float!");
    assert(type->is_float_type() && "destination type must be a float!");

    return insert(OP_FTRUNC, m_cfg.get_def_id(), type, { value });
}

Instruction* Builder::build_s2f(Type* type, Value* value) {
    assert(value && "value cannot be null!");
    assert(value->get_type()->is_integer_type() && "value must be an integer!");
    assert(type->is_float_type() && "destination type must be a float!");

    return insert(OP_S2F, m_cfg.get_def_id(), type, { value });
}

Instruction* Builder::build_u2f(Type* type, Value* value) {
    assert(value && "value cannot be null!");
    assert(value->get_type()->is_integer_type() && "value must be an integer!");
    assert(type->is_float_type() && "destination type must be a float!");

    return insert(OP_U2F, m_cfg.get_def_id(), type, { value });
}

Instruction* Builder::build_f2s(Type* type, Value* value) {
    assert(value && "value cannot be null!");
    assert(value->get_type()->is_float_type() && "value must be a float!");
    assert(type->is_integer_type() && "destination type must be an integer!");

    return insert(OP_F2S, m_cfg.get_def_id(), type, { value });
}

Instruction* Builder::build_f2u(Type* type, Value* value) {
    assert(value && "value cannot be null!");
    assert(value->get_type()->is_float_type() && "value must be a float!");
    assert(type->is_integer_type() && "destination type must be an integer!");

    return insert(OP_F2U, m_cfg.get_def_id(), type, { value });
}

Instruction* Builder::build_p2i(Type* type, Value* value) {
    assert(value && "value cannot be null!");
    assert(value->get_type()->is_pointer_type() && "value must a pointer!");
    assert(type->is_integer_type() && "destination type must be an integer!");

    return insert(OP_P2I, m_cfg.get_def_id(), type, { value });
}

Instruction* Builder::build_i2p(Type* type, Value* value) {
    assert(value && "value cannot be null!");
    assert(value->get_type()->is_integer_type() && "value must be an integer!");
    assert(type->is_pointer_type() && "destination type must be a pointer!");

    return insert(OP_I2P, m_cfg.get_def_id(), type, { value });
}

Instruction* Builder::build_reint(Type* type, Value* value) {
    assert(value && "value cannot be null!");
    assert(type->is_pointer_type() && "destination type must be a pointer!");

    return insert(OP_REINT, m_cfg.get_def_id(), type, { value });
}

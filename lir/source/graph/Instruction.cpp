//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lir/graph/BasicBlock.hpp"
#include "lir/graph/Constant.hpp"
#include "lir/graph/Instruction.hpp"

using namespace lir;

const char* lir::to_string(Mnemonic op) {
    switch (op) {
        case OP_ABORT:
            return "abort";
        case OP_AND:
            return "and";
        case OP_CALL:
            return "call";
        case OP_CMP:
            return "cmp";
        case OP_F2S:
            return "f2s";
        case OP_F2U:
            return "f2u";
        case OP_FADD:
            return "fadd";
        case OP_FDIV:
            return "fdiv";
        case OP_FEXT:
            return "fext";
        case OP_FMUL:
            return "fmul";
        case OP_FNEG:
            return "fneg";
        case OP_FSUB:
            return "fsub";
        case OP_FTRUNC:
            return "ftrunc";
        case OP_I2P:
            return "i2p";
        case OP_IADD:
            return "iadd";
        case OP_IMUL:
            return "imul";
        case OP_INEG:
            return "ineg";
        case OP_ISUB:
            return "isub";
        case OP_ITRUNC:
            return "itrunc";
        case OP_JIF:
            return "jif";
        case OP_JMP:
            return "jmp";
        case OP_LOAD:
            return "load";
        case OP_NOT:
            return "not";
        case OP_OR:
            return "or";
        case OP_P2I:
            return "p2i";
        case OP_PWALK:
            return "pwalk";
        case OP_REINT:
            return "reint";
        case OP_RET:
            return "ret";
        case OP_S2F:
            return "s2f";
        case OP_SAR:
            return "sar";
        case OP_SDIV:
            return "sdiv";
        case OP_SEXT:
            return "sext";
        case OP_SHL:
            return "shl";
        case OP_SHR:
            return "shr";
        case OP_SMOD:
            return "smod";
        case OP_STORE:
            return "store";
        case OP_STRING:
            return "string";
        case OP_U2F:
            return "u2f";
        case OP_UDIV:
            return "udiv";
        case OP_UMOD:
            return "umod";
        case OP_UNREACHABLE:
            return "unreachable";
        case OP_XOR:
            return "xor";
        case OP_ZEXT:
            return "zext";
    }
}

const char* lir::to_string(CMPPredicate pred) {
    switch (pred) {
        case CMP_IEQ:
            return "ieq";
        case CMP_INE:
            return "ine";
        case CMP_OEQ:
            return "oeq";
        case CMP_ONE:
            return "one";
        case CMP_SLT:
            return "slt";
        case CMP_SLE:
            return "sle";
        case CMP_SGT:
            return "sgt";
        case CMP_SGE:
            return "sge";
        case CMP_ULT:
            return "ult";
        case CMP_ULE:
            return "ule";
        case CMP_UGT:
            return "ugt";
        case CMP_UGE:
            return "uge";
        case CMP_OLT:
            return "olt";
        case CMP_OLE:
            return "ole";
        case CMP_OGT:
            return "ogt";
        case CMP_OGE:
            return "oge";
    }
}

void Instruction::detach() {
    assert(m_parent && "cannot detach a free-floating instruction!");
    m_parent->remove(this);
}

void Instruction::prepend_to(BasicBlock* block) {
    assert(block && "block cannot be null");
    block->prepend(this);
}

void Instruction::append_to(BasicBlock* block) {
    assert(block && "block cannot be null");
    block->append(this);
}

void Instruction::insert_before(Instruction* inst) {
    assert(inst && "inst cannot be null!");

    if (inst->get_prev())
        inst->get_prev()->set_next(this);

    m_prev = inst->get_prev();
    m_next = inst;

    inst->set_prev(this);
    m_parent = inst->get_parent();
}

void Instruction::insert_after(Instruction* inst) {
    assert(inst && "inst cannot be null!");

    if (inst->get_next())
        inst->get_next()->set_prev(this);

    m_prev = inst;
    m_next = inst->get_next();

    inst->set_next(this);
    m_parent = inst->get_parent();
}

bool Instruction::is_terminator() const {
    switch (op()) {
        case OP_ABORT:
        case OP_JIF:
        case OP_JMP:
        case OP_RET:
        case OP_UNREACHABLE:
            return true;
        default:
            return false;
    }
}

bool Instruction::is_cast() const {
    switch (op()) {
        case OP_F2S:
        case OP_F2U:
        case OP_FEXT:
        case OP_FTRUNC:
        case OP_I2P:
        case OP_ITRUNC:
        case OP_P2I:
        case OP_REINT:
        case OP_S2F:
        case OP_SEXT:
        case OP_U2F:
        case OP_ZEXT:
            return true;
        default:
            return false;
    }
}

std::vector<const Value*> Instruction::get_jump_args() const {
    assert(op() == OP_JMP);

    std::vector<const Value*> args = {};
    args.reserve(num_operands() - 1);

    for (uint32_t i = 1, e = num_operands(); i < e; ++i) // Skip label.
        args.push_back(get_operand(i));

    return args;
}

std::vector<const Value*> Instruction::get_jif_true_args() const {
    assert(op() == OP_JIF);

    std::vector<const Value*> args = {};

    for (uint32_t i = 2, e = num_operands(); i < e; ++i) { // Skip condition and "true" label.
        const Value* oper = get_operand(i);
        if (dynamic_cast<const BlockAddress*>(oper))
            break; // Stop at first label.
        
        args.push_back(oper);
    }

    return args;
}

std::vector<const Value*> Instruction::get_jif_false_args() const {
    assert(op() == OP_JIF);

    uint32_t i = 2, e = num_operands(); // i: skip condition and first label.
    for (; i < e; ++i) {
        if (dynamic_cast<const BlockAddress*>(get_operand(i)))
            break; // Stop at second label.
    }

    i += 1; // move past the second label.

    std::vector<const Value*> args = {};
    for (; i < e; ++i)
        args.push_back(get_operand(i));
    
    return args;
}

bool Instruction::is_trivially_dead() const {
    return false; // @Todo: no DCE for now.

    if (get_def() == 0 || Value::used())
        return false;

    //@ Todo: not quite right.
    return m_op != OP_CALL;
}

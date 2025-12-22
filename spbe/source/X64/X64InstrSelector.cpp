//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "spbe/graph/Constant.hpp"
#include "spbe/graph/Function.hpp"
#include "spbe/graph/Global.hpp"
#include "spbe/graph/InlineAsm.hpp"
#include "spbe/graph/Instruction.hpp"
#include "spbe/graph/Local.hpp"
#include "spbe/machine/MachFunction.hpp"
#include "spbe/machine/MachInstruction.hpp"
#include "spbe/machine/MachOperand.hpp"
#include "spbe/machine/MachRegister.hpp"
#include "spbe/X64/X64.hpp"
#include "spbe/X64/X64InstrSelector.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

using namespace spbe;
using namespace spbe::x64;

MachRegister X64InstrSelector::as_register(const Instruction* inst) {
    assert(inst->result_id() != 0 && "instruction does not produce a value!");

    // Determine the class based on the type of |inst|'s result.
    RegisterClass cls = GeneralPurpose;
    if (inst->get_type()->is_floating_point_type())
        cls = FloatingPoint;

    MachRegister vreg = get_temporary(cls);

    // Track the register mapping so that references to |inst| can reuse the 
    // same register later.
    m_regs.emplace(inst->result_id(), vreg);
    return vreg;
}

MachRegister X64InstrSelector::get_temporary(RegisterClass cls) {
    VRegInfo info = {};
    info.cls = cls;
    info.alloc = MachRegister::NoRegister;

    auto& regi = m_function.get_register_info();
    uint32_t next_id = regi.vregs.size() + MachRegister::VirtualBarrier;
    regi.vregs.emplace(next_id, info);
    return next_id;
}

uint16_t X64InstrSelector::get_subregister(const Type* type) const {
    if (!type)
        return 0;

    uint32_t size = m_target.get_type_size(type);
    assert((size == 1 || size == 2 || size == 4 || size == 8) &&
        "cannot determine x64 subregister for a non-scalar type!");

    return size;
}

MachOperand X64InstrSelector::as_operand(const Value* value) {
    if (auto integer = dynamic_cast<const ConstantInt*>(value)) {
        return MachOperand::create_imm(integer->get_value());

        MachOperand reg = MachOperand::create_reg(
            get_temporary(GeneralPurpose), 
            get_subregister(value->get_type()), 
            true);

        emit(get_move_op(value->get_type()))
            .add_imm(integer->get_value())
            .add_operand(reg);

        reg.set_is_use();
        return reg;

    } else if (auto fp = dynamic_cast<const ConstantFP*>(value)) {
        MachOperand reg = MachOperand::create_reg(
            get_temporary(FloatingPoint), 0, true);

        uint32_t const_index = 
            m_function.get_constant_pool().get_or_create_constant(
                fp, m_target.get_type_align(value->get_type()));

        emit(get_move_op(value->get_type()))
            .add_constant_index(const_index)
            .add_operand(reg);

        reg.set_is_use();
        return reg;

    } else if (auto null = dynamic_cast<const ConstantNull*>(value)) {
        MachOperand reg = MachOperand::create_reg(
            get_temporary(GeneralPurpose), 8, true);

        emit(x64::MOV64)
            .add_imm(0)
            .add_operand(reg);

        reg.set_is_use();
        return reg;

    } else if (auto block = dynamic_cast<const BlockAddress*>(value)) {
        // TODO: Change to adjust for position changes in of blocks in the
        // machine functions.
        return MachOperand::create_block(
            m_function.at(block->get_block()->get_number()));

    } else if (auto global = dynamic_cast<const Global*>(value)) {
        return MachOperand::create_symbol(global->get_name().c_str());

    } else if (auto arg = dynamic_cast<const Argument*>(value)) {
        return as_argument(value, arg->get_number());

    } else if (auto fn = dynamic_cast<const Function*>(value)) {
        return MachOperand::create_symbol(fn->get_name().c_str());

    } else if (auto local = dynamic_cast<const Local*>(value)) {
        return MachOperand::create_stack_index(m_locals[local]);

    } else if (auto inst = dynamic_cast<const Instruction*>(value)) {
        assert(m_regs.count(inst->result_id()) != 0 &&
            "instruction has not been mapped to x64 yet!");

        return MachOperand::create_reg(
            m_regs[inst->result_id()], 
            get_subregister(value->get_type()), 
            false);
    }

    assert(false && "cannot lower value to an x64 machine operand!");
}

MachOperand X64InstrSelector::as_argument(
        const Value* value, uint32_t index) const {
    if (index < 6) {
        MachRegister reg;

        if (value->get_type()->is_floating_point_type()) {
            const MachRegister table[6] = {
                x64::XMM0,
                x64::XMM1,
                x64::XMM2,
                x64::XMM3,
                x64::XMM4,
                x64::XMM5,
            };

            reg = table[index];
        } else {
            const MachRegister table[6] = {
                x64::RDI,
                x64::RSI,
                x64::RDX,
                x64::RCX,
                x64::R8,
                x64::R9,
            };

            reg = table[index];
        }

        return MachOperand::create_reg(
            reg,
            get_subregister(value->get_type()), 
            true);
    }

    // TODO: Allow spilling.
    assert(false && "calls with more than 6 arguments not supported!");
}

x64::Opcode X64InstrSelector::get_jcc_op(spbe::Opcode op) const {
    switch (op) {
    case INST_OP_CMP_IEQ:   return x64::JE;
    case INST_OP_CMP_INE:   return x64::JNE;
    case INST_OP_CMP_OEQ:   return x64::JE;
    case INST_OP_CMP_ONE:   return x64::JNE;
    case INST_OP_CMP_UNEQ:  return x64::JE;
    case INST_OP_CMP_UNNE:  return x64::JNE;
    case INST_OP_CMP_SLT:   return x64::JL;
    case INST_OP_CMP_SLE:   return x64::JLE;
    case INST_OP_CMP_SGT:   return x64::JG;
    case INST_OP_CMP_SGE:   return x64::JGE;
    case INST_OP_CMP_ULT:   return x64::JB;
    case INST_OP_CMP_ULE:   return x64::JBE;
    case INST_OP_CMP_UGT:   return x64::JA;
    case INST_OP_CMP_UGE:   return x64::JAE;
    case INST_OP_CMP_OLT:   return x64::JB;
    case INST_OP_CMP_OLE:   return x64::JBE;
    case INST_OP_CMP_OGT:   return x64::JA;
    case INST_OP_CMP_OGE:   return x64::JAE;
    case INST_OP_CMP_UNLT:  return x64::JB;
    case INST_OP_CMP_UNLE:  return x64::JBE;
    case INST_OP_CMP_UNGT:  return x64::JA;
    case INST_OP_CMP_UNGE:  return x64::JAE;
    default:
        assert(false && "expected comparison opcode!");
    }
}

x64::Opcode X64InstrSelector::get_setcc_op(spbe::Opcode op) const {
    switch (op) {
    case INST_OP_CMP_IEQ:   return x64::SETE;
    case INST_OP_CMP_INE:   return x64::SETNE;
    case INST_OP_CMP_OEQ:   return x64::SETE;
    case INST_OP_CMP_ONE:   return x64::SETNE;
    case INST_OP_CMP_UNEQ:  return x64::SETE;
    case INST_OP_CMP_UNNE:  return x64::SETNE;
    case INST_OP_CMP_SLT:   return x64::SETL;
    case INST_OP_CMP_SLE:   return x64::SETLE;
    case INST_OP_CMP_SGT:   return x64::SETG;
    case INST_OP_CMP_SGE:   return x64::SETGE;
    case INST_OP_CMP_ULT:   return x64::SETB;
    case INST_OP_CMP_ULE:   return x64::SETBE;
    case INST_OP_CMP_UGT:   return x64::SETA;
    case INST_OP_CMP_UGE:   return x64::SETAE;
    case INST_OP_CMP_OLT:   return x64::SETB;
    case INST_OP_CMP_OLE:   return x64::SETBE;
    case INST_OP_CMP_OGT:   return x64::SETA;
    case INST_OP_CMP_OGE:   return x64::SETAE;
    case INST_OP_CMP_UNLT:  return x64::SETB;
    case INST_OP_CMP_UNLE:  return x64::SETBE;
    case INST_OP_CMP_UNGT:  return x64::SETA;
    case INST_OP_CMP_UNGE:  return x64::SETAE;
    default:
        assert(false && "expected comparison opcode!");
    }
}

x64::Opcode X64InstrSelector::get_move_op(const Type* type) const {
    assert(type && "type cannot be null!");

    uint32_t size = m_target.get_type_size_in_bits(type);
    switch (size) {
    case 1:
    case 8:
        return x64::MOV8;
    case 16:
        return x64::MOV16;
    case 32:
        if (type->is_floating_point_type()) {
            return x64::MOVSS;
        } else {
            return x64::MOV32;
        }
    case 64:
        if (type->is_floating_point_type()) {
            return x64::MOVSD;
        } else {
            return x64::MOV64;
        }
    }

    assert(false && "cannot determine move opcode based on type!");
}

x64::Opcode X64InstrSelector::get_cmp_op(const Type* type) const {
    assert(type && "type cannot be null!");

    switch (m_target.get_type_size_in_bits(type)) {
    case 1:
    case 8:
        return x64::CMP8;
    case 16:
        return x64::CMP16;
    case 32:
        if (type->is_floating_point_type()) {
            return x64::UCOMISS;
        } else {
            return x64::CMP32;
        }
    case 64:
        if (type->is_floating_point_type()) {
            return x64::UCOMISD;
        } else {
            return x64::CMP64;
        }
    }

    assert(false && "cannot determine cmp opcode based on type!");
}

x64::Opcode X64InstrSelector::get_add_op(const Type* type) const {
    assert(type && "type cannot be null!");

    switch (m_target.get_type_size_in_bits(type)) {
    case 1:
    case 8:
        return x64::ADD8;
    case 16:
        return x64::ADD16;
    case 32:
        if (type->is_floating_point_type()) {
            return x64::ADDSS;
        } else {
            return x64::ADD32;
        }
    case 64:
        if (type->is_floating_point_type()) {
            return x64::ADDSD;
        } else {
            return x64::ADD64;
        }
    }

    assert(false && "cannot determine add opcode based on type!");
}

x64::Opcode X64InstrSelector::get_sub_op(const Type* type) const {
    assert(type && "type cannot be null!");

    switch (m_target.get_type_size_in_bits(type)) {
    case 1:
    case 8:
        return x64::SUB8;
    case 16:
        return x64::SUB16;
    case 32:
        if (type->is_floating_point_type()) {
            return x64::SUBSS;
        } else {
            return x64::SUB32;
        }
    case 64:
        if (type->is_floating_point_type()) {
            return x64::SUBSD;
        } else {
            return x64::SUB64;
        }
    }   

    assert(false && "cannot determine sub opcode based on type!");
}

x64::Opcode X64InstrSelector::get_imul_op(const Type* type) const {
    assert(type && "type cannot be null!");

    switch (m_target.get_type_size_in_bits(type)) {
    case 1:
    case 8:
        return x64::IMUL8;
    case 16:
        return x64::IMUL16;
    case 32:
        return x64::IMUL32;
    case 64:
        return x64::IMUL64;
    }

    assert(false && "cannot determine imul opcode based on type!");
}

x64::Opcode X64InstrSelector::get_mul_op(const Type* type) const {
    assert(type && "type cannot be null!");

    switch (m_target.get_type_size_in_bits(type)) {
    case 1:
    case 8:
        return x64::MUL8;
    case 16:
        return x64::MUL16;
    case 32:
        if (type->is_floating_point_type()) {
            return x64::MULSS;
        } else {
            return x64::MUL32;
        }
    case 64:
        if (type->is_floating_point_type()) {
            return x64::MULSD;
        } else {
            return x64::MUL64;
        }
    }

    assert(false && "cannot determine mul opcode based on type!");
}

x64::Opcode X64InstrSelector::get_idiv_op(const Type* type) const {
    assert(type && "type cannot be null!");

    switch (m_target.get_type_size_in_bits(type)) {
    case 1:
    case 8:
        return x64::IDIV8;
    case 16:
        return x64::IDIV16;
    case 32:
        return x64::IDIV32;
    case 64:
        return x64::IDIV64;
    }

    assert(false && "cannot determine idiv opcode based on type!");
}

x64::Opcode X64InstrSelector::get_div_op(const Type* type) const {
    assert(type && "type cannot be null!");

    switch (m_target.get_type_size_in_bits(type)) {
    case 1:
    case 8:
        return x64::DIV8;
    case 16:
        return x64::DIV16;
    case 32:
        if (type->is_floating_point_type()) {
            return x64::DIVSS;
        } else {
            return x64::DIV32;
        }
    case 64:
        if (type->is_floating_point_type()) {
            return x64::DIVSD;
        } else {
            return x64::DIV64;
        }
    }

    assert(false && "cannot determine div opcode based on type!");
}

x64::Opcode X64InstrSelector::get_and_op(const Type* type) const {
    assert(type && "type cannot be null!");

    switch (m_target.get_type_size_in_bits(type)) {
    case 1:
    case 8:
        return x64::AND8;
    case 16:
        return x64::AND16;
    case 32:
        return x64::AND32;
    case 64:
        return x64::AND64;
    }

    assert(false && "cannot determine and opcode based on type!");
}

x64::Opcode X64InstrSelector::get_or_op(const Type* type) const {
    assert(type && "type cannot be null!");

    switch (m_target.get_type_size_in_bits(type)) {
    case 1:
    case 8:
        return x64::OR8;
    case 16:
        return x64::OR16;
    case 32:
        return x64::OR32;
    case 64:
        return x64::OR64;
    }

    assert(false && "cannot determine or opcode based on type!");
}

x64::Opcode X64InstrSelector::get_xor_op(const Type* type) const {
    assert(type && "type cannot be null!");

    switch (m_target.get_type_size_in_bits(type)) {
    case 1:
    case 8:
        return x64::XOR8;
    case 16:
        return x64::XOR16;
    case 32:
        return x64::XOR32;
    case 64:
        return x64::XOR64;
    }

    assert(false && "cannot determine xor opcode based on type!");
}

x64::Opcode X64InstrSelector::get_shl_op(const Type* type) const {
    assert(type && "type cannot be null!");

    switch (m_target.get_type_size_in_bits(type)) {
    case 1:
    case 8:
        return x64::SHL8;
    case 16:
        return x64::SHL16;
    case 32:
        return x64::SHL32;
    case 64:
        return x64::SHL64;
    }

    assert(false && "cannot determine shl opcode based on type!");
}

x64::Opcode X64InstrSelector::get_shr_op(const Type* type) const {
    assert(type && "type cannot be null!");

    switch (m_target.get_type_size_in_bits(type)) {
    case 1:
    case 8:
        return x64::SHR8;
    case 16:
        return x64::SHR16;
    case 32:
        return x64::SHR32;
    case 64:
        return x64::SHR64;
    }

    assert(false && "cannot determine shr opcode based on type!");
}

x64::Opcode X64InstrSelector::get_sar_op(const Type* type) const {
    assert(type && "type cannot be null!");

    switch (m_target.get_type_size_in_bits(type)) {
    case 1:
    case 8:
        return x64::SAR8;
    case 16:
        return x64::SAR16;
    case 32:
        return x64::SAR32;
    case 64:
        return x64::SAR64;
    }

    assert(false && "cannot determine sar opcode based on type!");
}

x64::Opcode X64InstrSelector::get_not_op(const Type* type) const {
    assert(type && "type cannot be null!");

    switch (m_target.get_type_size_in_bits(type)) {
    case 1:
    case 8:
        return x64::NOT8;
    case 16:
        return x64::NOT16;
    case 32:
        return x64::NOT32;
    case 64:
        return x64::NOT64;
    }

    assert(false && "cannot determine not opcode based on type!");
}

x64::Opcode X64InstrSelector::get_neg_op(const Type* type) const {
    assert(type && "type cannot be null!");

    switch (m_target.get_type_size_in_bits(type)) {
    case 1:
    case 8:
        return x64::NEG8;
    case 16:
        return x64::NEG16;
    case 32:
        return x64::NEG32;
    case 64:
        return x64::NEG64;
    }

    assert(false && "cannot determine neg opcode based on type!");
}

x64::Opcode X64InstrSelector::flip_jcc(x64::Opcode jcc) const {
    switch (jcc) {
    case JE:
    case JNE:
    case JZ:
    case JNZ:
        return jcc;
    case JL:
        return JG;
    case JLE:
        return JGE;
    case JG:
        return JL;
    case JGE:
        return JLE;
    case JA:
        return JB;
    case JAE:
        return JBE;
    case JB:
        return JA;
    case JBE:
        return JAE;
    default:
        assert(false && "cannot flip non-jcc opcode!");
    }
}

x64::Opcode X64InstrSelector::flip_setcc(x64::Opcode setcc) const {
    switch (setcc) {
    case SETE:
    case SETNE:
    case SETZ:
    case SETNZ:
        return setcc;
    case SETL:
        return SETG;
    case SETLE:
        return SETGE;
    case SETG:
        return SETL;
    case SETGE:
        return SETLE;
    case SETA:
        return SETB;
    case SETAE:
        return SETBE;
    case SETB:
        return SETA;
    case SETBE:
        return SETAE;
    default:
        assert(false && "cannot flip non-setcc opcode!");
    }
}

x64::Opcode X64InstrSelector::negate_jcc(x64::Opcode jcc) const {
    switch (jcc) {
    case JE:    return JNE;
    case JNE:   return JE;
    case JZ:    return JNZ;
    case JNZ:   return JE;
    case JL:    return JGE;
    case JLE:   return JG;
    case JG:    return JLE;
    case JGE:   return JL;
    case JA:    return JBE;
    case JAE:   return JB;
    case JB:    return JAE;
    case JBE:   return JA;
    default:
        assert(false && "cannot negate non-jcc opcode!");
    }
}

x64::Opcode X64InstrSelector::negate_setcc(x64::Opcode setcc) const {
    switch (setcc) {
    case SETE:  return SETNE;
    case SETNE: return SETE;
    case SETZ:  return SETNZ;
    case SETNZ: return SETZ;
    case SETL:  return SETGE;
    case SETLE: return SETG;
    case SETG:  return SETLE;
    case SETGE: return SETL;
    case SETA:  return SETBE;
    case SETAE: return SETB;
    case SETB:  return SETAE;
    case SETBE: return SETA;
    default:
        assert(false && "cannot negate non-setcc opcode!");
    }
}

MachInstruction& X64InstrSelector::emit(
        x64::Opcode op, const std::vector<MachOperand>& ops, 
        bool before_terminators) {
    assert(m_insert != nullptr && "insertion block not set!");

    if (before_terminators) {
        auto& insts = m_insert->insts();

        uint32_t i = insts.size() - 1;
        while (is_terminating_opcode(static_cast<x64::Opcode>(insts.at(i).opcode())))
            --i;

        MachInstruction inst { op, ops };
        insts.insert(insts.end() - i + 1, inst);
        return insts.at(insts.size() - i);        
    } else {
        MachInstruction inst { op, ops, m_insert };
        return m_insert->back();
    }
}

bool X64InstrSelector::is_deferred(const Instruction* inst) const {
    assert(inst->is_comparison() &&
        "cannot defer a non-comparison instruction!");

    return std::find(
        m_deferred.begin(), m_deferred.end(), inst) != m_deferred.end();
}

void X64InstrSelector::defer(const Instruction* inst) {
    assert(inst->is_comparison() &&
        "cannot defer a non-comparison instruction!");

    m_deferred.push_back(inst);
}

void X64InstrSelector::select_constant(const Instruction* inst) {
    assert(inst->is_const() && "expected ConstOp!");

    MachOperand src = as_operand(inst->get_operand(0));
    emit(get_move_op(inst->get_type()), { src })
        .add_reg(as_register(inst), get_subregister(inst->get_type()), true);
}

void X64InstrSelector::select_string_constant(const Instruction* inst) {
    assert(inst->opcode() == INST_OP_STRING && "expected StringOp!");

    const auto* string = static_cast<const ConstantString*>(inst->get_operand(0));
    uint32_t pool_index = 
        m_function.get_constant_pool().get_or_create_constant(string, 1);

    emit(x64::LEA64)
        .add_constant_index(pool_index)
        .add_reg(as_register(inst), 8, true);
}

void X64InstrSelector::select_load_store(const Instruction* inst) {
    assert((inst->is_load() || inst->is_store()) &&
        "expected LoadInstr or StoreInstr opcode!");

    x64::Opcode opc = get_move_op(
        inst->is_load() ? inst->get_type() : inst->get_operand(0)->get_type());

    MachOperand src = as_operand(inst->get_operand(0));
    if (inst->is_load() && src.is_reg()) {
        // The pointer to load from is in a register, e.g. the result of a
        // pointer access, so it must be transformed into a memory reference to
        // dereference the pointer.
        src = MachOperand::create_mem(src.get_reg(), 0);

        if (src.get_mem_base().is_physical()) {
            src.set_is_use(true);

            if (dynamic_cast<const Argument*>(inst->get_operand(0)))
                src.set_is_kill(true);
        }
    }

    if (inst->is_store()) {
        if (src.is_reg() && src.get_reg().is_physical()) {
            src.set_is_use(true);

            if (dynamic_cast<const Argument*>(inst->get_operand(0)))
                src.set_is_kill(true);

        } else if (src.is_symbol() || src.is_mem() || 
            src.is_stack_index() || src.is_constant_index()) {
            // Both the store source and destination are memory references, so
            // the source must first be placed into a temporary register, we
            // choose %rax for simplicity.
            MachOperand tmp = MachOperand::create_reg(
                x64::RAX, 
                get_subregister(inst->get_operand(0)->get_type()), 
                true);

            emit(x64::LEA64, { src, tmp });

            // Now the source of the store can be considered tmp (in %rax), and
            // the next use will kill the value in it.
            src = tmp;
            src.set_is_use();
            src.set_is_kill();
        }

        MachOperand dst = as_operand(inst->get_operand(1));
        if (dst.is_reg()) {
            // The pointer to store to is in a register, e.g. the result 
            // of a pointer access, so it must be transformed into a memory 
            // reference.
            dst = MachOperand::create_mem(dst.get_reg(), 0);
            
            if (dst.get_mem_base().is_physical())
                dst.set_is_use(true);
        }

        emit(opc, { src, dst });
    } else {
        emit(opc, { src })
            .add_reg(as_register(inst), get_subregister(inst->get_type()), true);
    }
}

void X64InstrSelector::select_access_ptr(const Instruction* inst) {
    const Value* src_value = inst->get_operand(0);
    const Type* src_type = src_value->get_type();

    MachOperand src = as_operand(src_value);
    MachOperand dst = MachOperand::create_reg(as_register(inst), 8, true);
    
    assert(src_type->is_pointer_type() && "APtrOp source must be a pointer!");
    const Type* pointee = static_cast<const PointerType*>(
        src_type)->get_pointee();

    x64::Opcode opc;
    if (dynamic_cast<const Local*>(src_value)) {
        opc = x64::LEA64;
    } else {
        opc = get_move_op(src_type);
    }

    emit(opc, { src, dst });

    int64_t offset;
    if (auto constant = dynamic_cast<const ConstantInt*>(inst->get_operand(1))) {
        if (pointee->is_struct_type()) {
            offset = m_target.get_field_offset(
                static_cast<const StructType*>(pointee), 
                constant->get_value());
        } else {
            offset = m_target.get_type_size(pointee) * constant->get_value();
        }

        if (offset == 0)
            return;

        emit(x64::ADD64)
            .add_imm(offset)
            .add_operand(dst);
    } else {
        // TODO: This is mangled from an old expirementation, needs work.
        switch (pointee->get_kind()) {
        case Type::TK_Array:
            offset = m_target.get_type_size(
                static_cast<const ArrayType*>(pointee)->get_element_type());
            break;
        case Type::TK_Function:
            offset = m_target.get_type_size(
                static_cast<const PointerType*>(pointee)->get_pointee());
            break;
        default:
            offset = m_target.get_type_size(pointee);
        }

        MachOperand index = as_operand(inst->get_operand(1));
        MachOperand multiplier = MachOperand::create_imm(offset);
         
        if (offset == 1) {
            emit(x64::ADD64, { index, dst });
        } else {
            MachOperand tmp = MachOperand::create_reg(
                x64::RAX, 8, true);

            emit(x64::IMUL64)
                .add_imm(offset)
                .add_operand(index)
                .add_operand(tmp);

            tmp.set_is_use();
            tmp.set_is_kill();

            emit(x64::ADD64, { tmp, dst });
        }
    }
}

void X64InstrSelector::select_select(const Instruction* inst) {
    assert(false && "x64 selection for SelectOp not implemented!");
}

void X64InstrSelector::select_branch_if(const Instruction* inst) {
    const Value* condition = inst->get_operand(0);
    assert(condition->get_type()->is_integer_type(1) &&
        "BranchIfInstr condition type is not 'i1'!");

    const auto* instr = dynamic_cast<const Instruction*>(condition);
    if (instr && instr->is_comparison() && is_deferred(instr)) {
        x64::Opcode jcc = get_jcc_op(instr->opcode());
        MachOperand lhs = as_operand(instr->get_operand(0));
        MachOperand rhs = as_operand(instr->get_operand(1));

        if (rhs.is_imm()) {
            MachOperand tmp = lhs;
            lhs = rhs;
            rhs = tmp;
        } else {
            jcc = flip_jcc(jcc);
        }

        x64::Opcode cmp_opc = get_cmp_op(instr->get_operand(0)->get_type());
        emit(cmp_opc, { lhs, rhs });

        MachOperand tdst = as_operand(inst->get_operand(1));
        MachOperand fdst = as_operand(inst->get_operand(2));

        emit(jcc, { tdst });
        emit(x64::JMP, { fdst });
    } else {
        MachOperand cond = as_operand(condition);
        MachOperand tdst = as_operand(inst->get_operand(1));
        MachOperand fdst = as_operand(inst->get_operand(2));
        MachOperand zero = MachOperand::create_imm(0);

        // TODO: Adjust for floating point comparisons, using XORPx for zero.
        emit(x64::CMP8, { zero, cond });
        emit(x64::JNE, { tdst });
        emit(x64::JMP, { fdst });
    }
}

void X64InstrSelector::select_phi(const Instruction* inst) {
    MachRegister dst_reg = as_register(inst);
    uint32_t subreg = get_subregister(inst->get_type());

    for (uint32_t i = 0, e = inst->num_operands(); i != e; ++i) {
        // TODO: Currently, this implementation works for trivial merges since
        // it's naively inserting moves at the end of predecessors (before any
        // terminating instructions). However, when dealing with parallel 
        // copies, the implementation probably won't hold up.

        const Value* operand = inst->get_operand(i);
        const PhiOperand* phi_op = dynamic_cast<const PhiOperand*>(operand);
        assert(phi_op && "unexpected phi operand");

        const Value* incoming = phi_op->get_value();
        const BasicBlock* pred = phi_op->get_pred();

        MachBasicBlock* pred_mbb = m_function.at(pred->get_number());
        assert(pred_mbb && "could not find machine block for phi predecessor!");

        MachBasicBlock* saved_insert = m_insert;
        m_insert = pred_mbb;

        MachOperand src = as_operand(incoming);
        x64::Opcode opc = get_move_op(incoming->get_type());

        emit(opc, { src }, true)
            .add_reg(dst_reg, subreg, true);

        m_insert = saved_insert;
    }
}

void X64InstrSelector::select_return(const Instruction* inst) {
    MachRegister dst_reg = MachRegister::NoRegister;
    uint32_t sub_reg = 0;

    if (inst->num_operands() == 1) {
        const Value* return_value = inst->get_operand(0);
        if (return_value->get_type()->is_floating_point_type()) {
            dst_reg = x64::XMM0;
        } else {
            dst_reg = x64::RAX;
            sub_reg = get_subregister(return_value->get_type());
        }

        MachOperand src = as_operand(return_value);
        x64::Opcode opc = get_move_op(return_value->get_type());

        emit(opc, { src })
            .add_reg(dst_reg, sub_reg, false);
    }

    MachInstruction& instr = emit(x64::RET64);

    if (dst_reg != MachRegister::NoRegister)
        instr.add_reg(dst_reg, sub_reg, false, true);
}

void X64InstrSelector::select_call(const Instruction* inst) {
    assert(inst->num_operands() <= 6 && 
        "cannot call a function with more than 6 arguments!");
    
    const Value* first_oper = inst->get_operand(0);
    if (const auto* iasm = dynamic_cast<const InlineAsm*>(first_oper)) {
        const std::string& str = iasm->string();
        std::size_t pos = 0;

        while (pos != std::string::npos) {
            // Instructions are either divided by newlines '\n' or are the
            // substrings after the last newline. 
            std::size_t line_end = str.find_first_of('\n', pos);
            std::string line = (line_end == std::string::npos)
                ? str.substr(pos)
                : str.substr(pos, line_end - pos);

            if (line.empty()) {
                // If the line is empty (either intentionally or cause the real 
                // last line used a newline), then we can either stop now or
                // loop if there is more to the template string.
                if (line_end == std::string::npos)
                    break;

                pos = line_end + 1;
                continue;
            }

            std::size_t mnemonic_end = line.find_first_of(' ');
            std::string mnemonic_str = (mnemonic_end == std::string::npos)
                ? line.substr(0)
                : line.substr(0, mnemonic_end);
            
            x64::Opcode mnemonic = x64::parse_opcode(mnemonic_str);
            assert(mnemonic != NO_OPC && "unrecognized mnemonic!");
            MachInstruction& minst = emit(mnemonic);

            std::string post_mnemonic = (mnemonic_end == std::string::npos)
                ? ""
                : line.substr(mnemonic_end + 1);
            
            std::vector<std::string> operands = {};
            std::string curr = "";
            for (std::size_t i = 0, e = post_mnemonic.size(); i < e; ++i) {
                char c = post_mnemonic[i];
                if (c == ' ')
                    continue;
                
                if (c == ',' || c == '\n') {
                    operands.push_back(curr);
                    curr.clear();
                } else {
                    curr.push_back(c);
                }

                if (i + 1 == e)
                    operands.push_back(curr);
            }

            for (uint32_t i = 0, e = operands.size(); i < e; ++i) {
                const std::string& op = operands[i];

                switch (op[0]) {
                case '%': {
                    std::pair<x64::Register, uint16_t> reg = 
                        parse_register(op.substr(1));
                    assert(reg.first != x64::NO_REG);

                    if (i + 1 == e) {
                        minst.add_reg(reg.first, reg.second, true);
                    } else {
                        minst.add_reg(reg.first, reg.second, false);
                    }
                    
                    break;
                }

                case '$': {
                    minst.add_imm(std::stoll(op.substr(1)));
                    break;
                }

                case '#': {
                    uint32_t index = std::stoul(op.substr(1));

                    MachOperand oper = as_operand(inst->get_operand(index + 1));

                    const std::string& constraint = iasm->constraints()[index];

                    // @Todo: figure out these actual constraints, and deal 
                    // with clobbers.
                    if (constraint == "|m") {
                        //oper.set_is_def();
                    } else if (constraint == "|r") {
                        oper.set_is_def();
                    } else if (constraint == "&m") {
                        //oper.set_is_use();
                        //oper.set_is_def();
                    } else if (constraint == "&r") {
                        oper.set_is_use();
                        oper.set_is_def();
                    } else if (constraint == "m") {
                        //oper.set_is_use();
                    } else if (constraint == "r") {
                        oper.set_is_use();
                    } else if (constraint == "...") {

                    }

                    minst.add_operand(oper);
                    break;
                }

                default:
                    assert(false && "unknown inline assembly operand type!");
                }
            }

            if (line_end == std::string::npos)
                break;

            pos = line_end + 1;
        }

        return;
    }

    // @Todo: Add stack spilling for calls with more than 6 arguments.

    std::vector<MachRegister> regs = {};
    regs.reserve(inst->num_operands() - 1);

    for (int32_t idx = inst->num_operands() - 2; idx >= 0; --idx) {
        const Value* arg = inst->get_operand(idx + 1);
        MachOperand src = as_operand(arg);
        MachOperand dst = as_argument(arg, idx);
        dst.set_is_def(true);
        regs.push_back(dst.get_reg());

        x64::Opcode opc;
        if (dynamic_cast<const Local*>(arg)) {
            opc = x64::LEA64;
        } else {
            opc = get_move_op(arg->get_type());
        }

        emit(opc, { src, dst });
    }

    const Function* callee = dynamic_cast<const Function*>(first_oper);
    assert(callee && 
        "CallInstr first operand is not a function or inline assembly!");

    MachInstruction& call = emit(x64::CALL64)
        .add_symbol(callee->get_name());
    
    for (const auto& reg : regs)
        call.add_reg(reg, 8, false, true, true);

    if (inst->result_id() != 0) {
        MachRegister src_reg;
        uint32_t sub_reg = 0;
        if (inst->get_type()->is_floating_point_type()) {
            src_reg = x64::XMM0;
        } else {
            src_reg = x64::RAX;
            sub_reg = get_subregister(inst->get_type());
        }

        call.add_reg(src_reg, sub_reg, true, true);

        x64::Opcode opc = get_move_op(inst->get_type());
        emit(opc)
            .add_reg(src_reg, sub_reg, false, false, true, false)
            .add_reg(as_register(inst), sub_reg, true);
    }
}

void X64InstrSelector::select_add(const Instruction* inst) {
    MachOperand lhs = as_operand(inst->get_operand(0));
    MachOperand rhs = as_operand(inst->get_operand(1));

    if (rhs.is_imm()) {
        MachOperand tmp = lhs;
        lhs = rhs;
        rhs = tmp;
    }

    x64::Opcode add_opc = get_add_op(inst->get_type());
    MachInstruction& instr = emit(add_opc, { lhs, rhs });

    // TODO: A lot of x64 arithmetic operations implicitly kill values, but
    // whether that information is needed is unknown at this point, keeping
    // for future reference.
    // if (rhs.is_reg()) {
    //     instr.get_operand(1).set_is_kill();
    //     instr.add_reg(rhs.get_reg(), 8, true);
    // }

    x64::Opcode mov_opc = get_move_op(inst->get_type());

    emit(mov_opc, { rhs })
        .add_reg(as_register(inst), get_subregister(inst->get_type()), true);
}

void X64InstrSelector::select_sub(const Instruction* inst) {
    x64::Opcode sub_opc = get_sub_op(inst->get_type());
    x64::Opcode mov_opc = get_move_op(inst->get_type());

    MachOperand lhs = as_operand(inst->get_operand(0));
    MachOperand rhs = as_operand(inst->get_operand(1));

    if (lhs.is_imm()) {
        // Thanks to the beautiful AT&T syntax, we cannot have an immediate on
        // the right hand operand, which means for subtraction, where the order
        // matters, we should move left hand immediates to the destination
        // first so that they don't end up on the right side.
        MachOperand dst = MachOperand::create_reg(
            as_register(inst), get_subregister(inst->get_type()), true);

        emit(mov_opc, { lhs, dst });
        emit(sub_opc, { rhs, dst });
    } else {
        // Left hand side is not an immediate, so move the right operand to the
        // destination first (so that if it's an immedaite it doesn't matter,
        // which tends to be the more common case anyways).
        emit(sub_opc, { rhs, lhs });
        emit(mov_opc, { lhs })
            .add_reg(as_register(inst), get_subregister(inst->get_type()), true);
    }   
}

void X64InstrSelector::select_imul(const Instruction* inst) {
    x64::Opcode move_opc = get_move_op(inst->get_type());
    x64::Opcode imul_opc = get_imul_op(inst->get_type());

    MachOperand lhs = as_operand(inst->get_operand(0));
    MachOperand rhs = as_operand(inst->get_operand(1));
    MachOperand dst = MachOperand::create_reg(
        as_register(inst), get_subregister(inst->get_type()), true);

    if (rhs.is_imm()) {
        MachOperand tmp = lhs;
        lhs = rhs;
        rhs = tmp;
    }

    emit(move_opc, { lhs, dst });
    emit(imul_opc, { rhs, dst });
}

void X64InstrSelector::select_idiv_irem(const Instruction* inst) {
    x64::Opcode div_opc, mov_opc = get_move_op(inst->get_type());
    bool is_idiv = false, is_rem = false;

    switch (inst->opcode()) {
    case INST_OP_SREM:
        is_rem = true;
        /* Intentional fall-through */
    case INST_OP_SDIV:
        is_idiv = true;
        div_opc = get_idiv_op(inst->get_type());
        break;
    case INST_OP_UREM:
        is_rem = true;
        /* Intentional fall-through */
    case INST_OP_UDIV:
        div_opc = get_div_op(inst->get_type());
        break;
    default:
        assert(false && "unexpected opcode");
    }

    const Value* lhs_value = inst->get_operand(0);
    const Value* rhs_value = inst->get_operand(1);

    MachOperand lhs = as_operand(lhs_value);
    MachOperand rhs = as_operand(rhs_value);

    emit(get_move_op(lhs_value->get_type()), { lhs })
        .add_reg(x64::RAX, get_subregister(lhs_value->get_type()), true);

    if (is_idiv) {
        emit(x64::CQO) // cqo
            .add_reg(x64::RAX, 8, true, true) // impl-def %rax
            .add_reg(x64::RDX, 8, true, true) // impl-def %rdx
            .add_reg(x64::RAX, 8, false, true); // impl %rax

        emit(div_opc, { rhs }) // idivx ..rhs..
            .add_reg(x64::RAX, 8, true, true, false, is_rem) // impl (dead) %rax
            .add_reg(x64::RDX, 8, true, true, false, !is_rem) // impl (dead) %rdx
            .add_reg(x64::RAX, 8, false, true) // impl %rax
            .add_reg(x64::RDX, 8, false, true, true); // impl killed %rdx
    } else {
        emit(x64::MOV32) // movl $0, %edx
            .add_imm(0)
            .add_reg(x64::RDX, 4, true, false, false, true) // dead %edx
            .add_reg(x64::RDX, 8, true, true); // impl-def %rdx
            
        emit(div_opc, { rhs }) // divx ..rhs..
            .add_reg(x64::RAX, 8, true, true, false, is_rem) // (dead) %rax
            .add_reg(x64::RDX, 8, true, true, false, !is_rem) // (dead) %rdx
            .add_reg(x64::RAX, 8, false, true) // impl %rax
            .add_reg(x64::RDX, 8, false, true, true); // impl killed %rdx
    }

    MachOperand dst = MachOperand::create_reg(
        as_register(inst), get_subregister(inst->get_type()), true);

    if (is_rem) {
        // Remainders are in %rdx.
        emit(mov_opc)
            .add_reg(x64::RDX, get_subregister(inst->get_type()), false, false, true)
            .add_operand(dst);
    } else {
        // Quotients are in %rax.
        emit(mov_opc)
            .add_reg(x64::RAX, get_subregister(inst->get_type()), false, false, true)
            .add_operand(dst);
    }
}

void X64InstrSelector::select_fmul_fdiv(const Instruction* inst) {
    MachOperand lhs = as_operand(inst->get_operand(0));
    MachOperand rhs = as_operand(inst->get_operand(1));

    x64::Opcode opc;
    switch (inst->opcode()) {
    case INST_OP_FMUL:
        opc = get_mul_op(inst->get_type());
        break;

    case INST_OP_FDIV:
        opc = get_div_op(inst->get_type());
        break;

    default:
        assert(false && "unexpected opcode!");
    }

    if (lhs.is_constant_index()) {
        MachOperand tmp = MachOperand::create_reg(
            x64::XMM0, 0, true);

        emit(get_move_op(inst->get_type()), { lhs, tmp });

        lhs = tmp;
        lhs.set_is_use();
        lhs.set_is_kill();
    }
 
    emit(opc, { rhs, lhs });
    emit(get_move_op(inst->get_type()), { lhs })
        .add_reg(as_register(inst), 8, true);
}

void X64InstrSelector::select_bit_op(const Instruction* inst) {
    x64::Opcode opc;
    switch (inst->opcode()) {
    case INST_OP_AND:
        opc = get_and_op(inst->get_type());
        break;

    case INST_OP_OR:
        opc = get_or_op(inst->get_type());
        break;

    case INST_OP_XOR:
        opc = get_xor_op(inst->get_type());
        break;

    default:
        assert("expected AndInstr or OrInstr or XorInstr opcode!");
    }

    MachOperand lhs = as_operand(inst->get_operand(0));
    MachOperand rhs = as_operand(inst->get_operand(1));
    emit(opc, { lhs, rhs });

    x64::Opcode mov_opc = get_move_op(inst->get_type());
    emit(mov_opc, { rhs })
        .add_reg(as_register(inst), get_subregister(inst->get_type()), true);
}

void X64InstrSelector::select_shift(const Instruction* inst) {
    x64::Opcode op;

    switch (inst->opcode()) {
    case INST_OP_SHL:
        op = get_shl_op(inst->get_type());
        break;

    case INST_OP_SHR:
        op = get_shr_op(inst->get_type());
        break;

    case INST_OP_SAR:
        op = get_sar_op(inst->get_type());
        break;

    default:
        assert(false && "unexpected opcode");
    }

    MachOperand lhs = as_operand(inst->get_operand(0));
    MachOperand rhs = as_operand(inst->get_operand(1));
    MachOperand dst = MachOperand::create_reg(
        as_register(inst), get_subregister(inst->get_type()), true);

    emit(get_move_op(inst->get_operand(0)->get_type()), { lhs, dst });

    dst.set_is_use();

    if (rhs.is_imm()) {
        emit(op, { rhs, dst });
    } else {
        MachOperand cl = MachOperand::create_reg(
            x64::RCX, 1, true);
            
        if (rhs.is_reg())
            rhs.set_subreg(1);

        emit(x64::MOV8, { rhs, cl });
        emit(op, { cl, dst });
    }
}

void X64InstrSelector::select_not(const Instruction* inst) {
    MachOperand src = as_operand(inst->get_operand(0));
    
    emit(get_not_op(inst->get_operand(0)->get_type()), { src });
    emit(get_move_op(inst->get_type()), { src })
        .add_reg(as_register(inst), get_subregister(inst->get_type()), true);
}

void X64InstrSelector::select_neg(const Instruction* inst) {
    MachOperand src = as_operand(inst->get_operand(0));

    if (inst->opcode() == INST_OP_INEG) {
        x64::Opcode opc = get_neg_op(inst->get_type());
        emit(opc, { src });

        x64::Opcode mov_opc = get_move_op(inst->get_type());
        emit(mov_opc, { src })
            .add_reg(as_register(inst), get_subregister(inst->get_type()), true);
    } else if (inst->opcode() == INST_OP_FNEG) {
        // TODO: Implement FNegOp selection, also needs mask constants.
    } else {
        assert(false && "expected INegInstr or FNegInstr opcode!");
    }
}

void X64InstrSelector::select_ext(const Instruction* inst) {
    const Value* value = inst->get_operand(0);
    MachOperand src = as_operand(value);
    uint32_t src_sz_in_bits = m_target.get_type_size_in_bits(value->get_type());
    uint32_t dst_sz_in_bits = m_target.get_type_size_in_bits(inst->get_type());
    uint32_t dst_subreg = get_subregister(inst->get_type());
    x64::Opcode opc;

    switch (inst->opcode()) {
    case INST_OP_SEXT:
        if (src_sz_in_bits == 32 && dst_sz_in_bits == 64) {
            opc = x64::MOVSXD;
        } else {
            opc = x64::MOVSX;
        }

        break;

    case INST_OP_ZEXT:
        if (src_sz_in_bits == 32 && dst_sz_in_bits == 64) {
            opc = x64::MOV;
            dst_subreg = 4;
        } else {
            opc = x64::MOVZX;
        }

        break;
        
    case INST_OP_FEXT:
        opc = x64::CVTSS2SD;
        break;

    default:
        assert(false && 
            "expected SExtOp, ZExtOp or FExtOp!");
    }

    emit(opc, { src })
        .add_reg(as_register(inst), dst_subreg, true);
}

void X64InstrSelector::select_trunc(const Instruction* inst) {
    MachOperand src = as_operand(inst->get_operand(0));
    uint32_t dst_subreg = get_subregister(inst->get_type());
    x64::Opcode opc;

    switch (inst->opcode()) {
    case INST_OP_ITRUNC:
        if (src.is_reg())
            src.set_subreg(dst_subreg);
        
        opc = x64::MOV;
        break;

    case INST_OP_FTRUNC:
        opc = x64::CVTSD2SS;
        break;

    default:
        assert(false && 
            "expected ITruncOp or FTruncOp!");
    }

    emit(opc, { src })
        .add_reg(as_register(inst), dst_subreg, true);
}

void X64InstrSelector::select_int_to_fp_cvt(const Instruction* inst) {
    assert((inst->opcode() == INST_OP_SI2FP || inst->opcode() == INST_OP_UI2FP) 
        && "expected SI2FPOp or UI2FPOp!");

    MachOperand src = as_operand(inst->get_operand(0));
    const Type* dst_type = inst->get_type();
    x64::Opcode opc;

    if (dst_type->is_floating_point_type(32)) {
        opc = x64::CVTSI2SS;
    } else if (dst_type->is_floating_point_type(64)) {
        opc = x64::CVTSI2SD;
    } else {
        assert(false && "invalid integer to fp conversion destination type!");
    }

    emit(opc, { src })
        .add_reg(as_register(inst), get_subregister(inst->get_type()), true);
}

void X64InstrSelector::select_fp_to_int_cvt(const Instruction* inst) {
    assert((inst->opcode() == INST_OP_FP2SI || inst->opcode() == INST_OP_FP2UI) 
        && "expected FP2SIOp or FP2UIOp!");

    const Value* operand = inst->get_operand(0);
    const Type* src_type = operand->get_type();
    uint32_t dst_size = m_target.get_type_size_in_bits(inst->get_type());
    MachOperand src = as_operand(operand);
    x64::Opcode opc;

    if (src_type->is_floating_point_type(32)) {
        switch (dst_size) {
        case 8:
            opc = x64::CVTTSS2SI8;
            break;

        case 16:
            opc = x64::CVTTSS2SI16;
            break;

        case 32:
            opc = x64::CVTTSS2SI32;
            break;

        case 64:
            opc = x64::CVTTSS2SI64;
            break;
        }
    } else if (src_type->is_floating_point_type(64)) {
        switch (dst_size) {
        case 8:
            opc = x64::CVTTSD2SI8;
            break;

        case 16:
            opc = x64::CVTTSD2SI16;
            break;

        case 32:
            opc = x64::CVTTSD2SI32;
            break;

        case 64:
            opc = x64::CVTTSD2SI64;
            break;
        }
    } else {
        assert(false && "invalid fp to integer conversion destination type!");
    }

    emit(opc, { src })
        .add_reg(as_register(inst), get_subregister(inst->get_type()), true);
}

void X64InstrSelector::select_ptr_to_int_cvt(const Instruction* inst) {
    const Value* src = inst->get_operand(0);

    x64::Opcode opc;
    if (dynamic_cast<const Local*>(src)) {
        opc = x64::LEA64;
    } else {
        opc = get_move_op(src->get_type());
    }

    emit(opc)
        .add_operand(as_operand(src))
        .add_reg(as_register(inst), get_subregister(inst->get_type()), true);
}

void X64InstrSelector::select_int_to_ptr_cvt(const Instruction* inst) {
    emit(get_move_op(inst->get_type()))
        .add_operand(as_operand(inst->get_operand(0)))
        .add_reg(as_register(inst), get_subregister(inst->get_type()), true);
}

void X64InstrSelector::select_type_reinterpret(const Instruction* inst) {
    const Value* src = inst->get_operand(0);

    x64::Opcode opc;
    if (dynamic_cast<const Local*>(src)) {
        opc = x64::LEA64;
    } else {
        opc = get_move_op(src->get_type());
    }

    emit(opc)
        .add_operand(as_operand(src))
        .add_reg(as_register(inst), get_subregister(inst->get_type()), true);
}

void X64InstrSelector::select_comparison(const Instruction* inst) {
    if (inst->num_uses() == 1) {
        // If the only user of this comparison is a conditional branch, then
        // we defer this instruction until later (at the location of the
        // branch) so that we can skip a set and subsequent comparison.
        const User* user = inst->use_front()->get_user();
        const auto* instr = dynamic_cast<const Instruction*>(user);
        if (instr && instr->is_branch_if()) {
            defer(inst);
            return;
        }
    }

    x64::Opcode setcc = get_setcc_op(inst->opcode());
    x64::Opcode cmp_opc = get_cmp_op(inst->get_operand(0)->get_type());

    MachOperand lhs = as_operand(inst->get_operand(0));
    MachOperand rhs = as_operand(inst->get_operand(1));

    if (rhs.is_imm()) {
        MachOperand tmp = lhs;
        lhs = rhs;
        rhs = tmp;
    } else {
        setcc = flip_setcc(setcc);
    }

    emit(cmp_opc, { lhs, rhs });
    emit(setcc).add_reg(as_register(inst), 1, true);
}

void X64InstrSelector::run() {
    const Function* fn = m_function.get_function();
    auto& stack = m_function.get_stack_info();
    uint32_t stack_index = 0, stack_off = 0;

    // For each local in the function, reserve some space on the stack for its
    // allocated type.
    for (const auto& [name, local] : fn->locals()) {
        const Type* alloc_type = local->get_allocated_type();

        FunctionStackEntry entry = {};
        entry.offset = stack_off;

        uint32_t size = m_target.get_type_size(alloc_type);
        stack_off += size;

        entry.size = size;
        entry.align = m_target.get_type_align(alloc_type);
        entry.local = local;

        stack.entries.push_back(entry);
        m_locals.emplace(local, stack_index++);
    }

    for (auto* curr = m_function.front(); curr; curr = curr->next()) {
        const auto* bb = curr->get_basic_block();
        m_insert = curr;

        for (const auto* inst = bb->front(); inst; inst = inst->next()) {
            switch (inst->opcode()) {
            case INST_OP_NOP:
                emit(x64::NOP);
                break;

            case INST_OP_JUMP:
                emit(x64::JMP, { as_operand(inst->get_operand(0)) });
                break;

            case INST_OP_ABORT:
                emit(x64::UD2);
                break;

            case INST_OP_UNREACHABLE:
                break;

            case INST_OP_CONSTANT:
                select_constant(inst);
                break;

            case INST_OP_STRING:
                select_string_constant(inst);
                break;

            case INST_OP_LOAD:
            case INST_OP_STORE:
                select_load_store(inst);
                break;

            case INST_OP_ACCESS_PTR:
                select_access_ptr(inst);
                break;

            case INST_OP_SELECT:
                select_select(inst);
                break;

            case INST_OP_BRANCH_IF:
                select_branch_if(inst);
                break;

            case INST_OP_PHI:
                select_phi(inst);
                break;

            case INST_OP_RETURN:
                select_return(inst);
                break;

            case INST_OP_CALL:
                select_call(inst);
                break;

            case INST_OP_IADD:
            case INST_OP_FADD:
                select_add(inst);
                break;

            case INST_OP_ISUB:
            case INST_OP_FSUB:
                select_sub(inst);
                break;

            case INST_OP_SMUL:
            case INST_OP_UMUL:
                select_imul(inst);
                break;
                
            case INST_OP_SDIV:
            case INST_OP_UDIV:
            case INST_OP_SREM:
            case INST_OP_UREM:
                select_idiv_irem(inst);
                break;

            case INST_OP_FMUL:
            case INST_OP_FDIV:
                select_fmul_fdiv(inst);
                break;

            case INST_OP_AND:
            case INST_OP_OR:
            case INST_OP_XOR:
                select_bit_op(inst);
                break;

            case INST_OP_SHL:
            case INST_OP_SHR:
            case INST_OP_SAR:
                select_shift(inst);
                break;

            case INST_OP_NOT:
                select_not(inst);
                break;

            case INST_OP_INEG:
            case INST_OP_FNEG:
                select_neg(inst);
                break;

            case INST_OP_SEXT:
            case INST_OP_ZEXT:
            case INST_OP_FEXT:
                select_ext(inst);
                break;

            case INST_OP_ITRUNC:
            case INST_OP_FTRUNC:
                select_trunc(inst);
                break;

            case INST_OP_SI2FP:
            case INST_OP_UI2FP:
                select_int_to_fp_cvt(inst);
                break;

            case INST_OP_FP2SI:
            case INST_OP_FP2UI:
                select_fp_to_int_cvt(inst);
                break;

            case INST_OP_P2I:
                select_ptr_to_int_cvt(inst);
                break;

            case INST_OP_I2P:
                select_int_to_ptr_cvt(inst);
                break;

            case INST_OP_REINTERPET:
                select_type_reinterpret(inst);
                break;

            case INST_OP_CMP_IEQ:
            case INST_OP_CMP_INE:
            case INST_OP_CMP_OEQ:
            case INST_OP_CMP_ONE:
            case INST_OP_CMP_UNEQ:
            case INST_OP_CMP_UNNE:
            case INST_OP_CMP_SLT:
            case INST_OP_CMP_SLE:
            case INST_OP_CMP_SGT:
            case INST_OP_CMP_SGE:
            case INST_OP_CMP_ULT:
            case INST_OP_CMP_ULE:
            case INST_OP_CMP_UGT:
            case INST_OP_CMP_UGE:
            case INST_OP_CMP_OLT:
            case INST_OP_CMP_OLE:
            case INST_OP_CMP_OGT:
            case INST_OP_CMP_OGE:
            case INST_OP_CMP_UNLT:
            case INST_OP_CMP_UNLE:
            case INST_OP_CMP_UNGT:
            case INST_OP_CMP_UNGE:
                select_comparison(inst);
                break;
            }
        }
    }
}

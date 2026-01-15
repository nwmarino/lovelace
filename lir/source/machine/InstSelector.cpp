//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lir/graph/BasicBlock.hpp"
#include "lir/graph/Constant.hpp"
#include "lir/graph/Function.hpp"
#include "lir/graph/Global.hpp"
#include "lir/graph/Instruction.hpp"
#include "lir/graph/Local.hpp"
#include "lir/graph/Type.hpp"
#include "lir/machine/MachFunction.hpp"
#include "lir/machine/MachInst.hpp"
#include "lir/machine/MachLabel.hpp"
#include "lir/machine/MachOperand.hpp"
#include "lir/machine/Register.hpp"
#include "lir/machine/InstSelector.hpp"

#include <string>
#include <sstream>

using namespace lir;

/// Test if the given |op| is a terminator.
static bool is_terminator(X64_Mnemonic op) {
    switch (op) {
        case X64_Mnemonic::JMP:
        case X64_Mnemonic::UD2:
        case X64_Mnemonic::RET:
        case X64_Mnemonic::JE:
        case X64_Mnemonic::JNE:
        case X64_Mnemonic::JZ:
        case X64_Mnemonic::JNZ:
        case X64_Mnemonic::JL:
        case X64_Mnemonic::JLE:
        case X64_Mnemonic::JG:
        case X64_Mnemonic::JGE:
        case X64_Mnemonic::JA:
        case X64_Mnemonic::JAE:
        case X64_Mnemonic::JB:
        case X64_Mnemonic::JBE:
            return true;
        default:
            return false;
    }
}

/// Convert the given comparison |predicate| to an X64 JCC instruction.
static X64_Mnemonic to_jcc(CMPPredicate predicate) {
    switch (predicate) {
        case CMP_IEQ:
            return X64_Mnemonic::JE;
        case CMP_INE:
            return X64_Mnemonic::JNE;
        case CMP_OEQ:
            return X64_Mnemonic::JE;
        case CMP_ONE:
            return X64_Mnemonic::JNE;
        case CMP_SLT:
            return X64_Mnemonic::JL;
        case CMP_SLE:
            return X64_Mnemonic::JLE;
        case CMP_SGT:
            return X64_Mnemonic::JG;
        case CMP_SGE:
            return X64_Mnemonic::JGE;
        case CMP_ULT:
        case CMP_OLT:
            return X64_Mnemonic::JB;
        case CMP_ULE:
        case CMP_OLE:
            return X64_Mnemonic::JBE;
        case CMP_UGT:
        case CMP_OGT:
            return X64_Mnemonic::JA;
        case CMP_UGE:
        case CMP_OGE:
            return X64_Mnemonic::JAE;
    }
}

/// Convert the given comparison |predicate| to an X64 SETCC instruction.
static X64_Mnemonic to_setcc(CMPPredicate predicate) {
    switch (predicate) {
        case CMP_IEQ:
            return X64_Mnemonic::SETE;
        case CMP_INE:
            return X64_Mnemonic::SETNE;
        case CMP_OEQ:
            return X64_Mnemonic::SETE;
        case CMP_ONE:
            return X64_Mnemonic::SETNE;
        case CMP_SLT:
            return X64_Mnemonic::SETL;
        case CMP_SLE:
            return X64_Mnemonic::SETLE;
        case CMP_SGT:
            return X64_Mnemonic::SETG;
        case CMP_SGE:
            return X64_Mnemonic::SETGE;
        case CMP_ULT:
        case CMP_OLT:
            return X64_Mnemonic::SETB;
        case CMP_ULE:
        case CMP_OLE:
            return X64_Mnemonic::SETBE;
        case CMP_UGT:
        case CMP_OGT:
            return X64_Mnemonic::SETA;
        case CMP_UGE:
        case CMP_OGE:
            return X64_Mnemonic::SETAE;
    }
}

static X64_Mnemonic flip_jcc(X64_Mnemonic jcc) {
    switch (jcc) {
        case X64_Mnemonic::JE:
        case X64_Mnemonic::JNE:
        case X64_Mnemonic::JZ:
        case X64_Mnemonic::JNZ:
            return jcc;
        case X64_Mnemonic::JL:
            return X64_Mnemonic::JG;
        case X64_Mnemonic::JLE:
            return X64_Mnemonic::JGE;
        case X64_Mnemonic::JG:
            return X64_Mnemonic::JL;
        case X64_Mnemonic::JGE:
            return X64_Mnemonic::JLE;
        case X64_Mnemonic::JA:
            return X64_Mnemonic::JB;
        case X64_Mnemonic::JAE:
            return X64_Mnemonic::JBE;
        case X64_Mnemonic::JB:
            return X64_Mnemonic::JA;
        case X64_Mnemonic::JBE:
            return X64_Mnemonic::JAE;
        default:
            assert(false && "cannot flip non-jcc opcode!");
    }
}

static X64_Mnemonic flip_setcc(X64_Mnemonic setcc) {
    switch (setcc) {
        case X64_Mnemonic::SETE:
        case X64_Mnemonic::SETNE:
        case X64_Mnemonic::SETZ:
        case X64_Mnemonic::SETNZ:
            return setcc;
        case X64_Mnemonic::SETL:
            return X64_Mnemonic::SETG;
        case X64_Mnemonic::SETLE:
            return X64_Mnemonic::SETGE;
        case X64_Mnemonic::SETG:
            return X64_Mnemonic::SETL;
        case X64_Mnemonic::SETGE:
            return X64_Mnemonic::SETLE;
        case X64_Mnemonic::SETA:
            return X64_Mnemonic::SETB;
        case X64_Mnemonic::SETAE:
            return X64_Mnemonic::SETBE;
        case X64_Mnemonic::SETB:
            return X64_Mnemonic::SETA;
        case X64_Mnemonic::SETBE:
            return X64_Mnemonic::SETAE;
        default:
            assert(false && "cannot flip non-setcc opcode!");
    }
}

/*
x64::Opcode InstSelector::negate_jcc(x64::Opcode jcc) const {
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

x64::Opcode InstSelector::negate_setcc(x64::Opcode setcc) const {
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
*/

InstSelector::InstSelector(MachFunction& func) 
  : m_func(func), m_mach(func.get_machine()) {}

X64_Size InstSelector::as_size(Type* type) const {
    assert(type && "type cannot be null!");

    const uint32_t bytes = m_mach.get_size(type);
    assert(bytes <= 8 && "type must be scalar!");

    if (type->is_float_type()) switch (bytes) {
        case 4:
            return X64_Size::Single;
        case 8:
            return X64_Size::Double;
        default:
            return X64_Size::None;
    } else switch (bytes) {
        case 1:
            return X64_Size::Byte;
        case 2:
            return X64_Size::Word;
        case 4:
            return X64_Size::Long;
        case 8:
            return X64_Size::Quad;
        default:
            return X64_Size::None;
    }
}

uint16_t InstSelector::get_subregister(Type* type) const {
    assert(type && "type cannot be null!");
    
    switch (m_mach.get_size(type)) {
        case 1:
            return 1;
        case 2:
            return 2;
        case 4:
            return 4;
        case 8:
            return 8;
        default:
            return 0;
    }
}

Register InstSelector::as_register(const Instruction* inst) {
    assert(inst->is_def() && "instruction does not produce a value!");

    Register vreg = get_temporary(inst->get_type()->is_float_type()
        ? FloatingPoint
        : GeneralPurpose);

    // Track the register mapping so that references to |inst| can reuse the 
    // same register later.
    m_regs.emplace(inst->get_def(), vreg);
    return vreg;
}

Register InstSelector::get_temporary(RegisterClass cls) {
    VirtualRegister vreg = {};
    vreg.cls = cls;
    vreg.alloc = Register::NoRegister;

    MachFunction::RegisterTable& table = m_func.get_register_table();
    const uint32_t id = Register::VirtualBarrier + table.size();
    table.emplace(id, vreg);
    return id;
}

MachOperand InstSelector::as_operand(const Value* value) {
    if (const Integer* integer = dynamic_cast<const Integer*>(value)) 
    {
        return MachOperand::create_imm(integer->get_value());
    } 
    else if (const Float* fp = dynamic_cast<const Float*>(value)) 
    {
        MachOperand reg = MachOperand::create_reg(
            get_temporary(FloatingPoint), 0, true);

        uint32_t const_index = m_func.get_constant_pool().get_or_create_constant(
            fp, m_mach.get_align(value->get_type()));

        emit(X64_Mnemonic::MOVS, as_size(value->get_type()))
            .add_constant(const_index)
            .add_operand(reg)
            .add_comment("use fp (" + std::to_string(fp->get_value()) + ")");

        reg.set_is_use();
        return reg;
    } 
    else if (const Null* null = dynamic_cast<const Null*>(value)) 
    {
        return MachOperand::create_imm(0);
    } 
    else if (const BlockAddress* block = dynamic_cast<const BlockAddress*>(value)) 
    {
        // @Todo: change to adjust for position changes in the blocks of
        // machine functions. Specifically, if blocks change positions or
        // numbering during lowering, this breaks.
        return MachOperand::create_label(
            m_func.at(block->get_block()->position()));
    } 
    else if (auto global = dynamic_cast<const Global*>(value)) 
    {
        return MachOperand::create_symbol(global->get_name().c_str());
    } 
    else if (const Function::Arg* arg = dynamic_cast<const Function::Arg*>(value)) 
    {
        return as_argument(value, arg->get_index());
    } 
    else if (const BasicBlock::Arg* arg = dynamic_cast<const BasicBlock::Arg*>(value)) 
    {
        auto it = m_args.find(arg);
        if (it != m_args.end())
            return MachOperand::create_reg(it->second, get_subregister(arg->get_type()), true);

        Register vreg = get_temporary(arg->get_type()->is_float_type()
            ? FloatingPoint
            : GeneralPurpose);

        m_args.emplace(arg, vreg);
        return MachOperand::create_reg(vreg, get_subregister(arg->get_type()), false);
    } 
    else if (const Function* func = dynamic_cast<const Function*>(value)) 
    {
        return MachOperand::create_symbol(func->get_name().c_str());
    } 
    else if (const Local* local = dynamic_cast<const Local*>(value)) 
    {
        return MachOperand::create_stack_ref(m_locals.at(local));
    } 
    else if (const Instruction* inst = dynamic_cast<const Instruction*>(value)) 
    {
        assert(m_regs.count(inst->get_def()) != 0 && "instruction unmapped!");

        return MachOperand::create_reg(
            m_regs[inst->get_def()], get_subregister(value->get_type()), false);
    }

    assert(false && "cannot lower value to an x64 machine operand!");
}

MachOperand InstSelector::as_argument(const Value* value, uint32_t index) const {
    if (index >= 6) {
        // @Todo: implement spilling.
        assert(false && "cannot spill call arguments (yet)!");
    }

    Register reg;

    // @Todo: determine argument registers differently based on machine ABI.
    if (value->get_type()->is_float_type()) {
        static const Register table[6] = { XMM0, XMM1, XMM2, XMM3, XMM4, XMM5 };
        reg = table[index];
    } else {
        static const Register table[6] = { RDI, RSI, RDX, RCX, R8, R9 };
        reg = table[index];
    }

    return MachOperand::create_reg(
        reg, get_subregister(value->get_type()), true);
}

std::string InstSelector::get_asm_comment(const Instruction* inst) const {
    std::stringstream ss;
    inst->print(ss, PrintPolicy::Def);
    return ss.str();
}

MachInst& InstSelector::emit(X64_Mnemonic op, X64_Size size, 
                             const MachInst::Operands& ops, bool before_terms) {
    assert(m_insert && "no insertion point set!");

    if (before_terms) {
        std::vector<MachInst>& insts = m_insert->insts();

        uint32_t pos = insts.size() - 1;
        while (is_terminator((insts.at(pos--).op())));

        MachInst inst = MachInst(op, size, ops);
        insts.insert(insts.end() - pos + 1, inst);
        return insts.at(insts.size() - pos);        
    } else {
        MachInst inst = MachInst(op, size, ops, m_insert);
        return m_insert->back();
    }
}

void InstSelector::select(const Instruction* inst) {
    switch (inst->op()) {
        case OP_ABORT:
            return select_abort(inst);
        case OP_F2S:
        case OP_F2U:
            return select_cast_f2i(inst);
        case OP_UNREACHABLE:
            return select_unreachable(inst);
        case OP_LOAD:
        case OP_STORE:
            return select_load_store(inst);
        case OP_PWALK:
            return select_pwalk(inst);
        case OP_STRING:
            return select_string(inst);
        case OP_CALL:
            return select_call(inst);
        case OP_CMP:
            return select_comparison(inst);
        case OP_JIF:
            return select_conditional_jump(inst);
        case OP_JMP:
            return select_jump(inst);
        case OP_RET:
            return select_return(inst);
        case OP_IADD:
            return select_iadd(inst);
        case OP_FADD:
            return select_fadd(inst);
        case OP_ISUB:
            return select_isub(inst);
        case OP_FSUB:
            return select_fsub(inst);
        case OP_IMUL:
            return select_imul(inst);
        case OP_SDIV:
        case OP_UDIV:
        case OP_SMOD:
        case OP_UMOD:
            return select_division(inst);
        case OP_FMUL:
        case OP_FDIV:
            return select_float_mul_div(inst);
        case OP_AND:
        case OP_OR:
        case OP_XOR:
            return select_logic(inst);
        case OP_SHL:
        case OP_SHR:
        case OP_SAR:
            return select_shift(inst);
        case OP_NOT:
            return select_not(inst);
        case OP_INEG:
        case OP_FNEG:
            return select_negate(inst);
        case OP_S2F:
        case OP_U2F:
            return select_cast_i2f(inst);
        case OP_SEXT:
        case OP_ZEXT:
        case OP_FEXT:
            return select_extension(inst);
        case OP_ITRUNC:
        case OP_FTRUNC:
            return select_truncation(inst);
        case OP_I2P:
            return select_cast_i2p(inst);
        case OP_P2I:
            return select_cast_p2i(inst);
        case OP_REINT:
            return select_cast_reinterpret(inst);
        default:
            assert(false && "unknown mnemonic!");
    }
}

void InstSelector::select_abort(const Instruction* inst) {
    emit(X64_Mnemonic::UD2)
        .add_comment("abort");
}

void InstSelector::select_unreachable(const Instruction* inst) {
    // @Todo: could do more here.
    emit(X64_Mnemonic::UD2)
        .add_comment("unreachable");
}

void InstSelector::select_load_store(const Instruction* inst) {
    assert((inst->op() == OP_LOAD || inst->op() == OP_STORE) &&
        "expected OpLoad or OpStore!");

    Type* type = nullptr;
    if (inst->op() == OP_LOAD) {
        type = inst->get_type();
    } else {
        type = inst->get_operand(0)->get_type();
    }

    bool emitted_comment = false;
    MachOperand source = as_operand(inst->get_operand(0));
    if (inst->op() == OP_LOAD && source.is_reg()) {
        // The pointer to load from is in a register, e.g. the result of a
        // pointer access, so it must be transformed into a memory reference to
        // dereference the pointer.
        source = MachOperand::create_mem(source.get_reg(), 0);

        if (source.get_mem_base().is_physical()) {
            source.set_is_use(true);

            // @Todo: what is the point of this...
            if (dynamic_cast<const Function::Arg*>(inst->get_operand(0)))
                source.set_is_kill(true);
        }
    }

    X64_Mnemonic op = X64_Mnemonic::MOV;
    if (type->is_float_type())
        op = X64_Mnemonic::MOVS;

    if (inst->op() == OP_STORE) {
        if (source.is_reg() && source.get_reg().is_physical()) {
            source.set_is_use(true);

            // @Todo: what is the point of this...
            if (dynamic_cast<const Function::Arg*>(inst->get_operand(0)))
                source.set_is_kill(true);
        } else if (source.is_symbol() || source.is_mem() || 
            source.is_stack() || source.is_constant()) {
            // Both the store source and destination are memory references, so
            // the source must first be placed into a temporary register.
            // Choose %rax for simplicity.
            MachOperand tmp = MachOperand::create_reg(
                RAX, get_subregister(type), true);

            emit(X64_Mnemonic::LEA, X64_Size::Quad, { source, tmp })
                .add_comment(get_asm_comment(inst));

            emitted_comment = true;

            // Now the source of the store can be considered tmp i.e. in %rax,
            // and the next use will kill the value in it.
            source = tmp;
            source.set_is_use();
            source.set_is_kill();
        }

        MachOperand dest = as_operand(inst->get_operand(1));
        if (dest.is_reg()) {
            // The pointer to store to is in a register, e.g. the result 
            // of a pointer access, so it must be transformed into a memory 
            // reference.
            dest = MachOperand::create_mem(dest.get_reg(), 0);
            
            if (dest.get_mem_base().is_physical())
                dest.set_is_use(true);
        }

        MachInst& move = emit(op, as_size(type), { source, dest });
        
        if (!emitted_comment)
            move.add_comment(get_asm_comment(inst));
    } else {
        emit(op, as_size(type), { source })
            .add_reg(as_register(inst), get_subregister(inst->get_type()), true)
            .add_comment(get_asm_comment(inst));
    }
}

void InstSelector::select_pwalk(const Instruction* inst) {
    assert(inst->num_operands() >= 2 && 
        "pwalk expects a base and at least 1 index!");

    const Value* base = inst->get_operand(0);
    assert(base->get_type()->is_pointer_type());

    // Move the base pointer to the destination immediately.
    MachOperand source = as_operand(base);
    MachOperand dest = MachOperand::create_reg(as_register(inst), 8, true);
    emit(source.is_reg() ? X64_Mnemonic::MOV : X64_Mnemonic::LEA, X64_Size::Quad)
        .add_operand(source)
        .add_operand(dest)
        .add_comment(get_asm_comment(inst));

    struct DynamicIndex {
        const Value* index;
        int64_t scale;
    };

    int64_t offset = 0;
    std::vector<DynamicIndex> dynamics = {};
    const Type* curr_type = static_cast<const PointerType*>(
        base->get_type());

    for (uint32_t i = 1, e = inst->num_operands(); i < e; ++i) {
        const Value* index = inst->get_operand(i);
        auto integer = dynamic_cast<const Integer*>(index);
    
        if (curr_type->is_array_type()) {
            auto array = static_cast<const ArrayType*>(curr_type);
            int64_t scale = m_mach.get_size(array->get_element_type());

            if (integer) {
                offset += scale * integer->get_value();
            } else {
                dynamics.push_back({ index, scale });
            }

            curr_type = array->get_element_type();
        } else if (curr_type->is_pointer_type()) {
            auto ptr = static_cast<const PointerType*>(curr_type);
            int64_t scale = m_mach.get_size(ptr->get_pointee());

            if (integer) {
                offset += scale * integer->get_value();
            } else {
                dynamics.push_back({ index, scale });
            }

            curr_type = ptr->get_pointee();
        } else if (curr_type->is_struct_type()) {
            assert(integer);
            
            auto structure = static_cast<const StructType*>(curr_type);
            offset += m_mach.get_field_offset(structure, integer->get_value());
            curr_type = structure->get_field(integer->get_value());
        } else {
            assert(false && "invalid PWALK step type!");
        }
    }

    if (dynamics.empty() && offset != 0) {
        emit(X64_Mnemonic::ADD, X64_Size::Quad, { 
            MachOperand::create_imm(offset), 
            dest 
        });
    } else {
        for (const auto& dyn : dynamics) {
            MachOperand index = as_operand(dyn.index);

            if (dyn.scale != 1)
                emit(X64_Mnemonic::IMUL, X64_Size::Quad)
                    .add_imm(dyn.scale)
                    .add_operand(index);
            
            emit(X64_Mnemonic::ADD, X64_Size::Quad, { index, dest });
        }

        if (offset != 0)
            emit(X64_Mnemonic::ADD, X64_Size::Quad)
                .add_imm(offset)
                .add_operand(dest);
    }
}

void InstSelector::select_string(const Instruction* inst) {
    assert(inst->op() == OP_STRING && "expected StringOp!");

    const String* string = static_cast<const String*>(inst->get_operand(0));
    uint32_t pool_index = 
        m_func.get_constant_pool().get_or_create_constant(string, 1);

    emit(X64_Mnemonic::LEA, X64_Size::Quad)
        .add_constant(pool_index)
        .add_reg(as_register(inst), 8, true)
        .add_comment(get_asm_comment(inst));
}

void InstSelector::select_comparison(const Instruction* inst) {
    MachOperand lhs = as_operand(inst->get_operand(0));
    MachOperand rhs = as_operand(inst->get_operand(1));
    X64_Mnemonic setcc = to_setcc(inst->desc().cmp);
    
    if (rhs.is_imm()) {
        const MachOperand tmp = lhs;
        lhs = rhs;
        rhs = tmp;
    } else {
        setcc = flip_setcc(setcc);
    }

    emit(X64_Mnemonic::CMP, as_size(inst->get_operand(0)->get_type()), { lhs, rhs })
        .add_comment(get_asm_comment(inst));

    emit(setcc, X64_Size::Byte)
        .add_reg(as_register(inst), 1, true);
}

void InstSelector::select_conditional_jump(const Instruction* inst) {
    assert(inst->get_operand(0)->get_type()->is_integer_type(1) &&
        "invalid OpJif condition type!");

    const uint32_t num_operands = inst->num_operands();
    uint32_t true_dest = 0;
    uint32_t false_dest = 0;

    for (uint32_t i = 1; i < num_operands; ++i) {
        const Value* op = inst->get_operand(i);
        const BlockAddress* addr = dynamic_cast<const BlockAddress*>(op);
        if (!addr)
            continue;

        if (true_dest == 0) {
            true_dest = i;
        } else {
            false_dest = i;
            break;
        }
    }

    assert(true_dest && false_dest);

    MachOperand cond = as_operand(inst->get_operand(0));
    assert(cond.is_reg());
    cond.set_subreg(1);

    emit(X64_Mnemonic::CMP, X64_Size::Byte, { MachOperand::create_imm(0), cond })
        .add_comment(get_asm_comment(inst));

    if (true_dest + 1 != false_dest) {
        // There are operands i.e. block arguments between the true and false
        // destinations.

        const BlockAddress* true_addr = dynamic_cast<const BlockAddress*>(
            inst->get_operand(true_dest));
        assert(true_addr);

        for (uint32_t i = true_dest + 1; i < false_dest; ++i) {
            const Value* arg = inst->get_operand(i);
            X64_Mnemonic move;
            if (arg->get_type()->is_float_type()) {
                move = X64_Mnemonic::MOVS;
            } else {
                move = X64_Mnemonic::MOV;
            }

            emit(move, as_size(arg->get_type()), { 
                as_operand(arg), 
                as_operand(true_addr->get_block()->get_arg(i - true_dest - 1)) 
            });
        }
    }
    
    emit(X64_Mnemonic::JNE, X64_Size::None, { 
        as_operand(inst->get_operand(true_dest)) 
    });

    if (false_dest + 1 != num_operands) {
        const BlockAddress* false_addr = dynamic_cast<const BlockAddress*>(
            inst->get_operand(false_dest));
        assert(false_addr);

        for (uint32_t i = false_dest + 1; i < num_operands; ++i) {
            const Value* arg = inst->get_operand(i);
            X64_Mnemonic move;
            if (arg->get_type()->is_float_type()) {
                move = X64_Mnemonic::MOVS;
            } else {
                move = X64_Mnemonic::MOV;
            }

            emit(move, as_size(arg->get_type()), { 
                as_operand(arg), 
                as_operand(false_addr->get_block()->get_arg(i - false_dest - 1)) 
            });
        }
    }

    emit(X64_Mnemonic::JMP, X64_Size::None, { 
        as_operand(inst->get_operand(false_dest)) 
    });
}

void InstSelector::select_jump(const Instruction* inst) {
    const BlockAddress* label = dynamic_cast<const BlockAddress*>(
        inst->get_operand(0));
    assert(label);

    const BasicBlock* dest = label->get_block();
    assert(dest);

    bool emitted_comment = false;
    if (inst->num_operands() > 1) {
        assert(dest->num_args() == inst->num_operands() - 1);

        for (uint32_t i = 1, e = inst->num_operands(); i < e; ++i) {
            const Value* arg = inst->get_operand(i);
            X64_Mnemonic op;
            if (arg->get_type()->is_float_type()) {
                op = X64_Mnemonic::MOVS;
            } else {
                op = X64_Mnemonic::MOV;
            }

            MachInst& move = emit(op, as_size(arg->get_type()), { 
                as_operand(arg), 
                as_operand(dest->get_arg(i - 1)) 
            });

            if (!emitted_comment) {
                move.add_comment(get_asm_comment(inst));
                emitted_comment = true;
            }
        }
    }

    MachInst& jump = emit(X64_Mnemonic::JMP, X64_Size::None, { 
        as_operand(inst->get_operand(0) )
    });

    if (!emitted_comment)
        jump.add_comment(get_asm_comment(inst));
}

void InstSelector::select_return(const Instruction* inst) {
    Register ret_reg = Register::NoRegister;
    uint16_t sub_reg = 0;
    
    if (inst->num_operands() == 1) {
        // If the instruction uses a value, move it to the relevant return
        // register.
        //
        // @Todo: adjust per machine ABI.

        const Value* value = inst->get_operand(0);
        if (value->get_type()->is_float_type()) {
            ret_reg = XMM0;
        } else {
            ret_reg = RAX;
            sub_reg = get_subregister(value->get_type());
        }

        MachInst& move = emit(X64_Mnemonic::MOV, as_size(value->get_type()))
            .add_operand(as_operand(value))
            .add_reg(ret_reg, sub_reg, false)
            .add_comment(get_asm_comment(inst));
    }

    MachInst& ret = emit(X64_Mnemonic::RET);

    // Emit a comment on the ret if one wasn't added onto the register move.
    if (inst->num_operands() != 1)
        ret.add_comment(get_asm_comment(inst));

    // If a value was returned, add the return register as an implicit use.
    if (ret_reg != Register::NoRegister)
        ret.add_reg(ret_reg, sub_reg, false, true);
}

void InstSelector::select_call(const Instruction* inst) {
    // @Todo: Add stack spilling for calls with more than 6 arguments.
    const Value* first_oper = inst->get_operand(0);
    bool emitted_comment = false;

    std::vector<Register> regs = {};
    regs.reserve(inst->num_operands() - 1); // All args, but not callee.

    // Move arguments to their respective ABI register, in reverse order.
    for (int32_t i = inst->num_operands() - 2; i >= 0; --i) {
        const Value* arg = inst->get_operand(i + 1);
        MachOperand source = as_operand(arg);
        MachOperand dest = as_argument(arg, i);
        dest.set_is_def(true);
        regs.push_back(dest.get_reg());

        X64_Mnemonic op = dynamic_cast<const Local*>(arg)
            ? X64_Mnemonic::LEA
            : X64_Mnemonic::MOV;

        MachInst& move = emit(op, as_size(arg->get_type()), { source, dest });
        if (!emitted_comment) {
            move.add_comment(get_asm_comment(inst));
            emitted_comment = true;
        }
    }

    const Function* callee = dynamic_cast<const Function*>(first_oper);
    assert(callee && "OpCall does not use a function!");

    MachInst& call = emit(X64_Mnemonic::CALL)
        .add_symbol(callee->get_name());

    if (!emitted_comment)
        call.add_comment(get_asm_comment(inst));
    
    for (Register reg : regs)
        call.add_reg(reg, 8, false, true, true);

    if (inst->is_def()) {
        // If the call instruction is a def, i.e. the callee returns a value,
        // then move it to the defined register.

        // @Todo: carve out into function that decides return register based
        // on machine ABI. That function can then be reused by the return
        // selection.
        const bool is_float = inst->get_type()->is_float_type();
        Register ret_reg;
        uint32_t sub_reg = 0;
        if (is_float) {
            ret_reg = XMM0;
        } else {
            ret_reg = RAX;
            sub_reg = get_subregister(inst->get_type());
        }

        // Add the return register as an implicit def of the call instruction.
        call.add_reg(ret_reg, sub_reg, true, true);

        const X64_Mnemonic op = is_float ? X64_Mnemonic::MOVS : X64_Mnemonic::MOV;
        emit(op, as_size(inst->get_type()))
            .add_reg(ret_reg, sub_reg, false, false, true) // move kills return register
            .add_reg(as_register(inst), sub_reg, true);
    }
}

void InstSelector::select_iadd(const Instruction* inst) {
    MachOperand lhs = as_operand(inst->get_operand(0));
    MachOperand rhs = as_operand(inst->get_operand(1));

    if (rhs.is_imm()) {
        // Only left operand can be an immediate, so shuffle the operands.
        MachOperand tmp = lhs;
        lhs = rhs;
        rhs = tmp;
    }

    // @Todo: a lot of x64 arithmetic operations implicitly kill values, but
    // whether that information is needed for register liveness is kind of on
    // the fence. Keeping for future reference.
    // 
    // if (rhs.is_reg()) {
    //     instr.get_operand(1).set_is_kill();
    //     instr.add_reg(rhs.get_reg(), 8, true);
    // }

    emit(X64_Mnemonic::ADD, as_size(inst->get_type()), { lhs, rhs })
        .add_comment(get_asm_comment(inst));

    emit(X64_Mnemonic::MOV, as_size(inst->get_type()), { rhs })
        .add_reg(as_register(inst), get_subregister(inst->get_type()), true);
}

void InstSelector::select_fadd(const Instruction* inst) {
    MachOperand lhs = as_operand(inst->get_operand(0));
    MachOperand rhs = as_operand(inst->get_operand(1));

    if (rhs.is_imm()) {
        // Only left operand can be an immediate, so shuffle the operands.
        MachOperand tmp = lhs;
        lhs = rhs;
        rhs = tmp;
    }

    emit(X64_Mnemonic::ADDS, as_size(inst->get_type()), { lhs, rhs })
        .add_comment(get_asm_comment(inst));

    emit(X64_Mnemonic::MOVS, as_size(inst->get_type()), { rhs })
        .add_reg(as_register(inst), get_subregister(inst->get_type()), true);
}

void InstSelector::select_isub(const Instruction* inst) {
    const X64_Size size = as_size(inst->get_type());
    const MachOperand lhs = as_operand(inst->get_operand(0));
    const MachOperand rhs = as_operand(inst->get_operand(1));

    if (lhs.is_imm()) {
        MachOperand dst = MachOperand::create_reg(
            as_register(inst), get_subregister(inst->get_type()), true);

        emit(X64_Mnemonic::MOV, size, { lhs, dst })
            .add_comment(get_asm_comment(inst));
        emit(X64_Mnemonic::SUB, size, { rhs, dst });
    } else {
        // Left hand side is not an immediate, so move the right operand to the
        // destination first (so that even if it's an immediate it doesn't 
        // matter, which tends to be the more common case anyways).
        emit(X64_Mnemonic::SUB, size, { rhs, lhs })
            .add_comment(get_asm_comment(inst));
        emit(X64_Mnemonic::MOV, size, { lhs })
            .add_reg(as_register(inst), get_subregister(inst->get_type()), true);
    }   
}

void InstSelector::select_fsub(const Instruction* inst) {
    const X64_Size size = as_size(inst->get_type());
    const MachOperand lhs = as_operand(inst->get_operand(0));
    const MachOperand rhs = as_operand(inst->get_operand(1));

    if (lhs.is_imm()) {
        MachOperand dst = MachOperand::create_reg(
            as_register(inst), get_subregister(inst->get_type()), true);

        emit(X64_Mnemonic::MOVS, size, { lhs, dst })
            .add_comment(get_asm_comment(inst));
        emit(X64_Mnemonic::SUBS, size, { rhs, dst });
    } else {
        // Left hand side is not an immediate, so move the right operand to the
        // destination first (so that even if it's an immediate it doesn't 
        // matter, which tends to be the more common case anyways).
        emit(X64_Mnemonic::SUBS, size, { rhs, lhs })
            .add_comment(get_asm_comment(inst));
        emit(X64_Mnemonic::MOVS, size, { lhs })
            .add_reg(as_register(inst), get_subregister(inst->get_type()), true);
    } 
}

void InstSelector::select_imul(const Instruction* inst) {
    MachOperand lhs = as_operand(inst->get_operand(0));
    MachOperand rhs = as_operand(inst->get_operand(1));
    MachOperand dest = MachOperand::create_reg(
        as_register(inst), get_subregister(inst->get_type()), true);

    if (rhs.is_imm()) {
        MachOperand tmp = lhs;
        lhs = rhs;
        rhs = tmp;
    }

    const X64_Size size = as_size(inst->get_type());
    emit(X64_Mnemonic::MOV, size, { lhs, dest })
        .add_comment(get_asm_comment(inst));
    emit(X64_Mnemonic::IMUL, size, { rhs, dest });
}

void InstSelector::select_division(const Instruction* inst) {
    const X64_Size size = as_size(inst->get_type());
    bool is_mod = inst->op() == OP_SMOD || inst->op() == OP_UMOD;

    const Value* lhs_value = inst->get_operand(0);
    const Value* rhs_value = inst->get_operand(1);

    MachOperand lhs = as_operand(lhs_value);
    MachOperand rhs = as_operand(rhs_value);

    emit(X64_Mnemonic::MOV, size, { lhs })
        .add_reg(RAX, get_subregister(lhs_value->get_type()), true)
        .add_comment(get_asm_comment(inst));

    const MachOperand dest = MachOperand::create_reg(
        as_register(inst), get_subregister(inst->get_type()), true);

    emit(X64_Mnemonic::MOV, size, { rhs, dest });

    if (inst->op() == OP_SDIV || inst->op() == OP_SMOD) {
        emit(X64_Mnemonic::CQO) // cqo
            .add_reg(RAX, 8, true, true) // impl-def %rax
            .add_reg(RDX, 8, true, true) // impl-def %rdx
            .add_reg(RAX, 8, false, true); // impl %rax

        emit(X64_Mnemonic::IDIV, size, { dest }) // idivx ..rhs..
            .add_reg(RAX, 8, true, true, false, is_mod) // impl (dead) %rax
            .add_reg(RDX, 8, true, true, false, !is_mod) // impl (dead) %rdx
            .add_reg(RAX, 8, false, true) // impl %rax
            .add_reg(RDX, 8, false, true, true); // impl killed %rdx
    } else {
        emit(X64_Mnemonic::MOV, X64_Size::Long) // movl $0, %edx
            .add_imm(0)
            .add_reg(RDX, 4, true, false, false, true) // dead %edx
            .add_reg(RDX, 8, true, true); // impl-def %rdx
            
        emit(X64_Mnemonic::DIV, size, { dest }) // divx ..rhs..
            .add_reg(RAX, 8, true, true, false, is_mod) // (dead) %rax
            .add_reg(RDX, 8, true, true, false, !is_mod) // (dead) %rdx
            .add_reg(RAX, 8, false, true) // impl %rax
            .add_reg(RDX, 8, false, true, true); // impl killed %rdx
    }

    if (is_mod) {
        // Mark RDX as an implicit kill here, since the remainder is now in 
        // dest.
        emit(X64_Mnemonic::MOV)
            .add_reg(RDX, get_subregister(inst->get_type()), false, false, true)
            .add_operand(dest);
    } else {
        // Mark RAX as an implicit kill here, since the quotient is now in 
        // dest.
        emit(X64_Mnemonic::MOV)
            .add_reg(RAX, get_subregister(inst->get_type()), false, false, true)
            .add_operand(dest);
    }
}

void InstSelector::select_float_mul_div(const Instruction* inst) {
    const X64_Size size = as_size(inst->get_type());
    MachOperand lhs = as_operand(inst->get_operand(0));
    MachOperand rhs = as_operand(inst->get_operand(1));

    X64_Mnemonic op;
    switch (inst->op()) {
        case OP_FMUL:
            op = X64_Mnemonic::MULS;
            break;
        case OP_FDIV:
            op = X64_Mnemonic::DIVS;
            break;
        default:
            assert(false && "unexpected opcode!");
    }

    if (lhs.is_constant()) {
        MachOperand tmp = MachOperand::create_reg(XMM0, 0, true);
        emit(X64_Mnemonic::MOVS, size, { lhs, tmp })
            .add_comment(get_asm_comment(inst));

        lhs = tmp;
        lhs.set_is_use();
        lhs.set_is_kill();
    }
 
    MachInst& add = emit(op, size, { rhs, lhs });
    if (!lhs.is_constant())
        add.add_comment(get_asm_comment(inst));

    emit(X64_Mnemonic::MOVS, size, { lhs })
        .add_reg(as_register(inst), 8, true);
}

void InstSelector::select_logic(const Instruction* inst) {
    X64_Mnemonic op;
    switch (inst->op()) {
        case OP_AND:
            op = X64_Mnemonic::AND;
            break;
        case OP_OR:
            op = X64_Mnemonic::OR;
            break;
        case OP_XOR:
            op = X64_Mnemonic::XOR;
            break;
        default:
            assert("expected AndInstr or OrInstr or XorInstr opcode!");
    }
    
    // @Todo: support operand shuffling.

    MachOperand lhs = as_operand(inst->get_operand(0));
    MachOperand rhs = as_operand(inst->get_operand(1));
    const X64_Size size = as_size(inst->get_type());

    emit(op, size, { lhs, rhs })
        .add_comment(get_asm_comment(inst));

    emit(X64_Mnemonic::MOV, size, { rhs })
        .add_reg(as_register(inst), get_subregister(inst->get_type()), true);
}

void InstSelector::select_shift(const Instruction* inst) {
    X64_Mnemonic op;
    switch (inst->op()) {
        case OP_SHL:
            op = X64_Mnemonic::SHL;
            break;
        case OP_SHR:
            op = X64_Mnemonic::SHR;
            break;
        case OP_SAR:
            op = X64_Mnemonic::SAR;
            break;
        default:
            assert(false && "unexpected opcode!");
    }

    MachOperand lhs = as_operand(inst->get_operand(0));
    MachOperand rhs = as_operand(inst->get_operand(1));
    MachOperand dest = MachOperand::create_reg(
        as_register(inst), get_subregister(inst->get_type()), true);

    emit(X64_Mnemonic::MOV, as_size(inst->get_operand(0)->get_type()), { lhs, dest })
        .add_comment(get_asm_comment(inst));

    dest.set_is_use();

    if (rhs.is_imm()) {
        emit(op, as_size(inst->get_operand(1)->get_type()), { rhs, dest });
    } else {
        MachOperand CL = MachOperand::create_reg(RCX, 1, true);
            
        if (rhs.is_reg())
            rhs.set_subreg(1);

        emit(X64_Mnemonic::MOV, X64_Size::Byte, { rhs, CL });
        emit(op, as_size(inst->get_type()), { CL, dest });
    }
}

void InstSelector::select_not(const Instruction* inst) {
    const MachOperand source = as_operand(inst->get_operand(0));
    const X64_Size size = as_size(inst->get_type());

    emit(X64_Mnemonic::NOT, size, { source })
        .add_comment(get_asm_comment(inst));

    emit(X64_Mnemonic::MOV, size, { source })
        .add_reg(as_register(inst), get_subregister(inst->get_type()), true);
}

void InstSelector::select_negate(const Instruction* inst) {
    MachOperand source = as_operand(inst->get_operand(0));
    const X64_Size size = as_size(inst->get_type());

    if (inst->op() == OP_INEG) {
        emit(X64_Mnemonic::NEG, size, { source })
            .add_comment(get_asm_comment(inst));
        emit(X64_Mnemonic::MOV, size, { source })
            .add_reg(as_register(inst), get_subregister(inst->get_type()), true);
    } else if (inst->op() == OP_FNEG) {
        // @Todo: implement OpFNeg selection, also needs mask constants.
    } else {
        assert(false && "expected OpINeg or OpFNeg opcode!");
    }
}

void InstSelector::select_extension(const Instruction* inst) {
    const Value* value = inst->get_operand(0);
    const uint32_t source_size = m_mach.get_size(value->get_type());
    const uint32_t dest_size = m_mach.get_size(inst->get_type());
    uint32_t dest_subreg = get_subregister(inst->get_type());

    X64_Mnemonic op;
    switch (inst->op()) {
        case OP_SEXT:
            if (source_size == 4 && dest_size == 8) {
                op = X64_Mnemonic::MOVSXD;
            } else {
                op = X64_Mnemonic::MOVSX;
            }
            break;

        case OP_ZEXT:
            if (source_size == 4 && dest_size == 8) {
                op = X64_Mnemonic::MOV;
                dest_subreg = 4;
            } else {
                op = X64_Mnemonic::MOVZX;
            }
            break;
            
        case OP_FEXT:
            op = X64_Mnemonic::CVTSS2SD;
            break;

        default:
            assert(false && "expected OpSExt, OpZExt or OpFExt!");
    }

    emit(op, X64_Size::None, { as_operand(value) })
        .add_reg(as_register(inst), dest_subreg, true)
        .add_comment(get_asm_comment(inst));
}

void InstSelector::select_truncation(const Instruction* inst) {
    MachOperand src = as_operand(inst->get_operand(0));
    uint32_t dst_subreg = get_subregister(inst->get_type());
    
    X64_Mnemonic op;
    switch (inst->op()) {
        case OP_ITRUNC:
            if (src.is_reg())
                src.set_subreg(dst_subreg);
            
            op = X64_Mnemonic::MOV;
            break;
        case OP_FTRUNC:
            op = X64_Mnemonic::CVTSD2SS;
            break;
        default:
            assert(false && "expected OpITrunc or OpFTrunc!");
    }

    emit(op, X64_Size::None, { src })
        .add_reg(as_register(inst), dst_subreg, true)
        .add_comment(get_asm_comment(inst));
}

void InstSelector::select_cast_i2f(const Instruction* inst) {
    X64_Mnemonic op;
    if (inst->get_type()->is_float_type(32)) {
        op = X64_Mnemonic::CVTSI2SS;
    } else if (inst->get_type()->is_float_type(64)) {
        op = X64_Mnemonic::CVTSI2SD;
    } else {
        assert(false && "invalid integer to fp conversion destination type!");
    }

    emit(op, X64_Size::None, { as_operand(inst->get_operand(0)) })
        .add_reg(as_register(inst), get_subregister(inst->get_type()), true)
        .add_comment(get_asm_comment(inst));
}

void InstSelector::select_cast_f2i(const Instruction* inst) {
    const Value* operand = inst->get_operand(0);
    
    X64_Mnemonic op;
    if (operand->get_type()->is_float_type(32)) {
        op = X64_Mnemonic::CVTTSS2SI;
    } else if (operand->get_type()->is_float_type(64)) {
        op = X64_Mnemonic::CVTTSD2SI;
    } else {
        assert(false && "invalid fp to integer conversion destination type!");
    }

    emit(op, as_size(inst->get_type()), { as_operand(operand) })
        .add_reg(as_register(inst), get_subregister(inst->get_type()), true)
        .add_comment(get_asm_comment(inst));
}

void InstSelector::select_cast_p2i(const Instruction* inst) {
    const Value* source = inst->get_operand(0);

    X64_Mnemonic op;
    if (dynamic_cast<const Local*>(source)) {
        op = X64_Mnemonic::LEA;
    } else {
        op = X64_Mnemonic::MOV;
    }

    // @Todo: consider what happens when the destination integer type is not
    // 64-bit.

    emit(op, X64_Size::Quad)
        .add_operand(as_operand(source))
        .add_reg(as_register(inst), get_subregister(inst->get_type()), true)
        .add_comment(get_asm_comment(inst));
}

void InstSelector::select_cast_i2p(const Instruction* inst) {
    emit(X64_Mnemonic::MOV, X64_Size::Quad)
        .add_operand(as_operand(inst->get_operand(0)))
        .add_reg(as_register(inst), get_subregister(inst->get_type()), true)
        .add_comment(get_asm_comment(inst));
}

void InstSelector::select_cast_reinterpret(const Instruction* inst) {
    const Value* source = inst->get_operand(0);

    X64_Mnemonic op;
    if (dynamic_cast<const Local*>(source)) {
        op = X64_Mnemonic::LEA;
    } else {
        op = X64_Mnemonic::MOV;
    }

    emit(op, X64_Size::Quad)
        .add_operand(as_operand(source))
        .add_reg(as_register(inst), get_subregister(inst->get_type()), true)
        .add_comment(get_asm_comment(inst));
}

void InstSelector::run() {
    const Function* fn = m_func.get_function();
    StackFrame& stack = m_func.get_stack_frame();
    uint32_t stack_index = 0, stack_off = 0;

    // For each local in the function, reserve some space on the stack for its
    // allocated type.
    for (const auto& [name, local] : fn->get_locals()) {
        const Type* alloc_type = local->get_allocated_type();

        StackEntry entry = {};
        entry.offset = stack_off;

        uint32_t size = m_mach.get_size(alloc_type);
        stack_off += size;

        entry.size = size;
        entry.align = m_mach.get_align(alloc_type);
        entry.local = local;

        stack.entries.push_back(entry);
        m_locals.emplace(local, stack_index++);
    }

    MachLabel* curr = m_func.get_head();
    while (curr) {
        m_insert = curr;
        const BasicBlock* block = curr->get_basic_block();
        assert(block);

        for (const Instruction* inst = block->get_head(); inst; inst = inst->get_next())
            select(inst);

        curr = curr->get_next();
    }
}

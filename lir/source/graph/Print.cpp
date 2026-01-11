//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lir/graph/BasicBlock.hpp"
#include "lir/graph/CFG.hpp"
#include "lir/graph/Constant.hpp"
#include "lir/graph/Function.hpp"
#include "lir/graph/Global.hpp"
#include "lir/graph/Instruction.hpp"
#include "lir/graph/Local.hpp"

#include <format>

using namespace lir;

static const char* to_string(Mnemonic op) {
    switch (op) {
        case OP_ABORT:
            return "OpAbort";
        case OP_ACCESS:
            return "OpAccess";
        case OP_AND:
            return "OpAnd";
        case OP_AP:
            return "OpAp";
        case OP_CALL:
            return "OpCall";
        case OP_CMP:
            return "OpCmp";
        case OP_F2I:
            return "OpF2I";
        case OP_FADD:
            return "OpFAdd";
        case OP_FDIV:
            return "OpFDiv";
        case OP_FEXT:
            return "OpFExt";
        case OP_FMUL:
            return "OpFMul";
        case OP_FNEG:
            return "OpFNeg";
        case OP_FSUB:
            return "OpFSub";
        case OP_FTRUNC:
            return "OpFTrunc";
        case OP_I2P:
            return "OpI2P";
        case OP_IADD:
            return "OpIAdd";
        case OP_INEG:
            return "OpINeg";
        case OP_ISUB:
            return "OpISub";
        case OP_ITRUNC:
            return "OpITrunc";
        case OP_JIF:
            return "OpJif";
        case OP_JMP:
            return "OpJmp";
        case OP_LOAD:
            return "OpLoad";
        case OP_NOP:
            return "OpNop";
        case OP_NOT:
            return "OpNot";
        case OP_OR:
            return "OpOr";
        case OP_P2I:
            return "OpP2I";
        case OP_REINT:
            return "OpReint";
        case OP_RET:
            return "OpRet";
        case OP_S2F:
            return "OpS2F";
        case OP_SAR:
            return "OpSar";
        case OP_SDIV:
            return "OpSDiv";
        case OP_SEXT:
            return "OpSExt";
        case OP_SHL:
            return "OpShl";
        case OP_SHR:
            return "OpShr";
        case OP_SMUL:
            return "OpSMul";
        case OP_SREM:
            return "OpSRem";
        case OP_STORE:
            return "OpStore";
        case OP_STRING:
            return "OpString";
        case OP_SYSCALL:
            return "OpSyscall";
        case OP_U2F:
            return "OpU2F";
        case OP_UDIV:
            return "OpUDiv";
        case OP_UMUL:
            return "OpUMul";
        case OP_UNREACHABLE:
            return "OpUnreachable";
        case OP_UREM:
            return "OpURem";
        case OP_XOR:
            return "OpXor";
        case OP_ZEXT:
            return "OpZExt";
    }
}

static const char* to_string(CMPPredicate pred) {
    switch (pred) {
        case CMP_IEQ:
            return "IEQ";
        case CMP_INE:
            return "INE";
        case CMP_OEQ:
            return "OEQ";
        case CMP_ONE:
            return "ONE";
        case CMP_SLT:
            return "SLT";
        case CMP_SLE:
            return "SLE";
        case CMP_SGT:
            return "SGT";
        case CMP_SGE:
            return "SGE";
        case CMP_ULT:
            return "ULT";
        case CMP_ULE:
            return "ULE";
        case CMP_UGT:
            return "UGT";
        case CMP_UGE:
            return "UGE";
        case CMP_OLT:
            return "OLT";
        case CMP_OLE:
            return "OLE";
        case CMP_OGT:
            return "OGT";
        case CMP_OGE:
            return "OGE";
    }
}

static void print_instruction(std::ostream& os, Instruction& inst) {
    if (inst.is_def())
        os << std::format("${} := ", inst.get_def());
    
    const Mnemonic op = inst.op();
    os << to_string(op) << ' ';

    if (op == OP_CALL) 
    {
        inst.get_operand(0)->print(os);

        os << " (";
        for (uint32_t i = 1, e = inst.num_operands(); i < e; ++i) 
        {
            inst.get_operand(i)->print(os);
            if (i + 1 != e)
                os << ", ";
        }

        os << ")";
    } 
    else if (op == OP_JMP)
    {
        inst.get_operand(0)->print(os);

        if (inst.num_operands() > 1) 
        {
            os << "(";
            for (uint32_t i = 1, e = inst.num_operands(); i < e; ++i) 
            {
                inst.get_operand(i)->print(os);
                if (i + 1 != e)
                    os << ", ";
            }

            os << ")";
        }
    } 
    else if (op == OP_JIF) 
    {
        inst.get_operand(0)->print(os);
        os << ", ";
        inst.get_operand(1)->print(os);

        uint32_t i = 2;

        if (dynamic_cast<lir::BlockAddress*>(inst.get_operand(2))) 
        {
            // True destination has no arguments.
            os << ", ";
            inst.get_operand(2)->print(os);
            i = 3;
        } 
        else 
        {
            os << "(";

            for (uint32_t e = inst.num_operands(); i < e; ++i) 
            {
                Value* op = inst.get_operand(i);
                if (dynamic_cast<lir::BlockAddress*>(op)) {
                    break; // We're done with true block arguments.
                } else if (i != 2) {
                    os << ", ";
                }
                
                op->print(os);
            }

            os << "), ";
            inst.get_operand(i++)->print(os);
        }

        if (i < inst.num_operands()) {
            os << "(";

            for (uint32_t e = inst.num_operands(); i < e; ++i) {
                inst.get_operand(i)->print(os);
                if (i + 1 != e)
                    os << ", ";
            }

            os << ")";
        }
    } 
    else if (op == OP_STORE) 
    {
        inst.get_operand(0)->print(os);
        os << " -> ";
        inst.get_operand(1)->print(os);
        os << std::format(" |{}|", inst.desc().alignment);
    } 
    else 
    {
        if (op == OP_CMP)
            os << to_string(inst.desc().cmp) << ' ';

        for (uint32_t i = 0, e = inst.num_operands(); i < e; ++i) 
        {
            inst.get_operand(i)->print(os);
            if (i + 1 != e)
                os << ", ";
        }

        if (op == OP_LOAD)
            os << std::format(" |{}|", inst.desc().alignment);
    }

    os << '\n';
}

static void print_basicblock(std::ostream& os, BasicBlock& block) {
    os << std::format("bb{}", block.get_number());

    if (block.has_args()) {
        os << " (";
    } else {
        os << ":\n";
    }

    for (uint32_t i = 0, e = block.num_args(); i < e; ++i) {
        BasicBlock::Arg* arg = block.get_arg(i);

        os << std::format("p{}: {}", 
            arg->get_index(), 
            arg->get_type()->to_string());

        if (i + 1 != e)
            os << ", ";
    }

    if (block.has_args()) {
        os << "):\n";
    }

    for (Instruction* curr = block.get_head(); curr; curr = curr->get_next()) {
        os << '\t';
        print_instruction(os, *curr);
    }
}

static void print_local(std::ostream& os, Local& local) {
    os << std::format("#{} := {} |{}|\n", 
        local.get_name(), 
        local.get_allocated_type()->to_string(), 
        local.get_alignment());
}

static void print_function(std::ostream& os, Function& function) {
    os << std::format("{} :: ", function.get_name());

    switch (function.get_linkage()) {
        case Function::Internal:
            os << "int ";
            break;
        case Function::External:
            os << "ext ";
            break;
    }

    os << '(';

    for (uint32_t i = 0, e = function.num_args(); i < e; ++i) {
        lir::Function::Arg* arg = function.get_arg(i);

        if (arg->has_name()) {
            os << std::format("{}: {}", 
                arg->get_name(), 
                arg->get_type()->to_string());
        } else {
            os << std::format("{}", arg->get_type()->to_string());
        }

        if (i + 1 != e)
            os << ", ";
    }

    os << std::format(") -> {}", function.get_return_type()->to_string());

    if (function.empty()) {
        os << ";\n";
        return;
    } else {
        os << " {\n";
    }

    for (auto& pair : function.get_locals()) {
        os << '\t';
        print_local(os, *pair.second);
    }

    for (BasicBlock* curr = function.get_head(); curr; curr = curr->get_next())
        print_basicblock(os, *curr);

    os << "}\n";
}

static void print_global(std::ostream& os, Global& global) {
    os << std::format("@{} := ", global.get_name());

    switch (global.get_linkage()) {
        case Global::Internal:
            os << "int ";
            break;
        case Global::External:
            os << "ext ";
            break;
    }

    if (global.is_read_only())
        os << "read-only ";

    os << global.get_type()->to_string();

    if (global.has_initializer()) {
        os << ' ';
        global.get_initializer()->print(os);
    }
    
    os << '\n';
}

void BasicBlock::print(std::ostream& os) const {
    os << std::format("bb{}", get_number());
}

void BasicBlock::Arg::print(std::ostream& os) const {
    os << std::format("p{}: {}", get_index(), m_type->to_string());
}

void BlockAddress::print(std::ostream& os) const {
    m_block->print(os);
}

void CFG::print(std::ostream& os) const {
    os << std::format("LIR_CONTROL_FLOW_GRAPH \"{}\"\n\n", m_filename);

    for (const auto& pair : m_types.structs) {
        StructType* type = pair.second;

        os << std::format("{} :: struct {{ ", type->get_name());

        for (uint32_t i = 0, e = type->num_fields(); i < e; ++i) {
            os << type->get_field(i)->to_string();
            if (i + 1 != e)
                os << ", ";
        }

        os << " }\n";
    }

    if (!m_types.structs.empty())
        os << '\n';

    for (const auto& pair : m_globals) {
        print_global(os, *pair.second);
        os << '\n';
    }

    if (!m_globals.empty())
        os << '\n';

    uint32_t i = 0, e = m_functions.size();
    for (const auto& pair : m_functions) {
        print_function(os, *pair.second);
        if (++i != e)
            os << '\n';
    }
}

void Float::print(std::ostream& os) const {
    os << std::format("{}: {}", m_value, m_type->to_string());
}

void Function::print(std::ostream& os) const {
    os << std::format("{}: {}", m_name, get_return_type()->to_string());
}

void Function::Arg::print(std::ostream& os) const {
    os << std::format("{}: {}", m_name, m_type->to_string());
}

void Global::print(std::ostream& os) const {
    os << std::format("@{}: {}", m_name, m_type->to_string());
}

void Instruction::print(std::ostream& os) const {
    assert(is_def() && "cannot print non-def instruction!");

    os << std::format("${}: {}", m_def, m_type->to_string());
}

void Integer::print(std::ostream& os) const {
    os << std::format("{}: {}", m_value, m_type->to_string());
}

void Local::print(std::ostream& os) const {
    os << std::format("#{}: {}", m_name, m_type->to_string());
}

void Null::print(std::ostream& os) const {
    os << "null";
}

void String::print(std::ostream& os) const {
    os << std::format("\"{}\"", m_value);
}

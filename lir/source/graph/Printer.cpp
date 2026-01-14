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

static const char* to_string(CMPPredicate pred) {
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

static void print_instruction(std::ostream& os, Instruction& inst) {
    if (inst.is_def())
        os << std::format("${} = ", inst.get_def());
    
    const Mnemonic op = inst.op();
    os << to_string(op) << ' ';

    if (inst.is_cast())
        os << std::format("({}) ", inst.get_type()->to_string());

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
            os << std::format(" |{}", inst.desc().alignment);
    }

    os << '\n';
}

static void print_basicblock(std::ostream& os, BasicBlock& block) {
    os << std::format("bb{}", block.get_number());

    if (block.has_args()) {
        os << "(";
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
    os << std::format("{} := {} |{}\n", 
        local.get_name(), 
        local.get_allocated_type()->to_string(), 
        local.get_alignment());
}

static void print_function(std::ostream& os, Function& function) {
    os << std::format("{} :: ", function.get_name());

    switch (function.get_linkage()) {
        case Function::Internal:
            os << "internal ";
            break;
        case Function::External:
            os << "external ";
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
            os << "internal ";
            break;
        case Global::External:
            os << "external ";
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

        os << std::format("{} :: {{ ", type->get_name());

        for (uint32_t i = 0, e = type->num_fields(); i < e; ++i) {
            os << type->get_field(i)->to_string();
            if (i + 1 != e)
                os << ", ";
        }

        os << " }\n";
    }

    if (!m_types.structs.empty())
        os << '\n';

    for (const auto& pair : m_globals)
        print_global(os, *pair.second);

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
    os << std::format("{}: {}", m_name, m_type->to_string());
}

void Null::print(std::ostream& os) const {
    os << "null";
}

void String::print(std::ostream& os) const {
    os << std::format("\"{}\"", m_value);
}

void Aggregate::print(std::ostream& os) const {
    os << "{";

    for (uint32_t i = 0, e = num_operands(); i < e; ++i) {
        get_value(i)->print(os);
        
        if (i + 1 != e)
            os << ", ";
    }

    os << std::format("}}: {}", get_type()->to_string());
}

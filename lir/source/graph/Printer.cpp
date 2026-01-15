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
#include "lir/graph/Value.hpp"

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

void BasicBlock::print(std::ostream& os, PrintPolicy policy) const {
    switch (policy) {
        case PrintPolicy::Def:
            os << std::format("bb{}", get_number());

            if (has_args()) {
                os << "(";
            } else {
                os << ":\n";
            }

            for (uint32_t i = 0, e = num_args(); i < e; ++i) {
                get_arg(i)->print(os, PrintPolicy::Def);
                if (i + 1 != e)
                    os << ", ";
            }

            if (has_args())
                os << "):\n";

            for (const Instruction* curr = get_head(); curr; curr = curr->get_next()) {
                os << '\t';
                curr->print(os, PrintPolicy::Def);
            }

            break;
        
        case PrintPolicy::Use:
            os << std::format("bb{}", get_number());
            break;
    }
}

void BasicBlock::Arg::print(std::ostream& os, PrintPolicy policy) const {
    os << std::format("p{}: {}", get_index(), get_type()->to_string());
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
        pair.second->print(os, PrintPolicy::Def);

    if (!m_globals.empty())
        os << '\n';

    uint32_t i = 0, e = m_functions.size();
    for (const auto& pair : m_functions) {
        pair.second->print(os, PrintPolicy::Def);
        if (++i != e)
            os << '\n';
    }
}

void Function::print(std::ostream& os, PrintPolicy policy) const {
    switch (policy) {
        case PrintPolicy::Def:
            os << std::format("{} :: ", get_name());

            switch (get_linkage()) {
                case Function::Internal:
                    os << "internal ";
                    break;
                case Function::External:
                    os << "external ";
                    break;
            }

            os << '(';

            for (uint32_t i = 0, e = num_args(); i < e; ++i) {
                const lir::Function::Arg* arg = get_arg(i);

                if (arg->has_name()) {
                    arg->print(os, PrintPolicy::Def);
                } else {
                    os << std::format("{}", arg->get_type()->to_string());
                }

                if (i + 1 != e)
                    os << ", ";
            }

            os << std::format(") -> {}", get_return_type()->to_string());

            if (empty()) {
                os << ";\n";
                return;
            } else {
                os << " {\n";
            }

            for (auto& pair : get_locals()) {
                os << '\t';
                pair.second->print(os, PrintPolicy::Def);
            }

            for (const BasicBlock* curr = get_head(); curr; curr = curr->get_next())
                curr->print(os, PrintPolicy::Def); 

            os << "}\n";
            break;

        case PrintPolicy::Use:
            os << std::format("{}: {}", m_name, get_return_type()->to_string());
            break;
    }
}

void Function::Arg::print(std::ostream& os, PrintPolicy policy) const {
    switch (policy) {
        case PrintPolicy::Def:
            if (has_name()) {
                os << std::format("{}: {}", get_name(), get_type()->to_string());
            } else {
                os << std::format("{}", get_type()->to_string());
            }

            break;

        case PrintPolicy::Use:
            assert(has_name() && "cannot use unnamed argument!");
            os << std::format("{}: {}", get_name(), get_type()->to_string());
            break;
    }
}

void Global::print(std::ostream& os, PrintPolicy policy) const {
    switch (policy) {
        case PrintPolicy::Def:
            os << std::format("@{} := ", get_name());

            switch (get_linkage()) {
                case Global::Internal:
                    os << "internal ";
                    break;
                case Global::External:
                    os << "external ";
                    break;
            }

            if (is_read_only())
                os << "read-only ";

            os << m_type->to_string();

            if (has_initializer()) {
                os << ' ';
                get_initializer()->print(os, PrintPolicy::Use);
            }
            
            os << '\n';
            break;

        case PrintPolicy::Use:
            os << std::format("@{}: {}", m_name, m_type->to_string());
            break;
    }
}

void Instruction::print(std::ostream& os, PrintPolicy policy) const {
    switch (policy) {
        case PrintPolicy::Def: {
            if (is_def())
                os << std::format("${} = ", get_def());
            
            const Mnemonic op = m_op;
            os << to_string(op) << ' ';

            if (is_cast())
                os << std::format("({}) ", get_type()->to_string());

            if (op == OP_CALL) {
                get_operand(0)->print(os, PrintPolicy::Use);

                os << " (";
                for (uint32_t i = 1, e = num_operands(); i < e; ++i) 
                {
                    get_operand(i)->print(os, PrintPolicy::Use);
                    if (i + 1 != e)
                        os << ", ";
                }

                os << ")";
            } else if (op == OP_JMP) {
                get_operand(0)->print(os, PrintPolicy::Use);

                if (num_operands() > 1) {
                    os << "(";
                    for (uint32_t i = 1, e = num_operands(); i < e; ++i) {
                        get_operand(i)->print(os, PrintPolicy::Use);
                        if (i + 1 != e)
                            os << ", ";
                    }

                    os << ")";
                }
            } else if (op == OP_JIF) {
                get_operand(0)->print(os, PrintPolicy::Use);
                os << ", ";
                get_operand(1)->print(os, PrintPolicy::Use);

                uint32_t i = 2;

                if (dynamic_cast<const lir::BlockAddress*>(get_operand(2))) {
                    // True destination has no arguments.
                    os << ", ";
                    get_operand(2)->print(os, PrintPolicy::Use);
                    i = 3;
                } else {
                    os << "(";

                    for (uint32_t e = num_operands(); i < e; ++i) {
                        const Value* op = get_operand(i);
                        if (dynamic_cast<const lir::BlockAddress*>(op)) {
                            break; // We're done with true block arguments.
                        } else if (i != 2) {
                            os << ", ";
                        }
                        
                        op->print(os, PrintPolicy::Use);
                    }

                    os << "), ";
                    get_operand(i++)->print(os, PrintPolicy::Use);
                }

                if (i < num_operands()) {
                    os << "(";

                    for (uint32_t e = num_operands(); i < e; ++i) {
                        get_operand(i)->print(os, PrintPolicy::Use);
                        if (i + 1 != e)
                            os << ", ";
                    }

                    os << ")";
                }
            } else {
                if (op == OP_CMP)
                    os << to_string(desc().cmp) << ' ';

                for (uint32_t i = 0, e = num_operands(); i < e; ++i) {
                    get_operand(i)->print(os, PrintPolicy::Use);
                    if (i + 1 != e)
                        os << ", ";
                }

                if (op == OP_LOAD)
                    os << std::format(" |{}", desc().alignment);
            }

            os << '\n';
            break;
        }

        case PrintPolicy::Use:
            assert(is_def() && "cannot print non-def instruction!");
        
            os << std::format("${}: {}", get_def(), get_type()->to_string());
            break;
    }
}

void Local::print(std::ostream& os, PrintPolicy policy) const {
    switch (policy) {
        case PrintPolicy::Def:
            os << std::format("{} := {} |{}\n", 
                get_name(), 
                get_allocated_type()->to_string(), 
                get_alignment()
            );
            break;

        case PrintPolicy::Use:
            os << std::format("{}: {}", get_name(), get_type()->to_string());
            break;
    }
}

//>==- Constant Printing

void Integer::print(std::ostream& os, PrintPolicy policy) const {
    os << std::format("{}: {}", get_value(), get_type()->to_string());
}

void Float::print(std::ostream& os, PrintPolicy policy) const {
    os << std::format("{}: {}", get_value(), get_type()->to_string());
}

void BlockAddress::print(std::ostream& os, PrintPolicy policy) const {
    m_block->print(os, PrintPolicy::Use);
}

void Null::print(std::ostream& os, PrintPolicy policy) const {
    os << std::format("null: {}", get_type()->to_string());
}

void String::print(std::ostream& os, PrintPolicy policy) const {
    os << std::format("\"{}\"", get_value());
}

void Aggregate::print(std::ostream& os, PrintPolicy policy) const {
    os << "{";

    for (uint32_t i = 0, e = num_operands(); i < e; ++i) {
        get_value(i)->print(os, PrintPolicy::Use);
        if (i + 1 != e)
            os << ", ";
    }

    os << std::format("}}: {}", get_type()->to_string());
}

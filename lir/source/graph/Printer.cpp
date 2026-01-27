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

void BasicBlock::print(std::ostream& os, PrintPolicy policy) const {
    switch (policy) {
        case PrintPolicy::Def:
            os << std::format("bb{}", position());

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
                os << '\n';
            }

            break;
        
        case PrintPolicy::Use:
            os << std::format("bb{}", position());
            break;
    }
}

void BlockArgument::print(std::ostream& os, PrintPolicy policy) const {
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

    for (const auto& pair : m_globals) {
        pair.second->print(os, PrintPolicy::Def);
        os << '\n';
    }

    if (!m_globals.empty())
        os << '\n';

    uint32_t i = 0, e = m_functions.size();
    for (const auto& pair : m_functions) {
        pair.second->print(os, PrintPolicy::Def);

        // Don't print double empty lines at the end of a graph print.
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
                const lir::FunctionArgument* arg = get_arg(i);

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

void FunctionArgument::print(std::ostream& os, PrintPolicy policy) const {
    switch (policy) {
        case PrintPolicy::Def:
            if (has_name())
                os << std::format("{}: ", get_name());
            
            switch (get_trait()) {
                case Trait::None:
                    break;
                case Trait::ARet:
                    os << "aret ";
                    break;
                case Trait::Valued:
                    os << "valued ";
                    break;
            }

            os << get_type()->to_string();
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
            os << std::format("${} := {} |{}\n", 
                get_name(), 
                get_allocated_type()->to_string(), 
                get_alignment()
            );
            break;

        case PrintPolicy::Use:
            os << std::format("${}: {}", get_name(), get_type()->to_string());
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
    os << '"';

    const std::string& string = get_value();
    for (uint32_t i = 0, e = string.size(); i < e; ++i) {
        switch (string[i]) {
            case '\\':
                os << "\\\\";
                break;
            case '\'':
                os << "\\'";
                break;
            case '\"':
                os << "\\\"";
                break;
            case '\n':
                os << "\\n";
                break;
            case '\t':
                os << "\\t";
                break;
            case '\r':
                os << "\\r";
                break;
            case '\b':
                os << "\\b";
                break;
            case '\0':
                os << "\\0";
                break;
            default:
                os << string[i];
                break;
        }
    }
    
    os << '"';
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

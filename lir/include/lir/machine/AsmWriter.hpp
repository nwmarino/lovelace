//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_ASM_WRITER_H_
#define LOVELACE_IR_ASM_WRITER_H_

#include "lir/machine/MachFunction.hpp"
#include "lir/machine/Register.hpp"
#include "lir/machine/Segment.hpp"

namespace lir {

class AsmWriter final {
    const Segment& m_seg;

    std::unordered_map<const MachFunction*, uint32_t> m_funcs = {};

    /// Returns the allocated physical register for the given virtual |reg|
    /// based off the register table within |func|.
    ///
    /// If |reg| is already within the physical register boundaries, then this
    /// function does nothing.
    X64_Register map_register(Register reg, const MachFunction& func);

    /// Test if the given |inst| is a redundant move instruction. 
    ///
    /// In other words, tests if |inst| is a move instruction where both 
    /// operands are correspond to the same physical register. This means that
    /// an instruction like `movq %rax, %rax` would be considered a redundant
    /// move.
    bool is_redundant_move(const MachFunction &func, const MachInst& inst);

    /// Emit |op| as an X64 assembly instruction operand with |func| as
    /// context. 
    void emit_operand(std::ostream& os, const MachFunction& func, 
                      const MachOperand& op);

    /// Emit |inst| as an X64 assembly instruction with |func| as context.
    void emit_inst(std::ostream& os, const MachFunction& func, 
                   const MachInst& inst);

    /// Emit |label| as an X64 assembly label with |func| as context.
    void emit_label(std::ostream& os, const MachFunction& func, 
                    const MachLabel& label);

    /// Emit |func| as an X64 assembly function.
    void emit_function(std::ostream& os, const MachFunction& func);

    /// Emit |constant| as an X64 assembly constant.
    void emit_constant(std::ostream& os, const Constant& constant);

    /// Emit |global| as a global section of X64 data. 
    void emit_global(std::ostream& os, const Global& global);

public:
    AsmWriter(const Segment& seg) : m_seg(seg) {}

    void run(std::ostream& os);
};

} // namespace lir

#endif // LOVELACE_IR_ASM_WRITER_H_

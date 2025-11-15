#ifndef SPBE_X64_ASM_WRITER_H_
#define SPBE_X64_ASM_WRITER_H_

#include "X64.hpp"
#include "../machine/AsmWriter.hpp"
#include "../machine/MachFunction.hpp"
#include "../machine/MachObject.hpp"
#include "../machine/MachRegister.hpp"

namespace spbe::x64 {

class X64AsmWriter final : public spbe::AsmWriter {
    /// Global object iterator for unique function ids.
    uint32_t m_function = 0;

    /// Returns the corresponding assembly mnemonic for the opcode |op|.
    const char* opcode_to_string(x64::Opcode op) const;

    /// Returns the allocated physical register for virtual register |reg|.
    /// If |reg| is already within the physical register boundaries, then this
    /// function does nothing.
    x64::Register map_register(MachRegister reg, const MachFunction& MF);

    /// Returns true if |MI| is a redundant move instruction. More specifically,
    /// if both operands of |MI| correspond to the same physical register.
    bool is_redundant_move(const MachFunction &MF, const MachInstruction& MI);

    void write_operand(std::ostream& os, const MachFunction& MF, 
                       const MachOperand& MO) override;

    void write_instruction(std::ostream& os, const MachFunction& MF,
                           const MachInstruction& MI) override;

    void write_block(std::ostream& os, const MachFunction& MF,
                     const MachBasicBlock& MBB) override;

    void write_function(std::ostream& os, const MachFunction& MF) override;

    void write_constant(std::ostream& os, const Constant& constant) override;

    void write_global(std::ostream& os, const Global& global) override;

public:
    X64AsmWriter(const MachObject& object) : spbe::AsmWriter(object) {}

    X64AsmWriter(const X64AsmWriter&) = delete;
    X64AsmWriter& operator = (const X64AsmWriter&) = delete;
    
    ~X64AsmWriter() = default;

    void run(std::ostream& os) override;
};

} // namespace spbe::x64

#endif // SPBE_X64_ASM_WRITER_H_

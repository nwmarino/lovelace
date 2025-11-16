#ifndef SPBE_ASM_WRITER_H_
#define SPBE_ASM_WRITER_H_

#include "MachBasicBlock.hpp"
#include "MachFunction.hpp"
#include "MachInstruction.hpp"
#include "MachObject.hpp"
#include "MachOperand.hpp"
#include "../graph/Global.hpp"

#include <ostream>

namespace spbe {

/// Provides an abstraction for target architecture assembly emission.
class AsmWriter {
protected:
    const MachObject& m_object;

    virtual void write_operand(std::ostream& os, const MachFunction& MF, 
                               const MachOperand& MO) = 0;

    virtual void write_instruction(std::ostream& os, const MachFunction& MF,
                                   const MachInstruction& MI) = 0;

    virtual void write_block(std::ostream& os, const MachFunction& MF,
                             const MachBasicBlock& MBB) = 0;

    virtual void write_function(std::ostream& os, const MachFunction& MF) = 0;

    virtual void write_constant(std::ostream& os, const Constant& constant) = 0;

    virtual void write_global(std::ostream& os, const Global& global) = 0;

public:
    AsmWriter(const MachObject& object) : m_object(object) {}

    AsmWriter(const AsmWriter&) = delete;
    AsmWriter& operator = (const AsmWriter&) = delete;

    virtual ~AsmWriter() = default;

    /// Run the assembly writer directed to the output stream |os|.
    virtual void run(std::ostream& os) = 0;
};

} // namespace spbe

#endif // SPBE_ASM_WRITER_H_

#ifndef SPBE_X64_H_
#define SPBE_X64_H_

#include "../machine/MachRegister.hpp"
#include "../machine/RegisterAllocator.hpp"

#include <cstdint>
#include <string>

namespace spbe {

class MachOperand;
class MachInstruction;
class MachBasicBlock;
class MachFunction;

namespace x64 {

/// Recognized x64 opcodes. These are really mnemonics since they don't signify
/// any operand information.
enum Opcode : uint32_t {
    NO_OPC = 0x0,

    NOP, JMP, UD2, CQO, MOV,

    CALL64, RET64, 
    LEA32, LEA64, 
    PUSH64, POP64,
    MOV8, MOV16, MOV32, MOV64,
    ADD8, ADD16, ADD32, ADD64,
    SUB8, SUB16, SUB32, SUB64,
    MUL8, MUL16, MUL32, MUL64,
    IMUL8, IMUL16, IMUL32, IMUL64,
    DIV8, DIV16, DIV32, DIV64,
    IDIV8, IDIV16, IDIV32, IDIV64,
    AND8, AND16, AND32, AND64,
    OR8, OR16, OR32, OR64,
    XOR8, XOR16, XOR32, XOR64,
    SHL8, SHL16, SHL32, SHL64,
    SHR8, SHR16, SHR32, SHR64,
    SAR8, SAR16, SAR32, SAR64,
    CMP8, CMP16, CMP32, CMP64,
    NOT8, NOT16, NOT32, NOT64,
    NEG8, NEG16, NEG32, NEG64,

    MOVABS,
    MOVSX,
    MOVSXD,
    MOVZX,

    JE, JNE, JZ, JNZ,
    JL, JLE, JG, JGE,
    JA, JAE, JB, JBE,

    SETE, SETNE, SETZ, SETNZ,
    SETL, SETLE, SETG, SETGE,
    SETA, SETAE, SETB, SETBE,

    MOVSS, MOVSD,
    MOVAPS, MOVAPD,
    UCOMISS, UCOMISD,
    ADDSS, ADDSD,
    SUBSS, SUBSD,
    MULSS, MULSD,
    DIVSS, DIVSD,
    ANDPS, ANDPD,
    ORPS, ORPD,
    XORPS, XORPD,

    CVTSS2SD, CVTSD2SS,
    CVTSI2SS, CVTSI2SD,
    CVTTSS2SI8, CVTTSS2SI16, CVTTSS2SI32, CVTTSS2SI64,
    CVTTSD2SI8, CVTTSD2SI16, CVTTSD2SI32, CVTTSD2SI64,
};

/// Recognized x64 physical registers.
enum Register : uint32_t {
    NO_REG = 0x0,

    RAX, RBX, RCX, RDX,
    RDI, RSI,
    R8, R9, R10, R11, 
    R12, R13, R14, R15,
    RSP, RBP,
    RIP,
    XMM0, XMM1, XMM2, XMM3,
    XMM4, XMM5, XMM6, XMM7,
    XMM8, XMM9, XMM10, XMM11,
    XMM12, XMM13, XMM14, XMM15,
};

/// Returns true if the opcode |opc| is considered a call instruction.
bool is_call_opcode(x64::Opcode opc);

/// Returns true if the opcode |opc| is considered a return instruction.
bool is_ret_opcode(x64::Opcode opc);

/// Returns true if the opcode |opc| is considered a move instruction.
bool is_move_opcode(x64::Opcode opc);

/// Returns true if the opcode |opc| is considered terminating.
///
/// For x64, terminating means any JMP, JCC, or RET64 opcode.
bool is_terminating_opcode(x64::Opcode opc);

/// Returns the register class of the physical register |reg|.
RegisterClass get_class(x64::Register reg);

/// Returns true if the physical register |reg| is considered callee-saved.
bool is_callee_saved(x64::Register reg);

/// Returns true if the physical register |reg| is considered caller-saved.
bool is_caller_saved(x64::Register reg);

/// Return the target registers for x64.
TargetRegisters get_registers();

/// Returns the string representation of the opcode |op|. This is used for
/// dumping purposes, and does not represent the recognized x64 assembly
/// equivelant
std::string to_string(x64::Opcode op);

/// Returns the string representation of the physical register |reg|, with
/// optional x64 subregister |subreg|. This is used for dumping purposes,
/// and does not represent the recognized x64 assembly equivelant.
std::string to_string(Register reg, uint16_t subreg = 8);

} // namespace x64

} // namespace spbe

#endif // SPBE_X64_H_

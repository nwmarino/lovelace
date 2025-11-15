#ifndef SPBE_X64_H_
#define SPBE_X64_H_

#include "../graph/Instruction.hpp"
#include "../graph/Local.hpp"
#include "../machine/MachRegister.hpp"
#include "../machine/MachObject.hpp"
#include "../machine/Rega.hpp"

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>

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

/// x64 Instruction selection pass over a SIIR function.
class X64InstSelection final {
    MachFunction* m_function;
    MachBasicBlock* m_insert = nullptr;
    const Target& m_target;

    /// Temporary mapping between bytecode virtual registers and machine 
    /// register ids.
    std::unordered_map<uint32_t, MachRegister> m_vregs = {};

    /// Mapping between function locals and stack offsets.
    std::unordered_map<const Local*, uint32_t> m_stack_indices = {};

    /// Comparison instructions which have been "deferred" until later. This
    /// is mainly used for comparisons whose only user is a conditional branch.
    std::vector<const Instruction*> m_deferred_cmps = {};

    /// Returns true if the comparison instruction |inst| has been deferred.
    bool is_deferred(const Instruction* inst) const {
        assert(inst->is_comparison() &&
            "cannot defer a non-comparison instruction!");
        return std::find(m_deferred_cmps.begin(), m_deferred_cmps.end(), inst) 
            != m_deferred_cmps.end();
    }

    /// Mark the comparison instruction |inst| as deferred.
    void defer(const Instruction* inst) {
        assert(!is_deferred(inst) && 
            "comparison instruction has already been deferred!");
        m_deferred_cmps.push_back(inst);
    }

    /// Returns or creates a virtual machine register equivelant for the 
    /// defining SIIR instruction |inst|. 
    MachRegister as_machine_reg(const Instruction* inst);

    MachRegister scratch(RegisterClass cls);

    /// Returns the expected x64 general-purpose subregister for a given type. 
    /// This function will always return 1, 2, 4, or 8.
    uint16_t get_subreg(const Type* ty) const;

    x64::Opcode get_move_op(const Type* ty) const;
    x64::Opcode get_cmp_op(const Type* ty) const;
    x64::Opcode get_add_op(const Type* ty) const;
    x64::Opcode get_sub_op(const Type* ty) const;
    x64::Opcode get_imul_op(const Type* ty) const;
    x64::Opcode get_mul_op(const Type* ty) const;
    x64::Opcode get_idiv_op(const Type* ty) const;
    x64::Opcode get_div_op(const Type* ty) const;
    x64::Opcode get_and_op(const Type* ty) const;
    x64::Opcode get_or_op(const Type* ty) const;
    x64::Opcode get_xor_op(const Type* ty) const;
    x64::Opcode get_shl_op(const Type* ty) const;
    x64::Opcode get_shr_op(const Type* ty) const;
    x64::Opcode get_sar_op(const Type* ty) const;
    x64::Opcode get_not_op(const Type* ty) const;
    x64::Opcode get_neg_op(const Type* ty) const;

    x64::Opcode get_jcc_op(spbe::Opcode opc) const;
    x64::Opcode get_setcc_op(spbe::Opcode opc) const;
    
    /// Returns a machine operand equivelant of treating |value| as a use.
    MachOperand as_operand(const Value* value);

    /// Returns a machine operand representing the location of a function call
    /// argument as per the SystemV ABI for x64.
    ///
    /// TODO: Split out depending on target ABI.
    MachOperand as_call_argument(const Value* value, uint32_t arg_idx) const;

    /// Emit a new machine instruction with opcode |op| and operand list |ops|.
    MachineInst& emit(x64::Opcode opc, 
                      const std::vector<MachOperand>& ops = {});

    MachineInst& emit_before_terms(x64::Opcode opc,
                                   const std::vector<MachOperand>& ops = {});

    /// Perform instruction selection on a single SIIR instruction.
    void select(const Instruction* inst);

    void select_constant(const Instruction* inst);
    void select_string_constant(const Instruction* inst);
    void select_load_store(const Instruction* inst);
    void select_access_ptr(const Instruction* inst);
    void select_select(const Instruction* inst);
    void select_branch_if(const Instruction* inst);
    void select_phi(const Instruction* inst);
    void select_return(const Instruction* inst);
    void select_call(const Instruction* inst);
    void select_add(const Instruction* inst);
    void select_sub(const Instruction* inst);
    void select_imul(const Instruction* inst);
    void select_idiv_irem(const Instruction* inst);
    void select_fmul_fdiv(const Instruction* inst);
    void select_bit_op(const Instruction* inst);
    void select_shift(const Instruction* inst);
    void select_not(const Instruction* inst);
    void select_neg(const Instruction* inst);
    void select_ext(const Instruction* inst);
    void select_trunc(const Instruction* inst);
    void select_int_to_fp_cvt(const Instruction* inst);
    void select_fp_to_int_cvt(const Instruction* inst);
    void select_ptr_to_int_cvt(const Instruction* inst);
    void select_int_to_ptr_cvt(const Instruction* inst);
    void select_type_reinterpret(const Instruction* inst);
    void select_comparison(const Instruction* inst);

public:
    X64InstSelection(MachFunction* function) 
        : m_function(function), m_target(function->get_target()) {}

    X64InstSelection(const X64InstSelection&) = delete;
    X64InstSelection& operator = (const X64InstSelection&) = delete;

    void run();
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

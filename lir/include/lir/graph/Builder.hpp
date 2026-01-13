//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_IR_BUILDER_H_
#define LOVELACE_IR_BUILDER_H_

#include "lir/graph/BasicBlock.hpp"
#include "lir/graph/Constant.hpp"
#include "lir/graph/Instruction.hpp"
#include "lir/graph/Type.hpp"

#include <cassert>

namespace lir {

class Builder final {
public:
    /// The different insertion modes for new instructions.
    enum class InsertMode : uint8_t {
        Prepend, Append,
    };

private:
    /// The parent graph, used for type and constant pooling.
    CFG& m_cfg;

    /// The current basic block insertion point.
    BasicBlock* m_insert = nullptr;

    /// The mode of insertion for new instructions into the insertion block.
    InsertMode m_mode = InsertMode::Append;

public:
    Builder(CFG& cfg) : m_cfg(cfg) {}

    void set_insert(BasicBlock* block) { m_insert = block; }
    void clear_insert() { m_insert = nullptr; }
    const BasicBlock* get_insert() const { return m_insert; }
    BasicBlock* get_insert() { return m_insert; }

    void set_mode(InsertMode mode) { m_mode = mode; }
    InsertMode get_mode() const { return m_mode; }

    /// Insert |inst| to the current insertion point, if it is set.
    void insert(Instruction* inst);

    /// Build and insert a new instruction of the given |op|, possible def id 
    /// |def|, and operand list |ops|. Returns the newly created instruction.
    Instruction* insert(Mnemonic op, uint32_t def = 0, Type* type = nullptr, 
                        const std::vector<Value*>& ops = {}, 
                        Descriptor desc = {});

    /// Create a new constant string instruction defining |string| as a value.
    Instruction* build_string(String* string);

    /// Create a new memory load that reads a value typed with |type| from
    /// |source| with the given |alignment|.
    Instruction* build_load(Type* type, Value* source, uint16_t alignment);

    /// Create a new memory store that writes |value| to |dest| with the given 
    /// |alignment|.
    Instruction* build_store(Value* value, Value* dest, uint16_t alignment);

    /// Create a new pointer walk that steps through the list of integer 
    /// |indices| to access the base pointer |source|. The |type| argument
    /// indicates the type of the resulting value.
    Instruction* build_pwalk(Type* type, Value* source, 
                             const std::vector<Value*>& indices);

    /// Create a new conditional jump that chooses |true_dest| or
    /// |false_dest| based on the result of |cond|.
    Instruction* build_jif(Value* cond, BasicBlock* true_dest,
                           const std::vector<Value*>& true_args,
                           BasicBlock* false_dest,
                           const std::vector<Value*>& false_args);

    /// Build a new jump (JMP) instruction to the given |dest| block. 
    /// Optionally, provide a list of |args| to the basic block, if the block
    /// requires any.
    Instruction* build_jmp(BasicBlock* dest, const std::vector<Value*>& args = {});

    /// Build a new return (RET) instruction that returns the given |value|.
    /// If |value| is null, then the instruction is a void return.
    Instruction* build_ret(Value* value = nullptr);

    /// Build a new abort (ABORT) instruction that stops execution.
    Instruction* build_abort();

    /// Create a new unreachable instruction, used as a pseudo-terminator to
    /// mark places that should be unreachable by control flow.
    Instruction* build_unreachable();

    /// Create a new call instruction to |callee| with the given |args|.
    /// The |type| parameter designates the type signature of the argument list
    /// and anticipated return type.
    Instruction* build_call(FunctionType* type, Value* callee, 
                            const std::vector<Value*>& args = {});

    /// Create a new integer equality comparison.
    Instruction* build_cmp_ieq(Value* lhs, Value* rhs);

    /// Create a new integer inequality comparison.
    Instruction* build_cmp_ine(Value* lhs, Value* rhs);

    /// Create a new floating point ordered equality comparison.
    Instruction* build_cmp_oeq(Value* lhs, Value* rhs);

    /// Create a new floating point ordered inequality comparison.
    Instruction* build_cmp_one(Value* lhs, Value* rhs);

    /// Create a new signed integer less than comparison.
    Instruction* build_cmp_slt(Value* lhs, Value* rhs);

    /// Create a new signed integer less than equals comparison.
    Instruction* build_cmp_sle(Value* lhs, Value* rhs);

    /// Create a new signed integer greater than comparison.
    Instruction* build_cmp_sgt(Value* lhs, Value* rhs);

    /// Create a new signed integer greater than equals comparison.
    Instruction* build_cmp_sge(Value* lhs, Value* rhs);

    /// Create a new unsigned integer less than comparison.
    Instruction* build_cmp_ult(Value* lhs, Value* rhs);

    /// Create a new unsigned integer less than equals comparison.
    Instruction* build_cmp_ule(Value* lhs, Value* rhs);

    /// Create a new unsigned integer greater than comparison.
    Instruction* build_cmp_ugt(Value* lhs, Value* rhs);

    /// Create a new unsigned integer greater than equals comparison.
    Instruction* build_cmp_uge(Value* lhs, Value* rhs);

    /// Create a new floating point ordered less than comparison.
    Instruction* build_cmp_olt(Value* lhs, Value* rhs);

    /// Create a new floating point ordered less than equals comparison.
    Instruction* build_cmp_ole(Value* lhs, Value* rhs);

    /// Create a new floating point ordered greater than comparison.
    Instruction* build_cmp_ogt(Value* lhs, Value* rhs);

    /// Create a new floating point ordered greater than equals comparison.
    Instruction* build_cmp_oge(Value* lhs, Value* rhs);

    /// Create a new integer addition instruction.
    Instruction* build_iadd(Value* lhs, Value* rhs);

    /// Create a new integer subtraction instruction.
    Instruction* build_isub(Value* lhs, Value* rhs);

    /// Create a new integer multiplication instruction.
    Instruction* build_imul(Value* lhs, Value* rhs);

    /// Create a new signed integer division instruction.
    Instruction* build_sdiv(Value* lhs, Value* rhs);

    /// Create a new unsigned integer division instruction.
    Instruction* build_udiv(Value* lhs, Value* rhs);

    /// Create a new signed integer modulo instruction.
    Instruction* build_smod(Value* lhs, Value* rhs);

    /// Create a new unsigned integer modulo instruction.
    Instruction* build_umod(Value* lhs, Value* rhs);

    /// Create a new floating point addition instruction.
    Instruction* build_fadd(Value* lhs, Value* rhs);

    /// Create a new floating point subtraction instruction.
    Instruction* build_fsub(Value* lhs, Value* rhs);

    /// Create a new floating point multiplication instruction.
    Instruction* build_fmul(Value* lhs, Value* rhs);

    /// Create a new floating point division instruction.
    Instruction* build_fdiv(Value* lhs, Value* rhs);

    /// Create a new bitwise and instruction.
    Instruction* build_and(Value* lhs, Value* rhs);

    /// Create a new bitwise or instruction.
    Instruction* build_or(Value* lhs, Value* rhs);

    /// Create a new bitwise xor instruction.
    Instruction* build_xor(Value* lhs, Value* rhs);

    /// Create a new bitwise logical left shift instruction.
    Instruction* build_shl(Value* lhs, Value* rhs);

    /// Create a new bitwise logical right shift instruction.
    Instruction* build_shr(Value* lhs, Value* rhs);

    /// Create a new bitwise arithmetic right shift instruction.
    Instruction* build_sar(Value* lhs, Value* rhs);

    /// Create a new bitwise not instruction.
    Instruction* build_not(Value* value);

    /// Create a new integer negation instruction.
    Instruction* build_ineg(Value* value);

    /// Create a new floating point negation instruction.
    Instruction* build_fneg(Value* value);

    /// Create a new integer sign extension instruction. The instruction will
    /// extend |value| to the given |type|.
    Instruction* build_sext(Type* type, Value* value);

    /// Create a new integer zero extension instruction. The instruction will
    /// extend |value| to the given |type|.
    Instruction* build_zext(Type* type, Value* value);

    /// Create a new floating point extension instruction. The instruction will
    /// extend |value| to the given |type|.
    Instruction* build_fext(Type* type, Value* value);

    /// Create a new integer truncation instruction. The instruction will
    /// truncate |value| to the given |type|.
    Instruction* build_itrunc(Type* type, Value* value);

    /// Create a new floating point truncation instruction. The instruction 
    /// will truncate |value| to the given |type|.
    Instruction* build_ftrunc(Type* type, Value* value);

    /// Create a new signed integer to floating point conversion instruction.
    Instruction* build_s2f(Type* type, Value* value);

    /// Create a new unsigned integer to floating point conversion instruction.
    Instruction* build_u2f(Type* type, Value* value);

    /// Create a new floating point to signed integer cast instruction.
    Instruction* build_f2s(Type* type, Value* value);

    /// Build a new floating point to unsigned integer cast instruction.
    Instruction* build_f2u(Type* type, Value* value);
    
    /// Create a new pointer to integer conversion instruction. The instruction
    /// will convert |value| to the given |type|.
    Instruction* build_p2i(Type* type, Value* value);

    /// Create a new integer to pointer conversion instruction. The instruction
    /// will convert |value| to the given |type|.
    Instruction* build_i2p(Type* type, Value* value);

    /// Create a new pointer reinterpretation. The instruction will convert 
    /// |value| to the given |type|.
    Instruction* build_reint(Type* type, Value* value);
};

} // namespace lir

#endif // LOVELACE_IR_BUILDER_H_

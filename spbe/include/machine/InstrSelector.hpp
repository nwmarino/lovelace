#ifndef SPBE_INSTR_SELECTOR_H_
#define SPBE_INSTR_SELECTOR_H_

#include "MachFunction.hpp"
#include "../graph/Instruction.hpp"

namespace spbe {

class InstrSelector {
protected:
    MachFunction& m_function;

    virtual void select_constant(const Instruction* inst) {}

    virtual void select_string_constant(const Instruction* inst) {}

    virtual void select_load_store(const Instruction* inst) {}

    virtual void select_access_ptr(const Instruction* inst) {}

    virtual void select_select(const Instruction* inst) {}

    virtual void select_branch_if(const Instruction* inst) {}

    virtual void select_phi(const Instruction* inst) {}

    virtual void select_return(const Instruction* inst) {}

    virtual void select_call(const Instruction* inst) {}

    virtual void select_add(const Instruction* inst) {}

    virtual void select_sub(const Instruction* inst) {}

    virtual void select_imul(const Instruction* inst) {}

    virtual void select_idiv_irem(const Instruction* inst) {}

    virtual void select_fmul_fdiv(const Instruction* inst) {}

    virtual void select_bit_op(const Instruction* inst) {}

    virtual void select_shift(const Instruction* inst) {}

    virtual void select_not(const Instruction* inst) {}

    virtual void select_neg(const Instruction* inst) {}

    virtual void select_ext(const Instruction* inst) {}

    virtual void select_trunc(const Instruction* inst) {}

    virtual void select_int_to_fp_cvt(const Instruction* inst) {}

    virtual void select_fp_to_int_cvt(const Instruction* inst) {}

    virtual void select_ptr_to_int_cvt(const Instruction* inst) {}

    virtual void select_int_to_ptr_cvt(const Instruction* inst) {}

    virtual void select_type_reinterpret(const Instruction* inst) {}

    virtual void select_comparison(const Instruction* inst) {}

public:
    InstrSelector(MachFunction& MF) : m_function(MF) {}

    InstrSelector(const InstrSelector&) = delete;
    InstrSelector& operator = (const InstrSelector&) = delete;

    virtual ~InstrSelector() = default;

    virtual void run() = 0;
};

} // namespace spbe

#endif // SPBE_INSTR_SELECTOR_H_

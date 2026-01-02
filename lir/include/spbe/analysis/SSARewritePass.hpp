//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SPBE_SSA_REWRITE_PASS_H_
#define SPBE_SSA_REWRITE_PASS_H_

#include "spbe/analysis/Pass.hpp"
#include "spbe/graph/BasicBlock.hpp"
#include "spbe/graph/InstrBuilder.hpp"
#include "spbe/graph/Instruction.hpp"
#include "spbe/graph/Local.hpp"

#include <unordered_map>
#include <vector>

namespace spbe {

class Instruction;
class Local;

/// Function-based pass to rewrite memory load/store operations into true SSA
/// instructions so that optimizations can be properly ran over locals.
///
/// This pass implements some of the algorithms outlined by Braun et al.
/// See: https://link.springer.com/chapter/10.1007/978-3-642-37051-9_6
class SSARewritePass final : public Pass {
    using BlockDefs = std::unordered_map<Local*,
        std::unordered_map<BasicBlock*, Value*>>;

    InstrBuilder m_builder;

    Local* m_local = nullptr;

    std::unordered_map<BasicBlock*, Value*> m_current_def = {};

    std::unordered_map<BasicBlock*, std::unordered_map<Local*, 
        std::vector<Instruction*>>> m_incomplete_phis;

    /// A list of instructions to remove after the current process.
    std::vector<Instruction*> m_to_remove = {};

    std::vector<BasicBlock*> m_visited = {};

    std::vector<BasicBlock*> m_sealed = {};

    /// Process a function in the target graph.
    void process(Function* fn);

    void promote_local(Function* fn, Local* local);

    /// Register a variable write (def).
    void write_variable(BasicBlock* blk, Value* value);

    // Read the latest definition of |var| based on current block |blk|.
    Value* read_variable(BasicBlock* blk);
    Value* read_variable_recursive(BasicBlock* blk);

    Value* add_phi_operands(Instruction* phi);

    /// Attempt to remove a phi instruction which could be considered trivial,
    /// i.e. merges less than two unique values. Returns the result of the 
    /// operation; the phi instruction or the distinguishable operand.
    Value* try_remove_trivial_phi(Instruction* phi);

    bool visited(BasicBlock* blk);

    bool is_sealed(BasicBlock* blk);

    void seal_block(BasicBlock* blk);

public:
    SSARewritePass(CFG& cfg) : Pass(cfg), m_builder(cfg) {}

    SSARewritePass(const SSARewritePass&) = delete;
    SSARewritePass& operator = (const SSARewritePass&) = delete;

    void run() override;
};

} // namespace spbe

#endif // SPBE_SSA_REWRITE_PASS_H_

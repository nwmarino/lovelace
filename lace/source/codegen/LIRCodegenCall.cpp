//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lace/codegen/Codegen.hpp"
#include "lace/tree/Defn.hpp"
#include "lir/graph/Type.hpp"

using namespace lace;

void Codegen::visit(CallExpr& node) {
    m_vctx = LValue;
    node.get_callee()->accept(*this);
    assert(m_temp && "callee does not produce a value!");

    lir::Value* callee = m_temp;
    std::vector<lir::Value*> args(node.num_args(), nullptr);

    for (uint32_t i = 0, e = node.num_args(); i < e; ++i) {
        m_vctx = RValue;
        node.get_arg(i)->accept(*this);
        assert(m_temp && "argument does not produce a value!");
        args[i] = m_temp;
    }

    m_temp = m_builder.build_call(
        static_cast<lir::FunctionType*>(callee->get_type()), 
        callee, 
        args);
}

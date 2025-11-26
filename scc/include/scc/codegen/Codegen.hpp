//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SCC_CODEGEN_H_
#define SCC_CODEGEN_H_

//
// This header file declares a class which implements the visitor design 
// pattern over the abstract syntax tree for generating SPBE-IR code.
//

#include "scc/ast/Visitor.hpp"

#include "graph/BasicBlock.hpp"

namespace scc {

class Codegen final : public Visitor {

};

} // namespace scc

#endif // SCC_CODEGEN_H_

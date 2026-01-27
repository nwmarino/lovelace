# Changelog

## General
- Remove LLVM backend.
- Extend some standard library components e.g. runtime, strings, files.
- Remove some old statim code.
- Fixed bugs in runtime assembly.

## lace
- Implement code generation for moving around non-scalars in assignments,
function calls/returns, and local variable definitions.
- Fixed some formatting bugs in the diagnostics logger.
- Add phase timing to verbose outputs.

## LIR
- Move `BasicBlock::Arg` and `Function::Arg` out to standalone classes named
`BlockArgument` and `FunctionArgument`, respectively.
- Add traits to function arguments.
- Add weak patch for non-i64 indices in pwalk instructions to sign extend into
full x64 registers.
- Update old machine printer pass.

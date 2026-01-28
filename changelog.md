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
- Redesigned instruction API to use separate classes per instruction type.
- Added back the `Phi` node.
- Added the `extract` instruction for constant index field access of aggregates.
- Split the `pwalk` instruction into separate `access` and `index` instructions
for addressed structure access and pointer arithmetic, respectively.
- Move `Function::Arg` to a separate `Parameter` class.
- Add weak patch for non-i64 indices in pointer arithmetic instructions to sign 
extend into full x64 registers.
- Update old machine printer pass.
- Add multiple result types to functions.

Runes
- Assert
- Abort
- Syscall
- Unreachable

- Per-field destruction
- Allocator
- Format strings
- Inline assembly
- Unnamed Enums
- Unions
- For Loops
- Ternary '?' operator
- CTFE
- Build system
- Enum flags
- Namespacing
- DWARF symbols
- Type polymorphism
- Slices
- Defer statement

-- todo

do isel for pointer walk

move BasicBlock::Arg out of BasicBlock and into a standalone BlockArg class, 
same with function args

add aggregate initializers

restructure LIR code generation to be inline with the LLVM code generator

add comments to machine representation, that are printed from lir

add line debugging symbols

even though refs can be used as lvalues, make sure a reference to constants 
like enum variants isnt used as an lvalue.

thread-safe logger

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

add comments to machine representation, that are printed from lir

even though refs can be used as lvalues, make sure a reference to constants 
like enum variants isnt an lvalue.

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

reiterate graph builder

move Block::Arg out into a BlockArg, same with function args

even though refs can be used as lvalues, make sure a reference to constants 
like enum variants isnt an lvalue.

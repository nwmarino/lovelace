# lovelace

lovelace is a recreational systems language that takes major inspiration from
the philosophies of most of your favorite langs. It is, however, most similar
in nature to C by way of what is possible out of the box.

The project is split down the middle, with an x86-64 backend that is 
theoretically modular enough to be carved out and made to work with other 
frontends. 

### lace

lace is the frontend for the language, and in particular, handles the process
of turning source code into a "valid" syntax tree, and later into a target 
agnostic intermediate representation (LIR).

### LIR

The lovelace intermediate representation (LIR) handles target-specific jobs 
like ABI control, register allocation, and SSA-based optimizations. The IR uses
basic block arguments instead of phi nodes, for the reason that I like being
different and more importantly, it becomes a little bit easier to write an 
interpreter down the road. The IR can reach true SSA form by a rewrite pass 
based on algorithms described by 
[Braun et al.](https://link.springer.com/chapter/10.1007/978-3-642-37051-9_6)
Barring that one difference, since the IR is based on a control-flow graph, it
can cleanly translate into similar IRs like LLVM's.

Without getting too theoretical, the advantages of SSA form mean most 
operations produce an immutable value by using mostly other immutable values,
which gives way to a very clean use-def chain representation. This means 
optimization and analysis passes can easily model dead code, register naming,
and propogations. 

## Building

All components of the project use and suggest at least CMake 4.0 to build.

Both the frontend and backend depend only on Boost and Google Test, which are
available on most distro package managers via `boost` and `gtest`.

Most of the compiler is written in C++20, with the main features utilized being
ranges, jthreads interface, format strings, 

```sh
cd lovelace/
cmake -S . -B build/
cmake --build build/

# for tests, optionally
ctest --test-dir build/
```

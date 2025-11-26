# statim ('stat·​im) *"immediately"*

statim is a compiler toolchain that consists of both a custom backend intended
for systems-related programming languages, and two frontends for C (scc) and a 
custom language (stmc).

## scc

scc is the C compiler frontend that uses a standard version of the portable
backend. The frontend acts as moreso as a proof of concept for the backend than 
a compiler meant for release builds. It implements most major C99 language 
rules and features, with some exceptions.

## stmc

stmc is the frontend for an expiremental, general-purpose language with some 
systems-esque features.

## spbe

The portable backend leverages an intermediate representation (IR) capable of 
Static-Single Assignment (SSA) form that is based on a control-flow graph;
like other IRs you may be familiar with. The stmc frontend uses a custom 
fork of this backend to implement special intrinsics related to the language. 
The standard version can be translated to LLVM IR if it is desired for more
aggressive optimizations i.e. for release builds.

## Building

All components of the project use and suggest CMake >= 4.0 to build.

```sh
cd statim/
cmake -S . -B build/
cmake --build build/

# for tests, optionally
ctest --test-dir build/
```

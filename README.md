# statim ('stat·​im) *"immediately"*

statim is a compiler toolchain that consists of both a custom backend intended
for systems-related programming languages, and two frontends for C and a custom 
language.

## scc

scc is the C compiler frontend that uses a standard version of the portable
backend. It implements most major ANSI C language rules, with some exceptions.

## stmc

stmc is the frontend for an expiremental, general-purpose language with some 
systems-esque features.

## spbe

The portable backend leverages an intermediate representation (IR) capable of 
Static-Single Assignment (SSA) form that is based on a control-flow graph,
similar to other IRs you may be familiar with. The stmc frontend uses a custom 
fork of this backend to implement special intrinsics related to the language. 
The standard version can be translated to LLVM IR if it is desired for more
aggressive optimizations i.e. for release builds.

## Building

All components of the project use and require CMake >= 4.0 to build.

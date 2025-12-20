# statim ('stat·​im) *"immediately"*

statim is a compiler toolchain that consists of both a custom backend intended
for low level programming languages, and a frontend for an expiremental, C-like 
systems programming language (stmc).

## stmc

stmc is the frontend for an expiremental language with largely systems-esque 
features.

## spbe

The portable backend leverages an intermediate representation (IR) capable of 
Static-Single Assignment (SSA) form that is based on a control-flow graph,
like other IRs you may be familiar with.

## Building

All components of the project use and suggest CMake >= 4.0 to build.

```sh
cd statim/
cmake -S . -B build/
cmake --build build/

# for tests, optionally
ctest --test-dir build/
```

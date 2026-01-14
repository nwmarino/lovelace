# Changelog

## General

- Changed project name to lovelace.
- Changed middle-end IR name to LIR.
- Added sample programs featuring basic language constructs.
- Updated most classes with defined destructors to follow rule-of-5.
- Changed most filenames to use a PascalCase naming convention.
- Removed inline assembly.
- Updated runtime library to use a set of assembly functions, for now.
- Modularized the project CMake structure.
- Some bug fixes all around.

## Frontend

- Modified how projects with multiple files link eachother in.
- Simplified the dependency resolution process.
- Split symbol analysis into a separate name analysis pass, to replace deferred types.
- Added the `ThreadPool` class for an easier multi-threading interface with jthreads.
- Added multi-threading support for parsing.
- Remove some unused tokens.
- Added LLVM as a native backend rather than an IR translation pass.

## IR

- Switch instructions "opcode" to a more fitting "mnemonic".
- Simplify instruction API.
- Changed the IR *ap* operation to a GEP-like *pw* pointer walk.
- Replaced phi nodes with a cleaner basic block argument approach.
- Remove some unused instructions and value types e.g. inline asm, aggregates.

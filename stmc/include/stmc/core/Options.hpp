//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef STATIM_OPTIONS_H_
#define STATIM_OPTIONS_H_

#include <cstdint>
#include <string>

namespace stm {

using std::string;

/// Potential options and diagnostics for the compiler.
struct Options final {
    enum OptLevel : uint32_t {
        None,       //< No optimizations, i.e. -O0
        Default,    //< Basic optimizations, i.e. -O1
        Aggressive, //< More aggressive optimizations, i.e. -O2
        Space,      //< Optimizations for output file size, i.e. -Os
    };

    enum StopPoint : uint32_t {
        Assembly,   //< To stop after compilation, i.e. -S
        Object,     //< To stop after assembly, i.e. -c 
    };

    string output;      //< (-o) The name of the output file.
    OptLevel optLevel;  //< (-O0/-O1/-O2/-Os) The optimization level.
    StopPoint stop;     //< (-S/-c) The compilation process stopping point.
    bool debug;         //< (-g) If debugging symbols should be added.
    bool time;          //< (-t) If pipeline stages should be timed.

    bool printTree;     //< (-ast) If the AST should be printed.
    bool printSPBE;     //< (-spbe) If the SPBE-IR should be printed.

    Options() = default;
};

} // namespace stm

#endif // STATIM_OPTIONS_H_

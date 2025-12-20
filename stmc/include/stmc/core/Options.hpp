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

/// Set of potential options and diagnostics for the compiler.
struct Options final {
    /// Possible optimization levels.
    enum class OptLevel : uint32_t {
        None,       //< No optimizations, i.e. -O0
        Less,       //< Basic optimizations, i.e. -O1
        Default,    //< The standard set of optimizations, i.e. -O2
        Aggressive, //< More aggressive optimizations, i.e. -O3
        Space,      //< Optimizations for output file size, i.e. -Os
    };

    /// Possible stopping points during the compilation process.
    enum class StopPoint : uint32_t {
        None,       //< No stopping point, default.
        Assembly,   //< To stop after compilation, i.e. -S
        Object,     //< To stop after assembly, i.e. -c
    };

    string output;      //< (-o) The name of the output file.
    OptLevel opt_level; //< (-O0/-O1/-O2/-Os) The optimization level.
    StopPoint stop;     //< (-S/-c) The compilation process stopping point.
    bool debug;         //< (-g) If debugging symbols should be added.
    bool time;          //< (-t) If pipeline stages should be timed.
    bool verbose;       //< (-b) If extra notes should be logged.
    bool version;       //< (-v) If the version should be printed.

    // Dumping-related options.
    bool print_tree;    //< (-ast) If the AST should be printed.
    bool print_spbe;    //< (-spbe) If the SPBE-IR should be printed.
};

} // namespace stm

#endif // STATIM_OPTIONS_H_

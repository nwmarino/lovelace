//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_OPTIONS_H_
#define LOVELACE_OPTIONS_H_

//
//  This header file defines the Options structure, which contains various
//  options used by driver code to decide the behavior of the compilation
//  process.
//

#include <cstdint>
#include <string>

namespace lace {

struct Options final {
    /// The possible optimization levels.
    enum class OptLevel : uint32_t {
        None,       //< No optimizations.
        Few,        //< Basic optimizations.
        Default,    //< The standard set of optimizations.
        Many,       //< More aggressive optimizations.
        Space,      //< Optimizations for minimizing binary size.
    };

    std::string output; //< (-o) The name of the output file.
    OptLevel opt;       //< (-O0/-O1/-O2/-O3/-Os) The optimization level.
    bool debug;         //< (-g) If debugging symbols should be added.
    bool time;          //< (-t) If pipeline stages should be timed.
    bool verbose;       //< (-b) If extra notes should be logged.
    bool version;       //< (-v) If the version should be printed.

    bool print_tree;    //< (-ast) If the AST should be printed.
    bool print_spbe;    //< (-spbe) If the SPBE-IR should be printed.
};

} // namespace lace

#endif // LOVELACE_OPTIONS_H_

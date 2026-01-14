//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#ifndef LOVELACE_SOURCE_LOCATION_H_
#define LOVELACE_SOURCE_LOCATION_H_

//
//  This header file defines the SourceLocation type, used in the frontend to
//  reference particular locations in source code given to the compiler.
//

#include <cstdint>

namespace lace {

/// Represents a location in source code.
struct SourceLocation final {
    uint16_t line, col;

    SourceLocation(uint16_t line = 1, uint16_t col = 1) 
      : line(line), col(col) {}

    SourceLocation(const SourceLocation& other) {
        line = other.line;
        col = other.col;
    }

    void operator=(const SourceLocation& other) {
        line = other.line;
        col = other.col;
    }

    bool operator==(const SourceLocation& other) const {
        return line == other.line && col == other.col;
    }

    bool operator<(const SourceLocation& other) const {
        return line < other.line && col < other.col;
    }

    bool operator>(const SourceLocation& other) const {
        return line > other.line && col > other.col;
    }
};

} // namespace lace

#endif // LOVELACE_SOURCE_LOCATION_H_

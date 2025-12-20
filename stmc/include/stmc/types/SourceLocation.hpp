//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef STATIM_SOURCE_LOCATION_H_
#define STATIM_SOURCE_LOCATION_H_

#include <cstdint>
#include <string>

namespace stm {

using std::string;

/// Representation of a location in source code.
struct SourceLocation final {
    /// The line and column in source code this location refers to.
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

    bool operator!=(const SourceLocation& other) const {
        return line != other.line || col != other.col;
    }

    bool operator<(const SourceLocation& other) const {
        return line < other.line && col < other.col;
    }

    bool operator>(const SourceLocation& other) const {
        return line > other.line && col > other.col;
    }
};

} // namespace stm

#endif // STATIM_SOURCE_LOCATION_H_

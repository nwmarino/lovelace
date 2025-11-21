//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SCC_SPAN_H_
#define SCC_SPAN_H_

//
// This header file declares basic structures to represent the locations of
// symbols and other structures in source code.
//

#include <cstdint>
#include <string>

namespace scc {

/// Represents a location in source code.
struct SourceLocation final {
    /// The path of the file that holds the corresponding source.
    std::string path;

    /// The line this location refers to.
    uint32_t line;

    /// The column this location refers to.
    uint32_t column;
};

/// Represents a span of source code between two locations.
struct Span final {
    /// The start or beginning location.
    SourceLocation begin;

    /// The final or ending location.
    SourceLocation end;
};

} // namespace scc

#endif // SCC_SPAN_H_

//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SCC_SOURCE_SPAN_H_
#define SCC_SOURCE_SPAN_H_

//
// This header file declares basic structures to represent the locations of
// symbols and other structures in source code.
//

#include <cstdint>
#include <string>

namespace scc {

using std::string;

/// Represents a location in source code.
struct SourceLocation final {
    /// The path of the file that holds the corresponding source.
    string path;

    /// The line this location refers to.
    uint32_t line = 1;

    /// The column this location refers to.
    uint32_t column = 1;

    SourceLocation() = default;

    SourceLocation(const string& path) : path(path), line(1), column(1) {}

    SourceLocation(const string& path, uint32_t line, uint32_t column)
        : path(path), line(line), column(column) {}
};

/// Represents a span of source code between two locations.
struct SourceSpan final {
    /// The starting location.
    SourceLocation start;

    /// The ending location.
    SourceLocation end;

    SourceSpan() = default;

    SourceSpan(const SourceLocation& loc) : start(loc), end(loc) {}

    SourceSpan(const SourceLocation& s, const SourceLocation& e) 
        : start(s), end(e) {}
};

} // namespace scc

#endif // SCC_SOURCE_SPAN_H_

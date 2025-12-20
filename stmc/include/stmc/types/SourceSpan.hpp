//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef STATIM_SOURCE_SPAN_H_
#define STATIM_SOURCE_SPAN_H_

#include "stmc/types/SourceLocation.hpp"

namespace stm {

/// Representation of a span of source code between two locations.
struct SourceSpan final {
    /// The start and end locations in source code of this span.
    SourceLocation start, end;

    SourceSpan() = default;

    SourceSpan(SourceLocation loc)
        : start(loc), end(loc) {}

    SourceSpan(SourceLocation start, SourceLocation end) 
        : start(start), end(end) {}

    SourceSpan(const SourceSpan& other) {
        start = other.start;
        end = other.end;
    }

    void operator=(const SourceSpan& other) {
        start = other.start;
        end = other.end;
    }

    bool operator==(const SourceSpan& other) const {
        return start == other.start && end == other.end;
    }

    bool operator!=(const SourceSpan& other) const {
        return start != other.start || end != other.end;
    }

    bool operator<(const SourceSpan& other) const {
        return start < other.start && end < other.end;
    }

    bool operator>(const SourceSpan& other) const {
        return start > other.start && end > other.end;
    }
};

} // namespace stm

#endif // STATIM_SOURCE_SPAN_H_

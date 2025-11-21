#ifndef SCC_SPAN_H_
#define SCC_SPAN_H_

#include <cstdint>
#include <string>

namespace scc {

/// Represents a location in source code.
struct SourceLocation final {
    std::string path;
    uint32_t line;
    uint32_t column;
};

/// Represents a span of source code between two locations.
struct Span final {
    SourceLocation begin;
    SourceLocation end;
};

} // namespace scc

#endif // SCC_SPAN_H_

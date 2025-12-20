//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef STATIM_DIAGNOSTICS_H_
#define STATIM_DIAGNOSTICS_H_

#include "stmc/types/SourceLocation.hpp"
#include "stmc/types/SourceSpan.hpp"

#include <iostream>
#include <ostream>
#include <string>

namespace stm {

using std::ostream;
using std::string;

/// A diagnostics logger for different components of the frontend to display
/// informative or warning messages about input or compiler state.
class Diagnostics final {
private:
    /// The output stream where messages are written.
    ostream* m_output;

    /// The path to the file to read in source code from for diagnostics.
    ///
    /// If left unset, any attempts to log source code locations will fail
    /// silently.
    string m_path = "";

    /// Whether to use ANSI color codes in log messages. 
    /// 
    /// This is determined by the capabilities of the current output stream.
    bool m_color;

    /// Whether to honor requests to read in source code.
    ///
    /// If set to false, then attempts to read in locations or spans of source
    /// code will fail silently.
    bool m_read;

    void print_source(SourceSpan span) const;

public:
    Diagnostics(ostream* os = &std::cerr, bool read = true);

    const ostream* get_output_stream() const { return m_output; }
    ostream* get_output_stream() { return m_output; }

    void set_output_stream(ostream* os);

    const string& get_path() const { return m_path; }
    string& get_path() { return m_path; }

    void set_path(const string& path) { m_path = path; }

    void clear_path() { m_path.clear(); }

    /// Log a basic message \p msg without any flags or metadata.
    void log(const string& msg);

    /// Log a note message \p msg.
    void note(const string& msg);

    /// Log a note message \p msg at source location \p loc.
    void note(const string& msg, SourceLocation loc);

    /// Log a note message \p msg at source span \p span.
    void note(const string& msg, SourceSpan span);

    /// Log a warning message \p msg.
    void warn(const string& msg);

    /// Log a warning message \p msg at source location \p loc.
    void warn(const string& msg, SourceLocation loc);

    /// Log a warning message \p msg at source span \p span.
    void warn(const string& msg, SourceSpan span);

    /// Log a fatal error message \p msg.
    void fatal(const string& msg);

    /// Log a fatal error message \p msg at source location \p loc.
    void fatal(const string& msg, SourceLocation loc);

    /// Log a fatal error message \p msg at source span \p span.
    void fatal(const string& msg, SourceSpan span);
};

} // namespace stm

#endif // STATIM_DIAGNOSTICS_H_

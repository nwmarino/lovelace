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
class Diagnostics {
private:
    /// The output stream where messages are written.
    ostream* m_output = &std::cerr;

    /// The path to the file to read in source code from for diagnostics.
    ///
    /// If left unset, any attempts to log source code locations will fail
    /// silently.
    string m_path = "";

    /// Whether to use ANSI color codes in log messages. This is determined
    /// by the capabilities of the output stream \c s_output.
    bool m_color = true;

public:
    Diagnostics() = default;
    
    Diagnostics(ostream& os) : m_output(&os) {}

    Diagnostics(const Diagnostics&) = delete;
    Diagnostics& operator = (const Diagnostics&) = delete;

    ~Diagnostics() = default;

    /// Returns the output stream this logger is using.
    ostream* get_output_stream() const { return m_output; }

    /// Set the output stream this logger is using to \p os.
    void set_output_stream(ostream& os) { m_output = &os; }

    /// Returns the path to read in source code from.
    const string& get_path() const { return m_path; }

    /// Set the file path to read in source from to \p path.
    void set_path(const string& path) { m_path = path; }

    /// Clear the file path to read in source from for this logger.
    void clear_path() { m_path.clear(); }

    /// Log an informative message \p msg.
    void info(const string& msg);

    /// Log an informative message \p msg with a reference to a location in
    /// source \p loc.
    void info(const string& msg, const SourceLocation& loc);

    /// Log an informative message \p msg with a reference to a span of source
    /// code \p span.
    void info(const string& msg, const SourceSpan& span);

    /// Log a warning message \p msg.
    void warn(const string& msg);

    /// Log a warning message \p msg with a reference to a location in source
    /// \p loc.
    void warn(const string& msg, const SourceLocation& loc);

    /// Log a warning message \p msg with a reference to a span of source 
    /// \p span.
    void warn(const string& msg, const SourceSpan& span);

    /// Log an error message \p msg.
    void error(const string& msg);

    /// Log an error message \p msg with a reference to a location in source 
    /// \p loc.
    void error(const string& msg, const SourceLocation& loc);

    /// Log an error message \p msg with a reference to a span of source code
    /// \p span.
    void error(const string& msg, const SourceSpan& span);
};

} // namespace stm

#endif // STATIM_DIAGNOSTICS_H_

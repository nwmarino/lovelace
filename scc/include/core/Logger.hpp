//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SCC_LOGGER_H_
#define SCC_LOGGER_H_

//
// This header file declares a static command line error logger accessible by 
// most components of the C frontend.
//

#include "SourceSpan.hpp"

#include <iostream>
#include <ostream>
#include <string>

namespace scc {

using std::ostream;
using std::string;

class Logger {
private:
    /// The output stream where log messages are written. This must be
    /// initialized by driver code using the static \c init function.
    static ostream* s_output;

    /// Whether to use ANSI color codes in log messages. This is determined
    /// by the capabilities of the output stream \c s_output.
    static bool s_color;

    /// Fetch and log a span o fsource \p span from the file it represents.
    static void log_source(const SourceSpan& span);

public:
    Logger() = delete;

    /// Initialize the loggler with output stream \p output. If no output 
    /// stream is provided, but the function is still called, then messages 
    /// will be automatically directed to cerr.
    static void init(ostream& output = std::cerr);

    /// Log an informative message \p msg to the output stream.
    static void info(const string& msg);

    /// Log an informative message \p msg to the output stream with a reference 
    /// to a span of source designated by \p span.
    static void info(const string& msg, const SourceSpan &span);

    /// Log a warning message \p msg to the output stream.
    static void warn(const string& msg);

    /// Log a warning message \p msg to the output stream with a reference
    /// to a span of source designated by \p span.
    static void warn(const string& msg, const SourceSpan& span);

    /// Log an error message \p msg to the output stream.
    __attribute__((noreturn))
    static void error(const string& msg) noexcept;

    /// Log an error message \p msg to the output stream with a reference to a
    /// span of source designed by \p span.
    __attribute__((noreturn))
    static void error(const string& msg, const SourceSpan& span) noexcept;
};

} // namespace scc

#endif // SCC_LOGGER_H_

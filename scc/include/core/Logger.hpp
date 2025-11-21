#ifndef SCC_LOGGER_H_
#define SCC_LOGGER_H_

#include "Span.hpp"

#include <iostream>
#include <ostream>
#include <string>

namespace scc {

class Logger {
    static std::ostream* s_output;
    static bool s_color;

    static void log_source(const Span& span);

public:
    Logger() = delete;

    static void init(std::ostream& output = std::cerr);

    static void info(const std::string& msg);
    static void info(const std::string& msg, const Span& span);

    static void warn(const std::string& msg);
    static void warn(const std::string& msg, const Span &span);

    static void error(const std::string& msg);
    static void error(const std::string& msg, const Span &span);
};

} // namespace scc

#endif // SCC_LOGGER_H_

//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lace/core/Diagnostics.hpp"
#include "lace/tools/Files.hpp"

#include <cassert>
#include <string>
#include <vector>

using namespace lace;
using namespace lace::log;

static std::ostream* g_out = nullptr;
static bool g_color = false;
static bool g_errors = false;

/// Reassess whether colors should be used for the current output stream.
static void adjust_color_compatibility() {
    g_color = (g_out == &std::cout || g_out == &std::cerr);
}

/// Read in the lines of source code that |span| covers from the source file
/// at |path|.
static std::vector<std::string> read_source(const Span& span) {
    assert(span.end.line >= span.start.line && "span ends before it starts!");
    
    const std::string contents = read_file(span.path);
    std::size_t line = 1, start = 0;
    std::vector<std::string> lines = {};
    lines.reserve(std::min(1, span.end.line - span.start.line + 1));

    for (std::size_t i = 0, e = contents.size(); i <= e; ++i) {
        if (i == e || contents[i] == '\n') {
            if (span.start.line <= line && line <= span.end.line)
                lines.push_back(contents.substr(start, i - start));

            start = i + 1;
            ++line;
        }
    }

    return lines;
}

/// Print the lines of source code that |span| covers from the source file at
/// |path|.
static void print_source(const Span& span) {
    const uint32_t line_len = std::to_string(span.start.line).size();
    const std::vector<std::string> lines = read_source(span);
    uint32_t line_n = span.start.line;

    *g_out << std::string(line_len + 2, ' ') << "┌─[" << span.path << ':'
           << span.start.line << "]\n";

    for (const std::string& line : lines) {
        if (g_color) {
            std::string line_str = std::to_string(line_n);

            *g_out << "\e[38;5;240m" << line_str << "\033[0m" 
                   << std::string(line_len + 2 - line_str.length(), ' ') 
                   << "│ " << line << '\n';
        } else {
            *g_out << line_n << ' ' << line << '\n';
        }

        ++line_n;
    }

    *g_out << std::string(line_len + 2, ' ') << "╰──\n";
}

void log::init(std::ostream& os) {
    set_output_stream(os);
}

void log::set_output_stream(std::ostream& os) {
    g_out = &os;
    adjust_color_compatibility();
}

void log::clear_output_stream() {
    g_out = nullptr;
}

void log::flush() {
    if (g_errors) {
        log::fatal("unrecoverable errors found, stopping");
        std::exit(1);
    }

    if (g_out)
        g_out->flush();
}

void log::note(const std::string& msg) {
    if (!g_out)
        return;

    *g_out << (g_color ? "\033[1;35mnote:\033[0m " : "note: ") << msg << '\n';
}

void log::note(const std::string& msg, const Location& loc) {
    if (!g_out)
        return;

    *g_out << loc.path << ':' << loc.line << ':' << loc.col << ':'
           << (g_color ? " \033[1;35mnote:\033[0m " : " note: ") << msg << '\n';
}

void log::note(const std::string& msg, const Span& span) {
    if (!g_out)
        return;

    *g_out << (g_color ? "\033[1;35m ! \033[0m" : " ! ") << msg << '\n';
    print_source(span);
}

void log::warn(const std::string& msg) {
    if (!g_out)
        return;

    *g_out << (g_color ? "\033[1;33mwarning:\033[0m " : "warning: ") << msg 
           << '\n';
}

void log::warn(const std::string& msg, const Location& loc) {
    if (!g_out)
        return;

    *g_out << loc.path << ':' << loc.line << ':' << loc.col << ':'
           << (g_color ? " \033[1;33mwarning:\033[0m " : " warning: ") << msg 
           << '\n';
}

void log::warn(const std::string& msg, const Span& span) {
    if (!g_out)
        return;

    *g_out << (g_color ? "\033[33m * \033[0m" : " * ") << msg << '\n';
    print_source(span);
}

void log::error(const std::string& msg) {
    if (g_out) {
        *g_out << (g_color ? "\033[1;31merror:\033[0m " : "error: ") << msg 
               << '\n';
    }

    g_errors = true;
}

void log::error(const std::string& msg, const Location& loc) {
    if (g_out) {
        *g_out << loc.path << ':' << loc.line << ':' << loc.col << ':'
               << (g_color ? " \033[1;31merror:\033[0m " : " error: ") << msg 
               << '\n';
    }

    g_errors = true;
}

void log::error(const std::string& msg, const Span& span) {
    if (g_out) {
        *g_out << (g_color ? "\033[1;31m x \033[0m" : " x ") << msg << '\n';
        print_source(span);
    }

    g_errors = true;
}

void log::fatal(const std::string& msg) {
    if (g_out) {
        *g_out << (g_color ? "\033[1;31mfatal:\033[0m " : "fatal: ") << msg 
               << '\n';
    }

    std::exit(1);
}

void log::fatal(const std::string& msg, const Location& loc) {
    if (g_out) {
        *g_out << loc.path << ':' << loc.line << ':' << loc.col << ':'
               << (g_color ? " \033[1;31mfatal:\033[0m " : " fatal: ") << msg 
               << '\n';
    }

    std::exit(1);
}

void log::fatal(const std::string& msg, const Span& span) {
    error(msg, span);
    std::exit(1);
}

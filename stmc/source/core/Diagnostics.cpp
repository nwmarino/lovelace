//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/core/Diagnostics.hpp"
#include "stmc/tools/Files.hpp"
#include "stmc/types/SourceSpan.hpp"

#include <cassert>
#include <string>
#include <vector>

using namespace stm;

using std::vector;

/// Read in specific lines of source code from the file at \p path based on the
/// span of source code designated by \p span.
static vector<string> read_source(const string& path, SourceSpan span) {
    assert(span.end.line >= span.start.line && "span ends before it starts!");

    vector<string> lines = {};
    lines.reserve(std::min(1, span.end.line - span.start.line + 1));
    string contents = read_file(path);

    size_t line = 1, start = 0;
    for (size_t i = 0, e = contents.size(); i <= e; ++i) {
        if (i == e || contents[i] == '\n') {
            if (span.start.line <= line && line <= span.end.line) {
                lines.push_back(contents.substr(start, i - start));
            }

            start = i + 1;
            ++line;
        }
    }

    return lines;
}

void Diagnostics::print_source(SourceSpan span) const {
    if (!m_read)
        return;

    uint32_t line_len = std::to_string(span.start.line).size();

    *m_output << string(line_len + 2, ' ') << "┌─[" << m_path << ':'
        << span.start.line << "]\n";

    uint32_t line_n = span.start.line;
    vector<string> lines = read_source(m_path, span);
    for (const string& line : lines) {
        if (m_color) {
            string line_str = std::to_string(line_n);

            *m_output << "\e[38;5;240m" << line_str << "\033[0m" 
                << string(line_len + 2 - line_str.length(), ' ') 
                << "│ " << line << '\n';
        } else {
            *m_output << line_n << ' ' << line << '\n';
        }

        ++line_n;
    }
    
    *m_output << string(line_len + 2, ' ') << "╰──\n";
}

Diagnostics::Diagnostics(ostream* os, bool read) : m_output(os), m_read(read) {
    if (os == &std::cout || os == &std::cerr) {
        m_color = true;
    } else {
        m_color = false;
    }
}

void Diagnostics::set_output_stream(ostream* os) {
    m_output = os;

    if (os == &std::cout || os == &std::cerr) {
        m_color = true;
    } else {
        m_color = false;
    }
}

void Diagnostics::log(const string& msg) {
    assert(m_output && "no output stream set!");

    *m_output << "stmc: " << msg << '\n';
}

void Diagnostics::note(const string& msg) {
    assert(m_output && "no output stream set!");

    *m_output << "stmc: " << (m_color ? "\033[1;35mnote:\033[0m " : "note: ") 
        << msg << '\n';
}

void Diagnostics::note(const string& msg, SourceLocation loc) {
    assert(m_output && "no output stream set!");
    assert(!m_path.empty() && "file path not set!");

    *m_output << m_path << ':' << loc.line << ':' << loc.col << ':'
        << (m_color ? " \033[1;35mnote:\033[0m " : " note: ") << msg << '\n';
}

void Diagnostics::note(const string& msg, SourceSpan span) {
    assert(m_output && "no output stream set!");
    assert(!m_path.empty() && "file path not set!");

    *m_output << (m_color ? "\033[1;35m ! \033[0m" : " ! ") << msg << '\n';

    print_source(span);
}

void Diagnostics::warn(const string& msg) {
    assert(m_output && "no output stream set!");

    *m_output << "stmc: " << (m_color ? "\033[1;33mwarning:\033[0m " : "warning: ") 
        << msg << '\n';
}

void Diagnostics::warn(const string& msg, SourceLocation loc) {
    assert(m_output && "no output stream set!");
    assert(!m_path.empty() && "file path not set!");

    *m_output << m_path << ':' << loc.line << ':' << loc.col << ':'
        << (m_color ? " \033[1;33mwarning:\033[0m " : " warning: ") << msg 
        << '\n';
}

void Diagnostics::warn(const string& msg, SourceSpan span) {
    assert(m_output && "no output stream set!");
    assert(!m_path.empty() && "file path not set!");

    *m_output << (m_color ? "\033[33m * \033[0m" : " * ") << msg << '\n';
    print_source(span);
}

void Diagnostics::fatal(const string& msg) {
    assert(m_output && "no output stream set!");

    *m_output << "stmc: " << (m_color ? "\033[1;31mfatal:\033[0m " : "fatal: ") 
        << msg << '\n';

    std::exit(1);
}

void Diagnostics::fatal(const string& msg, SourceLocation loc) {
    assert(m_output && "no output stream set!");
    assert(!m_path.empty() && "file path not set!");

    *m_output << m_path << ':' << loc.line << ':' << loc.col << ':'
        << (m_color ? " \033[1;31mfatal:\033[0m " : " fatal: ") << msg << '\n';

    std::exit(1);
}

void Diagnostics::fatal(const string& msg, SourceSpan span) {
    assert(m_output && "no output stream set!");
    assert(!m_path.empty() && "file path not set!");

    *m_output << (m_color ?"\033[1;31m x \033[0m" : " x ") << msg << '\n';
    print_source(span);

    std::exit(1);
}

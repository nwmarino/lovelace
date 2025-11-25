//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "core/Logger.hpp"
#include "core/Tools.hpp"

#include <cassert>
#include <vector>

using namespace scc;

using std::vector;

ostream* Logger::s_output = nullptr;
bool Logger::s_color = false;

/// Returns the source code designated by \p span line-by-line.
static vector<string> source(const SourceSpan& span) {
    vector<string> lines { span.end.line - span.start.line };
    string full = read_file(span.start.path);

    uint64_t line = 1;
    uint64_t start = 0;

    // Move over the source code and break it up line-by-line if it is within
    // the bounds of |span|.
    for (uint64_t idx = 0; idx <= full.length(); ++idx) {
        if (idx == full.length() || full[idx] == '\n') {
            if (line >= span.start.line && line <= span.end.line)
                lines.push_back(full.substr(start, idx - start));
            
            start = idx + 1;
            line++;
        }
    }

    return lines;
}

void Logger::log_source(const SourceSpan& span) {
    uint32_t line_len = std::to_string(span.start.line).length();

    *s_output << string(line_len + 2, ' ') << "┌─[" << span.start.path 
              << ':' << span.start.line << "]\n";

    uint32_t line_n = span.start.line;
    for (auto line : source(span)) {
        
        if (Logger::s_color) {
            *s_output << "\e[38;5;240m" << line_n++ << "\033[0m" 
                      << string(line_len + 2 - std::to_string(line_n).length(), ' ') 
                      << "│ " << line << '\n';
        } else {
            *s_output << line_n++ << ' ' << line << '\n';
        }
    }

    *s_output << string(line_len + 2, ' ') << "╰──\n";
}

void Logger::init(ostream& output) {
    Logger::s_output = &output;
    Logger::s_color = s_output == &std::cout || s_output == &std::cerr;
}

void Logger::info(const string& msg) {
    if (!s_output) 
        return;

    *s_output << "scc: ";
    
    if (Logger::s_color) {
        *s_output << "\033[1;35minfo:\033[0m ";
    } else {
        *s_output << "info: ";
    }

    *s_output << msg << '\n';
}

void Logger::info(const string& msg, const SourceSpan& span) {
    if (!s_output) 
        return;

    if (Logger::s_color) {
        *s_output << "\033[1;35m !\033[0m ";
    } else {
        *s_output << " ! ";
    }
    
    *s_output << msg << '\n';
    log_source(span);
}

void Logger::warn(const string& msg) {
    if (!s_output) 
        return;

    *s_output << "scc: ";
    
    if (Logger::s_color) {
        *s_output << "\033[1;33mwarning:\033[0m ";
    } else {
        *s_output << "warning: ";
    }

    *s_output << msg << '\n';
}

void Logger::warn(const string& msg, const SourceSpan& span) {
    if (!s_output) 
        return;

    if (Logger::s_color) {
        *s_output << "\033[1;33m ⚠︎\033[0m ";
    } else {
        *s_output << " ⚠︎ ";
    }

    *s_output << msg << '\n';
    log_source(span);
}

void Logger::error(const string& msg) noexcept {
    if (s_output) {
        *s_output << "scc: ";
        
        if (Logger::s_color) {
            *s_output << "\033[1;31merror:\033[0m ";
        } else {
            *s_output << "error: ";
        }

        *s_output << msg << std::endl;
    }

    std::exit(1);
}

void Logger::error(const string& msg, const SourceSpan& span) noexcept {
    if (s_output) {
        if (Logger::s_color) {
            *s_output << "\033[1;31m ˣ\033[0m ";
        } else {
            *s_output << " ˣ ";
        }
        
        *s_output << msg << '\n';
        log_source(span);
    }

    std::exit(1);
}

#include "../../include/core/Logger.hpp"

#include <cassert>
#include <fstream>
#include <vector>

using namespace scc;

static std::string read_file(const std::string& path) {
    std::ifstream file(path, std::ios::ate);
    if (!file || !file.is_open())
        Logger::error("failed to open file: " + path);

    uint32_t size = file.tellg();
    std::string contents;
    contents.resize(size);
    file.seekg(std::ios::beg);

    if (!file.read(contents.data(), contents.size()))
        Logger::error("failed to read file: " + path);

    file.close();
    return contents;
}

static std::vector<std::string> source(const Span& span) {
    std::vector<std::string> lines { span.end.line - span.begin.line };
    std::string full = read_file(span.begin.path);

    uint64_t line = 1;
    uint64_t start = 0;
    for (uint64_t idx = 0; idx <= full.length(); ++idx) {
        if (idx == full.length() || full[idx] == '\n') {
            if (line >= span.begin.line && line <= span.end.line)
                lines.push_back(full.substr(start, idx - start));
            
            start = idx + 1;
            line++;
        }
    }

    return lines;
}

std::ostream* Logger::s_output = nullptr;
bool Logger::s_color = false;

void Logger::log_source(const Span& span) {
    uint32_t line_len = std::to_string(span.begin.line).length();

    *s_output << std::string(line_len + 2, ' ') << "┌─[" << span.begin.path 
              << ':' << span.begin.line << "]\n";

    uint32_t line_n = span.begin.line;
    for (auto line : source(span)) {
        
        if (Logger::s_color) {
            *s_output << "\e[38;5;240m" << line_n++ << "\033[0m" 
                      << std::string(line_len + 2 - std::to_string(line_n).length(), ' ') 
                      << "│ " << line << '\n';
        } else {
            *s_output << line_n++ << ' ' << line << '\n';
        }
    }

    *s_output << std::string(line_len + 2, ' ') << "╰──\n";
}

void Logger::init(std::ostream& output) {
    Logger::s_output = &output;
    Logger::s_color = s_output == &std::cout || s_output == &std::cerr;
}

void Logger::info(const std::string& msg) {
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

void Logger::info(const std::string& msg, const Span& span) {
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

void Logger::warn(const std::string& msg) {
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

void Logger::warn(const std::string& msg, const Span &span) {
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

void Logger::error(const std::string& msg) {
    if (s_output) {
        *s_output << "scc: ";
        
        if (Logger::s_color) {
            *s_output << "\033[1;31mfatal:\033[0m ";
        } else {
            *s_output << "error: ";
        }

        *s_output << msg << std::endl;
    }

    abort();
}

void Logger::error(const std::string& msg, const Span &span) {
    if (s_output) {
        if (Logger::s_color) {
            *s_output << "\033[1;31m ˣ\033[0m ";
        } else {
            *s_output << " ˣ ";
        }
        
        *s_output << msg << '\n';
        log_source(span);
    }

    abort();
}

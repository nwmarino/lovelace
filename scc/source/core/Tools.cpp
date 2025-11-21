//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "core/Logger.hpp"
#include "core/Tools.hpp"

#include <fstream>

using namespace scc;

std::string scc::read_file(const std::string& path) {
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

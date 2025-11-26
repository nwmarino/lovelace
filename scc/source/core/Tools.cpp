//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "scc/core/Logger.hpp"
#include "scc/core/Tools.hpp"

#include <fstream>

using namespace scc;

string scc::read_file(const string& path) {
    std::ifstream file(path, std::ios::ate);
    if (!file || !file.is_open())
        Logger::error("failed to open file: " + path);

    uint32_t size = file.tellg();
    string contents;
    contents.resize(size);
    file.seekg(std::ios::beg);

    if (!file.read(contents.data(), contents.size()))
        Logger::error("failed to read file: " + path);

    file.close();
    return contents;
}

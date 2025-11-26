//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "scc/core/Logger.hpp"
#include "scc/core/Tools.hpp"

#include <fstream>

using namespace scc;

using std::ifstream;
using std::ios;

string scc::without_extension(const string& path) {
    std::size_t first = path.find_last_of('.');
    if (first == string::npos)
        return string(path);

    return path.substr(0, first);
}

string scc::with_pure_extension(const string& path) {
    return without_extension(path) + ".i";
}

string scc::with_assembly_extension(const string& path) {
    return without_extension(path) + ".s";
}

string scc::with_object_extension(const string& path) {
    return without_extension(path) + ".o";
}

string scc::read_file(const string& path) {
    ifstream file(path, ios::ate);
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

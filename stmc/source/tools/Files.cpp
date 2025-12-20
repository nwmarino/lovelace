//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "stmc/tools/Files.hpp"

#include <cstdint>
#include <fstream>
#include <stdexcept>

using namespace stm;

using std::ifstream;
using std::ios;

string stm::read_file(const string& path) {
    ifstream file(path, ios::ate);
    if (!file || !file.is_open())
        throw std::runtime_error("failed to open file: " + path);

    uint32_t size = file.tellg();
    string contents;
    contents.resize(size);
    file.seekg(std::ios::beg);

    if (!file.read(contents.data(), contents.size()))
        throw std::runtime_error("failed to read file: " + path);

    file.close();
    return contents;
}

string stm::without_extension(const string& path) {
    std::size_t first = path.find_last_of('.');
    if (first == string::npos)
        return string(path);

    return path.substr(0, first);
}

string stm::with_assembly_extension(const string& path) {
    return without_extension(path) + ".s";
}

string stm::with_object_extension(const string& path) {
    return without_extension(path) + ".o";
}

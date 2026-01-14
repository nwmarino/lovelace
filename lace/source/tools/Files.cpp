//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lace/core/Diagnostics.hpp"
#include "lace/tools/Files.hpp"

#include <cstdint>
#include <fstream>

using namespace lace;

std::string lace::read_file(const std::string& path) {
    std::ifstream file(path, std::ios::ate);
    if (!file || !file.is_open())
        log::fatal("failed to open file: " + path);

    uint32_t size = file.tellg();
    std::string contents;
    contents.resize(size);
    file.seekg(std::ios::beg);

    if (!file.read(contents.data(), contents.size()))
        log::fatal("failed to read file: " + path);

    file.close();
    return contents;
}

std::string lace::without_extension(const std::string& path) {
    std::size_t first = path.find_last_of('.');
    if (first == std::string::npos)
        return path;

    return path.substr(0, first);
}

std::string lace::with_assembly_extension(const std::string& path) {
    return without_extension(path) + ".s";
}

std::string lace::with_object_extension(const std::string& path) {
    return without_extension(path) + ".o";
}

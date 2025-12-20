//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef STATIM_FILES_H_
#define STATIM_FILES_H_

#include <string>

namespace stm {

using std::string;

/// Read in the file at \p path and return its contents as a string.
string read_file(const string& path);

/// Returns the file path \p path without its last file extension.
string without_extension(const string& path);

/// Returns the file path \p path with only the '.s' file extension.
string with_assembly_extension(const string& path);

/// Returns the file path \p path with only the '.o' file extension.
string with_object_extension(const string& path);

} // namespace stm

#endif // STATIM_FILES_H_

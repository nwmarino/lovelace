//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SCC_TOOLS_H_
#define SCC_TOOLS_H_

//
// This header file declares some useful helper functions and internal tools
// for the C frontend.
//

#include <string>

namespace scc {
    
using std::string;

/// Returns the path \p path without any file extensions (suffixed with '.').
string without_extension(const string& path);

/// Returns the file at path \p path with just the pure post C preprocessor
/// file extension.
string with_pure_extension(const string& path);

/// Returns the file at path \p path with just the native assembly file 
/// extension.
string with_assembly_extension(const string& path);

/// Returns the file at path \p path with just the native assembly file 
/// extension.
string with_object_extension(const string& path);

/// Reads and returns the entirety of the contents of the file at \p path.
string read_file(const string& path);

} // namespace scc

#endif // SCC_TOOLS_H_

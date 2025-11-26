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

/// Reads and returns the entirety of the contents of the file at \p path.
string read_file(const string& path);

} // namespace scc

#endif // SCC_TOOLS_H_

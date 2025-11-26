//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#ifndef SPBE_REGISTER_ANALYSIS_H_
#define SPBE_REGISTER_ANALYSIS_H_

#include "spbe/machine/MachObject.hpp"

namespace spbe {

class RegisterAnalysis {
    MachObject& m_obj;

public:
    RegisterAnalysis(MachObject& obj) : m_obj(obj) {}
    
    RegisterAnalysis(const RegisterAnalysis&) = delete;
    RegisterAnalysis& operator = (const RegisterAnalysis&) = delete;

    void run();
};

} // namespace spbe

#endif // SPBE_REGISTER_ANALYSIS_H_

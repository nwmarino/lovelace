#ifndef SPBE_MACH_ANALYSIS_H_
#define SPBE_MACH_ANALYSIS_H_

#include "MachObject.hpp"

namespace spbe {

/// Machine analysis pass to do liveness analysis, register allocation, etc.
class FunctionRegisterAnalysis final {
    MachObject& m_obj;

public:
    FunctionRegisterAnalysis(MachObject& obj) : m_obj(obj) {}
    
    FunctionRegisterAnalysis(const FunctionRegisterAnalysis&) = delete;
    FunctionRegisterAnalysis& operator = (const FunctionRegisterAnalysis&) = delete;

    void run();
};

/// Machine pass to dump assembly with optimizer details.
class PrettyPrinter final {
    MachObject& m_obj;

public:
    PrettyPrinter(MachObject& obj) : m_obj(obj) {}
    
    PrettyPrinter(const PrettyPrinter&) = delete;
    PrettyPrinter& operator = (const PrettyPrinter&) = delete;

    void run(std::ostream& os);
};

/// Machine pass to emit final assembly code.
class AsmWriter final {
    MachObject& m_obj;

public:
    AsmWriter(MachObject& obj) : m_obj(obj) {}
    
    AsmWriter(const AsmWriter&) = delete;
    AsmWriter& operator = (const AsmWriter&) = delete;

    void run(std::ostream& os);
};

} // namespace spbe

#endif // SPBE_MACH_ANALYSIS_H_

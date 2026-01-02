cmake_minimum_required(VERSION 4.0)

project(spbe LANGUAGES CXX)

set(SPBE_HEADERS
    #include/analysis/LLVMTranslatePass.hpp
    include/spbe/analysis/Pass.hpp
    include/spbe/analysis/SSARewritePass.hpp
    include/spbe/analysis/TargetLoweringPass.hpp
    include/spbe/analysis/TrivialDCEPass.hpp
    include/spbe/graph/BasicBlock.hpp
    include/spbe/graph/CFG.hpp
    include/spbe/graph/Constant.hpp
    include/spbe/graph/Function.hpp
    include/spbe/graph/Global.hpp
    include/spbe/graph/InlineAsm.hpp
    include/spbe/graph/InstrBuilder.hpp
    include/spbe/graph/Instruction.hpp
    include/spbe/graph/Local.hpp
    include/spbe/graph/Type.hpp
    include/spbe/graph/Use.hpp
    include/spbe/graph/User.hpp
    include/spbe/graph/Value.hpp
    include/spbe/machine/AsmWriter.hpp
    include/spbe/machine/InstrSelector.hpp
    include/spbe/machine/MachBasicBlock.hpp
    include/spbe/machine/MachFunction.hpp
    include/spbe/machine/MachInstruction.hpp
    include/spbe/machine/MachObject.hpp
    include/spbe/machine/MachOperand.hpp
    include/spbe/machine/MachRegister.hpp
    include/spbe/machine/RegisterAllocator.hpp
    include/spbe/machine/RegisterAnalysis.hpp
    include/spbe/target/Target.hpp
    include/spbe/X64/X64.hpp
    include/spbe/X64/X64AsmWriter.hpp
    include/spbe/X64/X64InstrSelector.hpp
    include/spbe/X64/X64Printer.hpp
)

set(SPBE_SOURCES
    #source/analysis/LLVMTranslatePass.cpp
    source/analysis/PrintPass.cpp
    source/analysis/SSARewritePass.cpp
    source/analysis/TargetLoweringPass.cpp
    source/analysis/TrivialDCEPass.cpp
    source/graph/BasicBlock.cpp
    source/graph/CFG.cpp
    source/graph/Constant.cpp
    source/graph/Function.cpp
    source/graph/Global.cpp
    source/graph/InstrBuilder.cpp
    source/graph/Instruction.cpp
    source/graph/Local.cpp
    source/graph/Type.cpp
    source/graph/Use.cpp
    source/graph/Value.cpp
    source/machine/MachBasicBlock.cpp
    source/machine/MachFunction.cpp
    source/machine/MachInstruction.cpp
    source/machine/MachObject.cpp
    source/machine/MachOperand.cpp
    source/machine/RegisterAllocator.cpp
    source/machine/RegisterAnalysis.cpp
    source/target/Target.cpp
    source/X64/X64.cpp
    source/X64/X64AsmWriter.cpp
    source/X64/X64InstrSelector.cpp
    source/X64/X64Printer.cpp
)

add_library(spbe ${SPBE_SOURCES} ${SPBE_HEADERS})

target_include_directories(spbe
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(spbe PUBLIC cxx_std_20)

install(TARGETS spbe
    EXPORT spbeTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)

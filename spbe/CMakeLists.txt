cmake_minimum_required(VERSION 4.0)

project(spbe LANGUAGES CXX)

set(SPBE_HEADERS
    #include/analysis/LLVMTranslatePass.hpp
    include/analysis/Pass.hpp
    include/analysis/SSARewritePass.hpp
    include/analysis/TargetLoweringPass.hpp
    include/analysis/TrivialDCEPass.hpp
    include/graph/BasicBlock.hpp
    include/graph/CFG.hpp
    include/graph/Constant.hpp
    include/graph/Function.hpp
    include/graph/Global.hpp
    include/graph/InlineAsm.hpp
    include/graph/InstrBuilder.hpp
    include/graph/Instruction.hpp
    include/graph/Local.hpp
    include/graph/Type.hpp
    include/graph/Use.hpp
    include/graph/User.hpp
    include/graph/Value.hpp
    include/machine/AsmWriter.hpp
    include/machine/InstrSelector.hpp
    include/machine/MachBasicBlock.hpp
    include/machine/MachFunction.hpp
    include/machine/MachInstruction.hpp
    include/machine/MachObject.hpp
    include/machine/MachOperand.hpp
    include/machine/MachRegister.hpp
    include/machine/RegisterAllocator.hpp
    include/machine/RegisterAnalysis.hpp
    include/target/Target.hpp
    include/X64/X64.hpp
    include/X64/X64AsmWriter.hpp
    include/X64/X64InstrSelector.hpp
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

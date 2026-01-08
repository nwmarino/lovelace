cmake_minimum_required(VERSION 4.0)

project(lir LANGUAGES CXX)

set(LIR_HEADERS
    include/lir/analysis/LoweringPass.hpp
    include/lir/analysis/Pass.hpp
    include/lir/graph/BasicBlock.hpp
    include/lir/graph/CFG.hpp
    include/lir/graph/Constant.hpp
    include/lir/graph/Function.hpp
    include/lir/graph/Global.hpp
    include/lir/graph/Builder.hpp
    include/lir/graph/Instruction.hpp
    include/lir/graph/Local.hpp
    include/lir/graph/Type.hpp
    include/lir/graph/Use.hpp
    include/lir/graph/User.hpp
    include/lir/graph/Value.hpp
    include/lir/machine/AsmWriter.hpp
    include/lir/machine/InstSelector.hpp
    include/lir/machine/MachFunction.hpp
    include/lir/machine/Machine.hpp
    include/lir/machine/MachInst.hpp
    include/lir/machine/MachLabel.hpp
    include/lir/machine/MachOperand.hpp
    include/lir/machine/Register.hpp
    include/lir/machine/RegisterAllocator.hpp
    include/lir/machine/RegisterAnalysis.hpp
    include/lir/machine/Segment.hpp
)

set(LIR_SOURCES
    source/analysis/LoweringPass.cpp
    source/graph/BasicBlock.cpp
    source/graph/CFG.cpp
    source/graph/Constant.cpp
    source/graph/Function.cpp
    source/graph/Global.cpp
    source/graph/Builder.cpp
    source/graph/Instruction.cpp
    source/graph/Local.cpp
    source/graph/Print.cpp
    source/graph/Type.cpp
    source/graph/Value.cpp
    source/machine/AsmWriter.cpp
    source/machine/InstSelector.cpp
    source/machine/MachFunction.cpp
    source/machine/Machine.cpp
    source/machine/MachInst.cpp
    source/machine/MachLabel.cpp
    source/machine/MachOperand.cpp
    source/machine/Register.cpp
    source/machine/RegisterAllocator.cpp
    source/machine/RegisterAnalysis.cpp
    source/machine/Segment.cpp
)

add_library(lir ${LIR_SOURCES} ${LIR_HEADERS})

target_include_directories(lir
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(lir PUBLIC cxx_std_20)

install(TARGETS lir
    EXPORT lirTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)

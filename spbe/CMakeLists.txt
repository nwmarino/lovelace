cmake_minimum_required(VERSION 4.0)

project(spbe VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(spbe SHARED
    source/analysis/LLVMTranslatePass.cpp
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
    source/machine/MachAnalysis.cpp
    source/machine/MachBasicBlock.cpp
    source/machine/MachFunction.cpp
    source/machine/MachInstruction.cpp
    source/machine/MachObject.cpp
    source/machine/MachOperand.cpp
    source/machine/Rega.cpp
    source/target/Target.cpp
)

add_library(spbe::spbe ALIAS spbe)

target_include_directories(spbe
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(spbe PUBLIC cxx_std_20)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)


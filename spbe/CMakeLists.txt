cmake_minimum_required(VERSION 4.0)

project(spbe VERSION 0.1.0 LANGUAGES CXX)

option(SPBE_BUILD_SHARED "build as a shared libary" ON)
option(SPBE_BUILD_TESTS "build tests" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(spbe SHARED
    source/graph/Constant.cpp
    source/graph/Global.cpp
    source/graph/Instruction.cpp
    source/graph/Local.cpp
    source/graph/Type.cpp
    source/graph/Use.cpp
    source/graph/Value.cpp
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


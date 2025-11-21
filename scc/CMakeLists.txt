cmake_minimum_required(VERSION 4.0)

project(scc VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(scc
    source/scc.cpp
    source/ast/Scope.cpp
    source/core/Logger.cpp
)

target_include_directories(scc
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(scc PUBLIC cxx_std_20)

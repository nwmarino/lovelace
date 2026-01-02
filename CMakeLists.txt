cmake_minimum_required(VERSION 4.1)

project(lovelace VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_SHARED_LIBS "build LIR as a shared lib" OFF)

enable_testing()

find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(GTest REQUIRED)

#if (LOVELACE_COMPILE_LIR)
    add_subdirectory(lir)
#endif()

#if (LOVELACE_COMPILE_LIR)
    add_subdirectory(lace)
#endif()

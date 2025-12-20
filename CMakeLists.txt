cmake_minimum_required(VERSION 4.1)

project(statim VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_SHARED_LIBS "build spbe as shared" OFF)

enable_testing()

find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(GTest REQUIRED)

#if (STATIM_COMPILE_SPBE)
    add_subdirectory(spbe)
#endif()

#if (STATIM_COMPILE_STMC)
    add_subdirectory(stmc)
#endif()

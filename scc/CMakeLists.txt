set(SCC_SOURCES
    source/scc.cpp
    source/ast/Scope.cpp
    source/core/Logger.cpp
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(scc ${SCC_SOURCES})

target_include_directories(scc
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(scc PUBLIC cxx_std_20)

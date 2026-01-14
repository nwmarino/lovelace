set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

message(STATUS "LLVM includes: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM definitions: ${LLVM_DEFINITIONS}")
message(STATUS "LLVM tools: ${LLVM_TOOLS_BINARY_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

add_subdirectory(source)

add_executable(lace source/lace.cpp)

target_compile_options(lace PRIVATE -g)

target_link_libraries(lace PUBLIC
    Codegen
    Core
    Lexer
    LIR
    Parser
    Tree
    Tools
)

target_include_directories(Codegen PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(Core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(Lexer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(Tree PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(lace PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(lace PUBLIC cxx_std_20)

llvm_config(lace USE_SHARED core irreader support clang)

add_executable(lace_test
    test/LexerTests.cpp
    test/DefnParserTests.cpp
    test/ExprParserTests.cpp
    test/RuneParserTests.cpp
    test/StmtParserTests.cpp
    test/TypeParserTests.cpp
    test/SymbolAnalysisTests.cpp
    test/SemanticAnalysisTests.cpp
    test/CodegenTests.cpp
)

target_include_directories(lace_test PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(lace_test PRIVATE
    Codegen
    Core
    Lexer
    LIR
    Parser
    Tree
    Tools
    GTest::GTest
    GTest::Main
)

llvm_config(lace_test USE_SHARED core irreader support clang)
set(SCC_SOURCES
    source/scc.cpp
    source/ast/Decl.cpp
    source/ast/Expr.cpp
    source/ast/Parser.cpp
    source/ast/Printer.cpp
    source/ast/QualType.cpp
    source/ast/Scope.cpp
    source/ast/Stmt.cpp
    source/ast/Type.cpp
    source/ast/TypeContext.cpp
    source/core/Logger.cpp
    source/core/Tools.cpp
    source/lexer/Lexer.cpp
)

add_compile_options(-g -D SCC_COMPILE_SAMPLES)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(scc ${SCC_SOURCES})

target_include_directories(scc PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(scc PUBLIC cxx_std_20)

enable_testing()

add_executable(scc_test
    source/ast/TypeContext.cpp
    source/ast/Expr.cpp
    source/ast/Parser.cpp
    source/ast/Printer.cpp
    source/ast/QualType.cpp
    source/ast/Scope.cpp
    source/ast/Type.cpp
    source/core/Logger.cpp
    source/core/Tools.cpp
    source/lexer/Lexer.cpp
    test/LexerTests.cpp
    test/ParserTests.cpp
)

target_include_directories(scc_test PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(scc_test PRIVATE
    GTest::GTest
    GTest::Main
)

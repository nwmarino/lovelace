set(LACE_SOURCES
    source/lace.cpp
    source/core/Diagnostics.cpp
    source/core/Lexer.cpp
    source/parser/ParseDefn.cpp
    source/parser/ParseExpr.cpp
    source/parser/Parser.cpp
    source/parser/ParseRune.cpp
    source/parser/ParseStmt.cpp
    source/tools/Files.cpp
    source/tree/AST.cpp
    source/tree/Defn.cpp
    source/tree/Expr.cpp
    source/tree/NameAnalysis.cpp
    source/tree/Printer.cpp
    source/tree/Rune.cpp
    source/tree/Scope.cpp
    source/tree/SemanticAnalysis.cpp
    source/tree/Stmt.cpp
    source/tree/SymbolAnalysis.cpp
    source/tree/Type.cpp
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(lace ${LACE_SOURCES})

target_compile_options(lace PRIVATE -g -D LACE_COMPILE_SAMPLES)

target_link_libraries(lace PUBLIC
    spbe
)

target_include_directories(lace PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(lace PUBLIC cxx_std_20)

add_executable(lace_test
    source/core/Diagnostics.cpp
    source/core/Lexer.cpp
    source/parser/ParseDefn.cpp
    source/parser/ParseExpr.cpp
    source/parser/Parser.cpp
    source/parser/ParseRune.cpp
    source/parser/ParseStmt.cpp
    source/tools/Files.cpp
    source/tree/AST.cpp
    source/tree/Defn.cpp
    source/tree/Expr.cpp
    source/tree/NameAnalysis.cpp
    source/tree/Printer.cpp
    source/tree/Rune.cpp
    source/tree/Scope.cpp
    source/tree/SemanticAnalysis.cpp
    source/tree/Stmt.cpp
    source/tree/SymbolAnalysis.cpp
    source/tree/Type.cpp

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
    spbe
    GTest::GTest
    GTest::Main
)

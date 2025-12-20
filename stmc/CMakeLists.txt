set(STMC_SOURCES
    source/stmc.cpp
    source/codegen/SPBECodegen.cpp
    source/codegen/SPBECodegenDecl.cpp
    source/codegen/SPBECodegenExpr.cpp
    source/codegen/SPBECodegenRune.cpp
    source/codegen/SPBECodegenStmt.cpp
    source/core/Diagnostics.cpp
    source/core/Lexer.cpp
    source/parser/ParseDecl.cpp
    source/parser/ParseExpr.cpp
    source/parser/Parser.cpp
    source/parser/ParseRune.cpp
    source/parser/ParseStmt.cpp
    source/tools/Files.cpp
    source/tree/Context.cpp
    source/tree/Decl.cpp
    source/tree/Expr.cpp
    source/tree/Rune.cpp
    source/tree/Scope.cpp
    source/tree/SemanticAnalysis.cpp
    source/tree/Stmt.cpp
    source/tree/SymbolAnalysis.cpp
    source/tree/Type.cpp
    source/tree/TypeUse.cpp
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(stmc ${STMC_SOURCES})

target_compile_options(stmc PRIVATE -g -D STMC_COMPILE_SAMPLES)

target_link_libraries(stmc PUBLIC
    spbe
) 

target_include_directories(stmc PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(stmc PUBLIC cxx_std_20)

add_executable(stmc_test
    source/codegen/SPBECodegen.cpp
    source/codegen/SPBECodegenDecl.cpp
    source/codegen/SPBECodegenExpr.cpp
    source/codegen/SPBECodegenRune.cpp
    source/codegen/SPBECodegenStmt.cpp
    source/core/Diagnostics.cpp
    source/core/Lexer.cpp
    source/parser/ParseDecl.cpp
    source/parser/ParseExpr.cpp
    source/parser/Parser.cpp
    source/parser/ParseRune.cpp
    source/parser/ParseStmt.cpp
    source/tools/Files.cpp
    source/tree/Context.cpp
    source/tree/Decl.cpp
    source/tree/Expr.cpp
    source/tree/Rune.cpp
    source/tree/Scope.cpp
    source/tree/SemanticAnalysis.cpp
    source/tree/Stmt.cpp
    source/tree/SymbolAnalysis.cpp
    source/tree/Type.cpp
    source/tree/TypeUse.cpp
    test/DeclParserTests.cpp
    test/ExprParserTests.cpp
    test/LexerTests.cpp
    test/RuneParserTests.cpp
    test/SemanticAnalysisTests.cpp
    test/SPBECodegenTests.cpp
    test/StmtParserTests.cpp
    test/SymbolAnalysisTests.cpp
    test/TypeParserTests.cpp
)

target_include_directories(stmc_test PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(stmc_test PRIVATE
    spbe
    GTest::GTest
    GTest::Main
)

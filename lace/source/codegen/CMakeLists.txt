set(CODEGEN_SOURCES
    LIRCodegen.cpp
    LIRCodegenBinary.cpp
    LIRCodegenCall.cpp
    LIRCodegenCast.cpp
    LIRCodegenDefn.cpp
    LIRCodegenExpr.cpp
    LIRCodegenStmt.cpp
    LIRCodegenUnary.cpp
    LLVMCodegen.cpp
    LLVMCodegenAddressed.cpp
    LLVMCodegenDefn.cpp
    LLVMCodegenOp.cpp
    LLVMCodegenStmt.cpp
    LLVMCodegenValued.cpp
)

add_library(Codegen
    STATIC
        ${CODEGEN_SOURCES}
)

target_include_directories(Codegen PUBLIC
    ${CMAKE_SOURCE_DIR}/lir/include/
)

target_link_libraries(Codegen
    PRIVATE
        Core
        Tree
        LIR
)

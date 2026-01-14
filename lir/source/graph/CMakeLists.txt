set(GRAPH_SOURCES
    BasicBlock.cpp
    Builder.cpp
    CFG.cpp
    Constant.cpp
    Function.cpp
    Global.cpp
    Instruction.cpp
    Local.cpp
    Printer.cpp
    Type.cpp
    Value.cpp
)

add_library(Graph
    STATIC
        ${GRAPH_SOURCES}
)

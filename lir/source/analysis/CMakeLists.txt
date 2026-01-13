set(ANALYSIS_SOURCES
    LoweringPass.cpp
)

add_library(Analysis
    STATIC
        ${ANALYSIS_SOURCES}
)

target_link_libraries(Analysis
    PRIVATE
        Graph
        Machine
)

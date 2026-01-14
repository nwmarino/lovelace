set(PARSER_SOURCES
    ParseDefn.cpp
    ParseExpr.cpp
    Parser.cpp
    ParseRune.cpp
    ParseStmt.cpp
)

add_library(Parser
    STATIC
        ${PARSER_SOURCES}
)

target_link_libraries(Parser
    PRIVATE
        Core
        Lexer
        Tools
        Tree
)

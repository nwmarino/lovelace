set(LEXER_SOURCES
    Lexer.cpp
)

add_library(Lexer
    STATIC
        ${LEXER_SOURCES}
)

target_link_libraries(Lexer
    PRIVATE
        Tools
)

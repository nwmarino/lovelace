set(TOOLS_SOURCES
    Files.cpp
)

add_library(Tools
    STATIC
        ${TOOLS_SOURCES}
)

target_link_libraries(Tools
    PRIVATE
        Core
)

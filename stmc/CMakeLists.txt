set(STMC_SOURCES
    source/stmc.cpp
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
    source/stmc.cpp
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

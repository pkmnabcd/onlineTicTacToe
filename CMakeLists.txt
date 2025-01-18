cmake_minimum_required(VERSION 3.14)

set(PROJECT_NAME OnlineTicTacToe)
set(CLIENT Client)
set(SERVER Server)
set(UNIT_TEST_RUNNER UnitTestRunner)

project(${PROJECT_NAME})

#
# Adding the relevant directories
#
include_directories(
    client
    game
    server
)

#
# Manually specifying all the source files.
#
set(GAME_HEADER_FILES
    game/ai.hpp
    game/engine.hpp
    game/interface.hpp
    game/ttt.hpp
    game/util.hpp

    game/rlutil.h
)
set(CLIENT_HEADER_FILES
    ${GAME_HEADER_FILES}
)
set(SERVER_HEADER_FILES
    ${GAME_HEADER_FILES}
)
set(ALL_HEADER_FILES
    ${GAME_HEADER_FILES}
    ${CLIENT_HEADER_FILES}
    ${SERVER_HEADER_FILES}
)

set(GAME_SOURCE_FILES
    game/ai.cpp
    game/engine.cpp
    game/interface.cpp
    game/ttt.cpp
    game/util.cpp
)
set(CLIENT_SOURCE_FILES
    ${GAME_SOURCE_FILES}
)
set(SERVER_SOURCE_FILES
    ${GAME_SOURCE_FILES}
)
set(ALL_CLIENT_FILES
    ${GAME_CLIENT_FILES}
    ${CLIENT_CLIENT_FILES}
    ${SERVER_CLIENT_FILES}
)

set(UNIT_TEST_FILES
    TestTicTacToe.cpp
)

#
# This is the main target
#
add_executable(${CLIENT} ${CLIENT_HEADER_FILES} ${CLIENT_SOURCE_FILES} client/main.cpp)
add_executable(${SERVER} ${SERVER_HEADER_FILES} ${SERVER_SOURCE_FILES} server/main.cpp)
add_executable(${UNIT_TEST_RUNNER} ${ALL_HEADER_FILES} ${ALL_SOURCE_FILES} ${UNIT_TEST_FILES})

#
# We want the C++ 20 standard for our project
#
set_property(TARGET ${CLIENT} PROPERTY CXX_STANDARD 23)
set_property(TARGET ${SERVER} PROPERTY CXX_STANDARD 23)
set_property(TARGET ${UNIT_TEST_RUNNER} PROPERTY CXX_STANDARD 23)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${CLIENT} PRIVATE /W4 /permissive- )
    target_compile_options(${SERVER} PRIVATE /W4 /permissive- )
    target_compile_options(${UNIT_TEST_RUNNER} PRIVATE /W4 /permissive-)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${CLIENT} PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(${SERVER} PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(${UNIT_TEST_RUNNER} PRIVATE -Wall -Wextra -pedantic)
endif()

#
# Prepare a pre-build step to run clang-format over all the [ch]pp source files.
# Start by finding the location of the clang-format executable.
#
find_program(CLANG_FORMAT "clang-format")
if (CLANG_FORMAT)
    #
    # Need to take the simple source file locations used for the project and get their full
    # file system locations for use in putting together the clang-format command line
    #
    unset(SOURCE_FILES_PATHS)
    foreach(SOURCE_FILE ${ALL_HEADER_FILES} ${ALL_SOURCE_FILES} ${UNIT_TEST_FILES} client/main.cpp server/main.cpp)
        get_source_file_property(WHERE ${SOURCE_FILE} LOCATION)
        set(SOURCE_FILES_PATHS ${SOURCE_FILES_PATHS} ${WHERE})
    endforeach()

    #
    # This creates the clang-format target/command
    #
    add_custom_target(
        ClangFormat
        COMMAND ${CLANG_FORMAT}
        -i
        -style=file
        ${SOURCE_FILES_PATHS}
    )
    #
    # This makes the clang-format target a dependency of the main project
    #
    add_dependencies(${CLIENT} ClangFormat)
    add_dependencies(${SERVER} ClangFormat)
else()
    message("Unable to find clang-format")
endif()

#
# Add GoogleTest
#
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY    https://github.com/google/googletest.git
    GIT_TAG           v1.15.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googleTest)

# Now simply link against gtest or gtest_main as needed.
target_link_libraries(${UNIT_TEST_RUNNER} gtest_main)


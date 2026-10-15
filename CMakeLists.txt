cmake_minimum_required(VERSION 3.20)
project(forge CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# frg.exe goes in bin/, with the runtime folder next to it.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_executable(frg
    src/main.cpp
    src/diagnostics.cpp
    src/types.cpp
    src/builtins.cpp
    src/prelude.cpp
    src/lexer.cpp
    src/parser.cpp
    src/project.cpp
    src/checker.cpp
    src/codegen.cpp
)
target_compile_options(frg PRIVATE -Wall -Wextra)
target_link_options(frg PRIVATE -static)

add_custom_command(TARGET frg POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/runtime ${CMAKE_SOURCE_DIR}/bin/runtime
)

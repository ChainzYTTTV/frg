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
    src/query.cpp
    src/format.cpp
    src/templates.cpp
    src/update.cpp
)
target_compile_options(frg PRIVATE -Wall -Wextra -Wno-missing-field-initializers)
target_link_options(frg PRIVATE -static)
target_link_libraries(frg PRIVATE winhttp)   # the update check (src/update.cpp)

# The runtime folder is copied next to frg.exe on every build, so a change to a runtime file alone is picked up too.
# (copy_directory_if_different only copies changed files, so unchanged files keep their dates and the build cache.)
add_custom_target(runtime ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_SOURCE_DIR}/runtime ${CMAKE_SOURCE_DIR}/bin/runtime
)
add_dependencies(frg runtime)

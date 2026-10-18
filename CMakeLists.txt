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
if(WIN32)
    target_link_options(frg PRIVATE -static)
    target_link_libraries(frg PRIVATE winhttp)   # the update check (src/update.cpp)
else()
    # Linux: C++'s own library goes into frg, so it runs on other Linux PCs too.
    target_link_options(frg PRIVATE -static-libstdc++ -static-libgcc)
endif()

# The runtime folder is copied next to frg.exe on every build, so a change to a runtime file alone is picked up too.
# (copy_directory_if_different only copies changed files, so unchanged files keep their dates and the build cache.)
# (CMake before 3.26, like Ubuntu 22.04's, only has copy_directory, which copies everything every time.)
if(CMAKE_VERSION VERSION_LESS 3.26)
    set(FRG_COPY_DIR copy_directory)
else()
    set(FRG_COPY_DIR copy_directory_if_different)
endif()
add_custom_target(runtime ALL
    COMMAND ${CMAKE_COMMAND} -E ${FRG_COPY_DIR} ${CMAKE_SOURCE_DIR}/runtime ${CMAKE_SOURCE_DIR}/bin/runtime
)
add_dependencies(frg runtime)

# Forge Edit (editor/): a small, light code editor for Forge games, drawn with raylib. Its fonts go next to it.
add_executable(forge-edit editor/forge_edit.cpp editor/process.cpp)
target_include_directories(forge-edit PRIVATE ${CMAKE_SOURCE_DIR}/runtime/raylib/include)
if(WIN32)
    target_link_libraries(forge-edit PRIVATE ${CMAKE_SOURCE_DIR}/runtime/raylib/lib/libraylib.a opengl32 gdi32 winmm)
    target_link_options(forge-edit PRIVATE -static -mwindows)
else()
    target_link_libraries(forge-edit PRIVATE ${CMAKE_SOURCE_DIR}/runtime/raylib/lib-linux/libraylib.a GL m pthread dl rt X11)
    target_link_options(forge-edit PRIVATE -static-libstdc++ -static-libgcc)
endif()
add_custom_target(editor-fonts ALL
    COMMAND ${CMAKE_COMMAND} -E ${FRG_COPY_DIR} ${CMAKE_SOURCE_DIR}/editor/fonts ${CMAKE_SOURCE_DIR}/bin/fonts
)
add_dependencies(forge-edit editor-fonts)

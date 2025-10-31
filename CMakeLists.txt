cmake_minimum_required(VERSION 3.21)

include(cmake/prelude.cmake)

project(
    clox
    VERSION 0.1.0
    DESCRIPTION "Lox Bytecode Interpreter in C"
    LANGUAGES C CXX
)

include(cmake/variables.cmake)

# ---- Declare library ----
add_library(
    clox_lib OBJECT
    src/lib/chunk.c
    src/lib/compiler.c
    src/lib/debug.c
    src/lib/memory.c
    src/lib/object.c
    src/lib/scanner.c
    src/lib/table.c
    src/lib/value.c
    src/lib/vm.c
)

target_include_directories(
    clox_lib ${warning_guard}
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

# ---- Declare executable ----
add_executable(clox src/bin/main.c)
add_executable(clox::exe ALIAS clox)
set_property(TARGET clox PROPERTY OUTPUT_NAME clox)
target_link_libraries(clox PRIVATE clox_lib)

# ---- Testing, Profiling and Coverage ----
if(CLOX_DEVELOPER_MODE)

    # ---- Install Catch2 ----
    include(FetchContent)

    FetchContent_Declare(
      Catch2
      GIT_REPOSITORY https://github.com/catchorg/Catch2.git
      GIT_TAG        v3.8.1 # or a later release
    )

    FetchContent_MakeAvailable(Catch2)

    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    include(CTest)
    include(Catch)

    # ---- Testing `scan()` ----
    add_executable(clox_test_builtin_input tests/test_builtin_input.cxx)
    target_link_libraries(clox_test_builtin_input PRIVATE clox_lib)
    target_link_libraries(clox_test_builtin_input PRIVATE Catch2::Catch2WithMain)

    catch_discover_tests(clox_test_builtin_input)

    # ---- Testing ____ ----
    # abc
endif()

# ---- Install rules ----
if(NOT CMAKE_SKIP_INSTALL_RULES)
    include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----
if(NOT CLOX_DEVELOPER_MODE)
    return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
    message(
        AUTHOR_WARNING
        "Developer mode is intended for developers of clox"
    )
endif()

include(cmake/dev-mode.cmake)


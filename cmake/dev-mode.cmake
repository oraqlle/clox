add_compile_definitions(CLOX_DEVELOPER_MODE)

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

add_custom_target(
    run
    COMMAND clox
    VERBATIM
)

add_dependencies(run clox)

option(ENABLE_COVERAGE "Enable coverage support separate from CTest's" OFF)
if(ENABLE_COVERAGE)
  include(cmake/coverage.cmake)
endif()

include(cmake/lint-targets.cmake)
include(cmake/spell-targets.cmake)

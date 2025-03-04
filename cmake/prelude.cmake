# ---- In-source guard ----

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(
      FATAL_ERROR
      "In-source builds are not supported. "
      "You may need to delete 'CMakeCache.txt' and 'CMakeFiles/' first."
  )
endif()

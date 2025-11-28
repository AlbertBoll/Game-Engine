function(fx_find_or_fetch_glad2)
  find_package(glad2 QUIET CONFIG) # expected to export glad::glad
  if(glad2_FOUND)
    return()
  endif()

  if(NOT ENABLE_FETCH_FALLBACK)
    message(FATAL_ERROR "glad2 not found and fallback disabled")
  endif()

  message(STATUS "[Fallback] Fetching glad2 v2.0.8")

  FetchContent_Declare(glad2
    URL https://github.com/Dav1dde/glad/archive/refs/tags/v2.0.8.zip
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    # URL_HASH SHA256=<REAL_SHA256>
  )

    # 2) …then fetch/extract
  FetchContent_MakeAvailable(glad2)

#   FetchContent_GetProperties(glad2)
#   if(NOT glad2_POPULATED)
#     FetchContent_Populate(glad2)
  add_subdirectory("${glad2_SOURCE_DIR}/cmake" glad_cmake)
#  endif()

  # Use GLAD_API from Options.cmake
  glad_add_library(glad REPRODUCIBLE EXCLUDE_FROM_ALL LOADER API ${GLAD_API})
  
  # 5) Normalize the public target name  
  if(NOT TARGET glad::glad)
    add_library(glad::glad ALIAS glad)
  endif()

  # (Optional) nice IDE grouping
  set_target_properties(glad PROPERTIES FOLDER "Dependencies")
endfunction()
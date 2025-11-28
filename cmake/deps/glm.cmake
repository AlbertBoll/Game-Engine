function(fx_find_or_fetch_glm)
  find_package(glm QUIET CONFIG) # glm::glm
  if(NOT glm_FOUND)
    if(NOT ENABLE_FETCH_FALLBACK)
      message(FATAL_ERROR "glm not found and fallback disabled")
    endif()
    message(STATUS "[Fallback] Fetching glm 1.0.1")
    FetchContent_Declare(glm
      URL https://github.com/g-truc/glm/archive/refs/tags/1.0.1.zip
      DOWNLOAD_EXTRACT_TIMESTAMP OFF
      # URL_HASH SHA256=<REAL_SHA256>
    )
    FetchContent_MakeAvailable(glm)
    if(NOT TARGET glm::glm)
      add_library(glm::glm ALIAS glm)
    endif()
  endif()
endfunction()
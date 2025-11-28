function(fx_find_or_fetch_glfw)
  find_package(glfw3 QUIET CONFIG) # provides target 'glfw'
  if(NOT glfw3_FOUND)
    if(NOT ENABLE_FETCH_FALLBACK)
      message(FATAL_ERROR "GLFW not found and fallback disabled")
    endif()
    message(STATUS "[Fallback] Fetching GLFW 3.4")
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
    set(GLFW_VULKAN_STATIC  OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(glfw
      URL https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip
      DOWNLOAD_EXTRACT_TIMESTAMP OFF
      # URL_HASH SHA256=<REAL_SHA256>
    )
    FetchContent_MakeAvailable(glfw) # defines 'glfw'
  endif()
endfunction()
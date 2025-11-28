function(fx_find_or_fetch_sdl3)
  find_package(SDL3 QUIET CONFIG) # SDL3::SDL3
  if(NOT SDL3_FOUND)
    if(NOT ENABLE_FETCH_FALLBACK)
      message(FATAL_ERROR "SDL3 not found and fallback disabled")
    endif()
    message(STATUS "[Fallback] Fetching SDL3 (SDL3 branch)")

    set(SDL3_TAG "release-3.2.0" CACHE STRING "SDL3 release tag to fetch")
    set(SDL3_URL "https://github.com/libsdl-org/SDL/archive/refs/tags/${SDL3_TAG}.zip")
    FetchContent_Declare(sdl3
      # URL https://github.com/libsdl-org/SDL/archive/refs/heads/SDL3.zip
      # URL https://github.com/libsdl-org/SDL/archive/refs/tags/<tag>.zip
      URL "${SDL3_URL}"
      DOWNLOAD_EXTRACT_TIMESTAMP OFF
    )
    FetchContent_MakeAvailable(sdl3)
    
    # Normalize exported target names
    if(NOT TARGET SDL3::SDL3)
      if(TARGET SDL3::SDL3-shared)
        add_library(SDL3::SDL3 ALIAS SDL3::SDL3-shared)
      elseif(TARGET SDL3::SDL3-static)
        add_library(SDL3::SDL3 ALIAS SDL3::SDL3-static)
      else()
        message(FATAL_ERROR "Could not normalize SDL3 target")
      endif()
    endif()
  endif()
endfunction()
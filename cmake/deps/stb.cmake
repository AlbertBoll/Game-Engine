function(fx_find_or_fetch_stb_image)
  if(TARGET stb::image)
    return()
  endif()

  find_package(stb QUIET CONFIG)
  if(stb_FOUND)

    if(TARGET stb::stb)
      add_library(stb::image ALIAS stb::stb)
      return()
    endif()
  endif()

  if(NOT ENABLE_FETCH_FALLBACK)
    message(FATAL_ERROR "stb not found and fallback disabled")
  endif()

  message(STATUS "[Fallback] Fetching stb (stb_image)")
  include(FetchContent)
  FetchContent_Declare(stb
    URL https://github.com/nothings/stb/archive/refs/heads/master.zip
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
  )
  FetchContent_MakeAvailable(stb)

  # stb 仓库是纯头文件，没有 CMake target，我们自己包装一个 INTERFACE
  add_library(stb_image INTERFACE)
  target_include_directories(stb_image INTERFACE
    ${stb_SOURCE_DIR}
  )

  add_library(stb::image ALIAS stb_image)
endfunction()
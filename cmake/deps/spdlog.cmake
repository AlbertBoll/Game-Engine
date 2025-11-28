include_guard(GLOBAL)
include(FetchContent)

# Options (cache so users can override with -D)
set(SPDLOG_VERSION "1.14.1" CACHE STRING "spdlog version to use when fetching")
option(SPDLOG_USE_EXTERNAL_FMT "Link spdlog against external fmt::fmt" OFF)
option(ENABLE_FETCH_FALLBACK "Allow FetchContent fallback if find_package fails" ON)
# set(SPDLOG_ACTIVE_LEVEL "SPDLOG_LEVEL_INFO" CACHE STRING "Compile-time active level for spdlog (e.g. SPDLOG_LEVEL_DEBUG)")

# Helper: try find_package(spdlog CONFIG)
function(_try_find_spdlog OUT_VAR)
  # CMakeDeps (Conan) or system/vcpkg usually provide 'spdlogConfig.cmake'
  find_package(spdlog QUIET CONFIG)
  if (spdlog_FOUND)
    set(${OUT_VAR} TRUE PARENT_SCOPE)
  else()
    set(${OUT_VAR} FALSE PARENT_SCOPE)
  endif()
endfunction()

# Public function to call from your ResolveDeps.cmake
function(fx_find_or_fetch_spdlog)
  if (TARGET spdlog::spdlog) # already available
    return()
  endif()

  _try_find_spdlog(_have)
  if (_have)
    message(STATUS "spdlog: found (package)")
  elseif (ENABLE_FETCH_FALLBACK)
    message(STATUS "spdlog: not found, fetching v${SPDLOG_VERSION}")
    # FetchContent
    FetchContent_Declare(spdlog
      GIT_REPOSITORY https://github.com/gabime/spdlog.git
      GIT_TAG        v${SPDLOG_VERSION}
      DOWNLOAD_EXTRACT_TIMESTAMP OFF
    )
    FetchContent_MakeAvailable(spdlog)
    # If using external fmt, set the define & link fmt before spdlog usage
    if (SPDLOG_USE_EXTERNAL_FMT)
      find_package(fmt REQUIRED)
      add_library(__spdlog_external_fmt INTERFACE)
      target_compile_definitions(__spdlog_external_fmt INTERFACE SPDLOG_FMT_EXTERNAL=1)
      target_link_libraries(__spdlog_external_fmt INTERFACE fmt::fmt spdlog::spdlog)
      add_library(spdlog::spdlog_external_fmt ALIAS __spdlog_external_fmt)
    endif()
  else()
    message(FATAL_ERROR "spdlog not found and ENABLE_FETCH_FALLBACK=OFF")
  endif()

  # Optional: set compile-time active level on a small interface target you can link
#   if (NOT TARGET spdlog::active_level)
#     add_library(spdlog::active_level INTERFACE)
#     target_compile_definitions(spdlog::active_level INTERFACE SPDLOG_ACTIVE_LEVEL=${SPDLOG_ACTIVE_LEVEL})
#   endif()
endfunction()
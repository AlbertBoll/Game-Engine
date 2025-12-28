include(FetchContent)

include(${CMAKE_CURRENT_LIST_DIR}/glm.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/entt.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/glfw.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/sdl3.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/glad2.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/imgui_docking.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/spdlog.cmake)

# Prefer Conan (find_package) first; otherwise FetchContent fallback
fx_find_or_fetch_glm()
fx_find_or_fetch_entt()
fx_find_or_fetch_spdlog()
if(CORE_WINDOW_BACKEND STREQUAL "GLFW")
  fx_find_or_fetch_glfw()
elseif(CORE_WINDOW_BACKEND STREQUAL "SDL3")
  fx_find_or_fetch_sdl3()
else()
  message(FATAL_ERROR "Unknown CORE_WINDOW_BACKEND='${CORE_WINDOW_BACKEND}' (SDL3|GLFW)")
endif()
fx_find_or_fetch_glad2()

if(ENABLE_IMGUI)
  fx_find_or_fetch_imgui_docking()
endif()
# System SDK provides OpenGL
find_package(OpenGL QUIET)


# cmake/deps/glm.cmake
# cmake
# Copy code
# function(fx_find_or_fetch_glm)
#   find_package(glm QUIET CONFIG) # glm::glm
#   if(NOT glm_FOUND)
#     if(NOT ENABLE_FETCH_FALLBACK)
#       message(FATAL_ERROR "glm not found and fallback disabled")
#     endif()
#     message(STATUS "[Fallback] Fetching glm 1.0.1")
#     FetchContent_Declare(glm
#       URL https://github.com/g-truc/glm/archive/refs/tags/1.0.1.zip
#       DOWNLOAD_EXTRACT_TIMESTAMP OFF
#       # URL_HASH SHA256=<REAL_SHA256>
#     )
#     FetchContent_MakeAvailable(glm)
#     if(NOT TARGET glm::glm)
#       add_library(glm::glm ALIAS glm)
#     endif()
#   endif()
# endfunction()
# Toggle automatic vendor fallback when Conan packages aren’t found
option(ENABLE_FETCH_FALLBACK "Allow FetchContent fallback when packages are missing" ON)

# Choose windowing backend once per build
set(CORE_BACKEND "SDL3" CACHE STRING "Graphics backend: GLFW or SDL3")
set_property(CACHE CORE_BACKEND PROPERTY STRINGS GLFW SDL3)

# Portable default for glad2 (macOS tops near 4.1)
if(APPLE)
  set(GLAD_API "gl:core=3.3" CACHE STRING "glad2 API (e.g., gl:core=3.3|4.6)")
else()
  set(GLAD_API "gl:core=4.6" CACHE STRING "glad2 API (e.g., gl:core=3.3|4.6)")
endif()

# Build Core as shared to hide backend link deps from App
option(BUILD_SHARED_CORE "Build Core as a shared library" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
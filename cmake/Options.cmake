# Toggle automatic vendor fallback when Conan packages aren’t found
option(ENABLE_FETCH_FALLBACK "Allow FetchContent fallback when packages are missing" ON)

# Choose windowing backend once per build
set(CORE_WINDOW_BACKEND "SDL3" CACHE STRING "Graphics backend: GLFW or SDL3")
set_property(CACHE CORE_WINDOW_BACKEND PROPERTY STRINGS GLFW SDL3)

# Portable default for glad2 (macOS tops near 4.1)
if(APPLE)
  set(GLAD_API "gl:core=3.3" CACHE STRING "glad2 API (e.g., gl:core=3.3|4.6)")
else()
  set(GLAD_API "gl:core=4.6" CACHE STRING "glad2 API (e.g., gl:core=3.3|4.6)")
endif()


# Choose STATIC vs SHARED for targets without an explicit type
# Build Core as shared to hide backend link deps from App
option(BUILD_SHARED_LIBS "Build libraries as shared" ON)

# Link scope for binary deps:
# - If Engine is STATIC → PUBLIC so App's final link gets them.
# - If Engine is SHARED → PRIVATE (Engine links them internally).
if(BUILD_SHARED_LIBS)
  set(_ENGINE_BIN_SCOPE PRIVATE)
else()
  set(_ENGINE_BIN_SCOPE PUBLIC)
endif()

# set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dear ImGui on/off
option(ENABLE_IMGUI "Build Dear ImGui with chosen backend" ON)
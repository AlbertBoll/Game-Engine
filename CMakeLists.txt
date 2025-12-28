cmake_minimum_required(VERSION 3.21)
project(GameEngine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/Options.cmake)

# Conan's CMakeDeps will make find_package(...) work when you ran conan install.
# If not found, our deps modules will fall back to FetchContent(if enabled)

# Helpful when building shared libs on Linux/macOS and mixing static deps:
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Resolve Dependencies
include(cmake/deps/ResolveDeps.cmake)

# Set centralized Target name
set(TARGET_ENGINE Engine CACHE STRING "Engine target name")
set(TARGET_APP    App    CACHE STRING "App target name")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(Engine)

# 1) Stable public aliases (everyone else links these, not the real names)
add_library(${PROJECT_NAME}::Engine ALIAS ${TARGET_ENGINE})


add_subdirectory(App)

add_executable(${PROJECT_NAME}::App  ALIAS ${TARGET_APP})




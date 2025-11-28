cmake_minimum_required(VERSION 3.20)
project(MyCMakeConanTest LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/Options.cmake)

# Conan's CMakeDeps will make find_package(...) work when you ran conan install.
# If not found, our deps modules will fall back to FetchContent(if enabled)


include(cmake/deps/ResolveDeps.cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_subdirectory(Engine)
add_subdirectory(App)



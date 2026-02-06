# set(_engine_type STATIC)
# if(BUILD_SHARED_LIBS)
#   set(_engine_type SHARED)
# endif()

# add_library(Engine ${_engine_type}
#   # ${CMAKE_CURRENT_SOURCE_DIR}/src/Core/Core.cpp
#   # ${CMAKE_CURRENT_SOURCE_DIR}/include/Core/Core.h
#   # ${CMAKE_CURRENT_SOURCE_DIR}/include/Core/Utility.h
#   # ${CMAKE_CURRENT_SOURCE_DIR}/include/Windows/Window.h
#   #include/Core/Core.hpp
# )

# BUILD_SHARED_LIBS control if the engine built as static lib or shared lib
add_library(${TARGET_ENGINE}) 
add_subdirectory(src/Core)
add_subdirectory(src/Windows)
add_subdirectory(src/Input)
add_subdirectory(src/Renderer)
add_subdirectory(src/Primitives)


if(CORE_WINDOW_BACKEND STREQUAL "SDL3")
  add_subdirectory(src/Platform/SDL3)
elseif(CORE_WINDOW_BACKEND STREQUAL "GLFW")
  add_subdirectory(src/Platform/GLFW)
endif()


target_include_directories(${TARGET_ENGINE} 
  PUBLIC 
    # ${CMAKE_CURRENT_SOURCE_DIR}/include/Engine_Public 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/Engine_Public>
    $<INSTALL_INTERFACE:include/Engine_Public>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Engine_Private 
)


# expose glm to public headers, propagate its include dirs
target_link_libraries(${TARGET_ENGINE} PUBLIC
  glm::glm
)

target_link_libraries(${TARGET_ENGINE} PRIVATE
  spdlog::spdlog_header_only
)

# # Link scope for binary deps:
# # - If Engine is STATIC → PUBLIC so App's final link gets them.
# # - If Engine is SHARED → PRIVATE (Engine links them internally).
# if(BUILD_SHARED_LIBS)
#   set(_ENGINE_BIN_SCOPE PRIVATE)
# else()
#   set(_ENGINE_BIN_SCOPE PUBLIC)
# endif()


# Keep the backend hidden: PRIVATE link deps
target_link_libraries(${TARGET_ENGINE} ${_ENGINE_BIN_SCOPE}
  EnTT::EnTT
  glad::glad
  imgui::imgui
  $<$<STREQUAL:${CORE_WINDOW_BACKEND},GLFW>:glfw>
  $<$<STREQUAL:${CORE_WINDOW_BACKEND},SDL3>:SDL3::SDL3>
  $<$<BOOL:${OpenGL_FOUND}>:OpenGL::GL>
)


# Platform GL fallbacks
if(WIN32 AND NOT OpenGL_FOUND)
  target_link_libraries(${TARGET_ENGINE} PRIVATE opengl32)
elseif(APPLE)
  target_link_libraries(${TARGET_ENGINE} PRIVATE "-framework OpenGL")
endif()

# Compile-time switch (private; doesn’t leak to App)
target_compile_definitions(${TARGET_ENGINE} PRIVATE
  $<$<STREQUAL:${CORE_WINDOW_BACKEND},GLFW>:USE_GLFW>
  $<$<STREQUAL:${CORE_WINDOW_BACKEND},SDL3>:USE_SDL3>
)
target_compile_definitions(${TARGET_ENGINE} PUBLIC GLM_ENABLE_EXPERIMENTAL)
# Optional: nice IDE folder
set_target_properties(${TARGET_ENGINE} PROPERTIES FOLDER "Engine")

# Visibility / export macros
if(WIN32)
  if(BUILD_SHARED_LIBS)
    target_compile_definitions(${TARGET_ENGINE} PRIVATE ENGINE_SHARED)   # dllexport
  else()
    target_compile_definitions(${TARGET_ENGINE} PUBLIC  ENGINE_STATIC)  # no dllimport
  endif()
else()
  # Hide non-API symbols on ELF/Mach-O
  set_target_properties(${TARGET_ENGINE} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES)
endif()

# unified outputs
set_target_properties(${TARGET_ENGINE} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY         "${CMAKE_BINARY_DIR}/bin"
  RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/bin"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin"
  LIBRARY_OUTPUT_DIRECTORY         "${CMAKE_BINARY_DIR}/lib"
  LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/lib"
  LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib"
  ARCHIVE_OUTPUT_DIRECTORY         "${CMAKE_BINARY_DIR}/lib"
  ARCHIVE_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/lib"
  ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib"
)
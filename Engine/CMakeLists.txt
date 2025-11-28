set(_engine_type STATIC)
if(BUILD_SHARED_CORE)
  set(_engine_type SHARED)
endif()

add_library(Engine ${_engine_type}
  # ${CMAKE_CURRENT_SOURCE_DIR}/src/Core/Core.cpp
  # ${CMAKE_CURRENT_SOURCE_DIR}/include/Core/Core.h
  # ${CMAKE_CURRENT_SOURCE_DIR}/include/Core/Utility.h
  # ${CMAKE_CURRENT_SOURCE_DIR}/include/Windows/Window.h
  #include/Core/Core.hpp
)

target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(src/Core)
add_subdirectory(src/Windows)

# Keep the backend hidden: PRIVATE link deps
target_link_libraries(Engine PRIVATE
  glm::glm
  EnTT::EnTT
  glad::glad
  imgui::imgui
  $<$<STREQUAL:${CORE_BACKEND},GLFW>:glfw>
  $<$<STREQUAL:${CORE_BACKEND},SDL3>:SDL3::SDL3>
  $<$<BOOL:${OpenGL_FOUND}>:OpenGL::GL>
)

# Platform GL fallbacks
if(WIN32 AND NOT OpenGL_FOUND)
  target_link_libraries(Engine PRIVATE opengl32)
elseif(APPLE)
  target_link_libraries(Engine PRIVATE "-framework OpenGL")
endif()

# Compile-time switch (private; doesn’t leak to App)
target_compile_definitions(Engine PRIVATE
  $<$<STREQUAL:${CORE_BACKEND},GLFW>:USE_GLFW>
  $<$<STREQUAL:${CORE_BACKEND},SDL3>:USE_SDL3>
)

# Optional: nice IDE folder
set_target_properties(Engine PROPERTIES FOLDER "Engine")
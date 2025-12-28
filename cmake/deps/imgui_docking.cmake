function(fx_find_or_fetch_imgui_docking)
  find_package(imgui-docking QUIET CONFIG) # provides imgui::imgui
  if(imgui-docking_FOUND)
    return()
  endif()

  if(NOT ENABLE_FETCH_FALLBACK)
    message(FATAL_ERROR "imgui-docking not found and fallback disabled")
  endif()

  message(STATUS "[Fallback] Fetching Dear ImGui (docking branch)")
  FetchContent_Declare(imgui
    URL https://github.com/ocornut/imgui/archive/refs/heads/docking.zip
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    # URL_HASH SHA256=<REAL_SHA256>
  )
  FetchContent_MakeAvailable(imgui)

  add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
  )
  target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
  )
  if(CORE_WINDOW_BACKEND STREQUAL "SDL3")
    target_sources(imgui PRIVATE ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp)
    target_link_libraries(imgui PRIVATE SDL3::SDL3) 
elseif(CORE_WINDOW_BACKEND STREQUAL "GLFW")
    target_sources(imgui PRIVATE ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp)
    target_link_libraries(imgui PRIVATE glfw)
  endif()
  target_link_libraries(imgui PRIVATE glad::glad)
  add_library(imgui::imgui ALIAS imgui)
endfunction()
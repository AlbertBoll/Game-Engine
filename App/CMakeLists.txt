add_executable(${TARGET_APP} main.cpp)
target_link_libraries(${TARGET_APP} PRIVATE ${PROJECT_NAME}::Engine)
set_target_properties(${TARGET_APP} PROPERTIES FOLDER "${TARGET_APP}")


# SDL3 on Windows often needs SDL3main for entry glue
if(WIN32 AND CORE_WINDOW_BACKEND STREQUAL "SDL3")
  if(TARGET SDL3::SDL3main)
    target_link_libraries(${TARGET_APP} PRIVATE SDL3::SDL3main)
  endif()
  # Avoid windows.h macro pollution
  target_compile_definitions(${TARGET_APP} PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

# Outputs next to Engine
set_target_properties(${TARGET_APP} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY         "${CMAKE_BINARY_DIR}/bin"
  RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/bin"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin"
)

# Let the exe find libEngine.so/.dylib from the build tree
if(UNIX AND NOT APPLE)
  set_target_properties(${TARGET_APP} PROPERTIES BUILD_RPATH "$ORIGIN")
elseif(APPLE)
  set_target_properties(${TARGET_APP} PROPERTIES BUILD_RPATH "@loader_path")
endif()

# Post-build: copy third-party DLLs next to App.exe (only if those targets exist)
if(WIN32)
  add_custom_command(TARGET ${TARGET_APP} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Copying runtime DLLs for $<TARGET_FILE_NAME:${TARGET_APP}> ..."
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:App> $<TARGET_FILE_DIR:App>
    COMMAND_EXPAND_LISTS
  )
endif()
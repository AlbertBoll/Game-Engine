add_executable(App main.cpp)
target_link_libraries(App PRIVATE Engine)
set_target_properties(App PROPERTIES FOLDER "Apps")
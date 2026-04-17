add_library(cg_physics INTERFACE)
target_sources(cg_physics INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/leap_frog.h)
target_link_libraries(cg_physics INTERFACE cg::project_options cg_debug cg::glad)
add_library(cg::physics ALIAS cg_physics)

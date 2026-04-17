add_library(
  cg_physics_scene
  STATIC
    angry_bunny_scene.h
    angry_bunny_scene.cc
)
target_link_libraries(
  cg_physics_scene
  PUBLIC
    cg::project_options
    cg_physics
    cg_renderer
    cg::glm
    cg::imgui
)
add_library(cg::physics_scene ALIAS cg_physics_scene)

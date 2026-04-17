add_library(
  cg_playground
  STATIC
    playground.h
    playground.cc
)
target_link_libraries(
  cg_playground
  PUBLIC
    cg::project_options
    cg_renderer
    cg_geometry_scene
    cg_physics_scene
    cg_renderer_scene
)
if(TARGET cg_rhi_vulkan)
  target_compile_definitions(cg_playground PUBLIC CG_HAS_VULKAN_RHI)
endif()
add_library(cg::playground ALIAS cg_playground)

add_executable(
  window
  window.cc
)
target_link_libraries(
  window
  PRIVATE
    cg::project_options
    cg_debug
    cg_rhi_gl
    cg_renderer_gl
    cg_playground
    cg::glad
    glfw
    cg::glm
    cg::imgui
)
if(TARGET cg_rhi_vulkan)
  target_link_libraries(window PRIVATE cg_rhi_vulkan)
endif()
if(CG_HAS_OSMESA)
  target_compile_definitions(window PRIVATE CG_HAS_OSMESA)
endif()
set_target_properties(
  window
  PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

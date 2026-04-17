add_library(
  cg_rhi
  STATIC
    device.h
    device.cc
    types.h
)
target_link_libraries(
  cg_rhi
  PUBLIC
    cg::project_options
    cg::glm
  PRIVATE
    cg_debug
)
add_library(cg::rhi ALIAS cg_rhi)

add_subdirectory(gl)
add_subdirectory(vulkan)

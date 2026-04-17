add_library(
  cg_rhi_gl
  STATIC
    device.cc
    types.h
)
target_link_libraries(
  cg_rhi_gl
  PUBLIC
    cg_rhi
  PRIVATE
    cg_base
    cg_proto
    cg::glad
)
add_library(cg::rhi_gl ALIAS cg_rhi_gl)
add_library(cg::rhi_opengl ALIAS cg_rhi_gl)

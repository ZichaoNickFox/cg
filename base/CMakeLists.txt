add_library(cg_debug INTERFACE)
target_sources(cg_debug INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/debug.h)
target_link_libraries(cg_debug INTERFACE cg::project_options glog::glog cg::glm)
add_library(cg::debug ALIAS cg_debug)

add_library(
  cg_base
  STATIC
    color.h
    geometry.h
    geometry.cc
    math.h
    math.cc
    util.h
    util.cc
)
target_link_libraries(cg_base PUBLIC cg::project_options cg_debug cg::eigen cg::glm fmt::fmt)
add_library(cg::base ALIAS cg_base)

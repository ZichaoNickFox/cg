add_library(
  cg_geometry
  STATIC
    plot.h
    plot.cc
)
target_link_libraries(cg_geometry PUBLIC cg::project_options cg_base cg::glad cg::implot)
add_library(cg::geometry ALIAS cg_geometry)

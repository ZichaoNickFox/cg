add_executable(geometry_plot_test plot_test.cc)
target_link_libraries(
  geometry_plot_test
  PRIVATE
    cg::geometry
    GTest::gtest
    GTest::gtest_main
)
add_test(NAME geometry_plot_test COMMAND geometry_plot_test)

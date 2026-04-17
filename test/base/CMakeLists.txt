add_executable(base_util_test util_test.cc)
target_link_libraries(
  base_util_test
  PRIVATE
    cg::base
    GTest::gtest
    GTest::gtest_main
)
add_test(NAME base_util_test COMMAND base_util_test)

add_executable(base_math_test math_test.cc)
target_link_libraries(
  base_math_test
  PRIVATE
    cg::base
    GTest::gtest
    GTest::gtest_main
)
add_test(NAME base_math_test COMMAND base_math_test)

add_executable(base_geometry_test geometry_test.cc)
target_link_libraries(
  base_geometry_test
  PRIVATE
    cg::base
    GTest::gtest
    GTest::gtest_main
)
add_test(NAME base_geometry_test COMMAND base_geometry_test)

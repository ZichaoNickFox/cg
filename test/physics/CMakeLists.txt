add_executable(physics_leap_frog_test leap_frog_test.cc)
target_link_libraries(
  physics_leap_frog_test
  PRIVATE
    cg::physics
    GTest::gtest
    GTest::gtest_main
)
add_test(NAME physics_leap_frog_test COMMAND physics_leap_frog_test)

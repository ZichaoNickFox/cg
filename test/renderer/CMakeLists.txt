add_executable(renderer_logic_test renderer_logic_test.cc)
target_link_libraries(
  renderer_logic_test
  PRIVATE
    cg::renderer
    GTest::gtest
    GTest::gtest_main
)
add_test(NAME renderer_logic_test COMMAND renderer_logic_test)

add_executable(renderer_state_test renderer_state_test.cc)
target_link_libraries(
  renderer_state_test
  PRIVATE
    cg::renderer
    GTest::gtest
    GTest::gtest_main
)
add_test(NAME renderer_state_test COMMAND renderer_state_test)

add_executable(renderer_buffer_test renderer_buffer_test.cc)
target_link_libraries(
  renderer_buffer_test
  PRIVATE
    cg::renderer
    GTest::gtest
    GTest::gtest_main
)
add_test(NAME renderer_buffer_test COMMAND renderer_buffer_test)

add_executable(renderer_mesh_test renderer_mesh_test.cc)
target_link_libraries(
  renderer_mesh_test
  PRIVATE
    cg::renderer
    GTest::gtest
    GTest::gtest_main
)
add_test(NAME renderer_mesh_test COMMAND renderer_mesh_test)

add_executable(rhi_logic_test rhi_logic_test.cc)
target_link_libraries(
  rhi_logic_test
  PRIVATE
    cg::rhi_gl
    cg::renderer_gl
    GTest::gtest
    GTest::gtest_main
)
add_test(NAME rhi_logic_test COMMAND rhi_logic_test)

if(TARGET cg_rhi_vulkan)
  add_executable(rhi_vulkan_test vulkan_device_test.cc)
  target_link_libraries(
    rhi_vulkan_test
    PRIVATE
      cg::rhi_vulkan
      cg::renderer
      GTest::gtest
      GTest::gtest_main
  )
  add_test(NAME rhi_vulkan_test COMMAND rhi_vulkan_test)
endif()

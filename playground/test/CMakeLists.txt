if(NOT BUILD_TESTING)
  return()
endif()

find_package(Python3 COMPONENTS Interpreter REQUIRED)

add_executable(eigen_test eigen_test.cc)
target_link_libraries(
  eigen_test
  PRIVATE
    cg::project_options
    cg_base
    cg_renderer
    cg::eigen
    GTest::gtest
    GTest::gtest_main
    cg::glad
    cg::glm
)
add_test(NAME eigen_test COMMAND eigen_test)

add_executable(geometry_test geometry_test.cc)
target_link_libraries(
  geometry_test
  PRIVATE
    cg::project_options
    cg_renderer
    cg::eigen
    GTest::gtest
    GTest::gtest_main
    cg::glad
    cg::glm
)
add_test(NAME geometry_test COMMAND geometry_test)

add_executable(renderer_test renderer_test.cc)
target_link_libraries(
  renderer_test
  PRIVATE
    cg::project_options
    cg_renderer
    GTest::gtest
    GTest::gtest_main
    cg::glad
    cg::glm
)
add_test(NAME renderer_test COMMAND renderer_test)

add_test(
  NAME runtime_smoke_opengl_native
  COMMAND
    ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime_smoke.py
    $<TARGET_FILE:window>
    ${PROJECT_SOURCE_DIR}
    opengl
    native
    "RHI Runtime API :"
    "Scene Renderer API :"
)
set_tests_properties(runtime_smoke_opengl_native PROPERTIES RUN_SERIAL TRUE)

if(TARGET cg_rhi_vulkan)
  add_test(
    NAME runtime_smoke_vulkan_native
    COMMAND
      ${Python3_EXECUTABLE}
      ${CMAKE_CURRENT_SOURCE_DIR}/runtime_smoke.py
      $<TARGET_FILE:window>
      ${PROJECT_SOURCE_DIR}
      vulkan
      native
      "Presentation Runtime : Vulkan"
      "RHI Runtime API : Vulkan"
      "Scene Renderer API :"
      "Offscreen GL Runtime : Hidden Native OpenGL"
  )
  set_tests_properties(runtime_smoke_vulkan_native PROPERTIES RUN_SERIAL TRUE)
endif()

if(CG_HAS_OSMESA)
  add_test(
    NAME runtime_smoke_opengl_mesa
    COMMAND
      ${Python3_EXECUTABLE}
      ${CMAKE_CURRENT_SOURCE_DIR}/runtime_smoke.py
      $<TARGET_FILE:window>
      ${PROJECT_SOURCE_DIR}
      opengl
      mesa
      "Presentation Runtime : OpenGL"
      "RHI Runtime API :"
      "Scene Renderer API :"
      "Offscreen GL Runtime : Mesa OSMesa"
  )
  set_tests_properties(runtime_smoke_opengl_mesa PROPERTIES RUN_SERIAL TRUE)

  if(TARGET cg_rhi_vulkan)
    add_test(
      NAME runtime_smoke_vulkan_mesa
      COMMAND
        ${Python3_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/runtime_smoke.py
        $<TARGET_FILE:window>
        ${PROJECT_SOURCE_DIR}
        vulkan
        mesa
        "Presentation Runtime : Vulkan"
        "RHI Runtime API : Vulkan"
        "Scene Renderer API :"
        "Offscreen GL Runtime : Mesa OSMesa"
    )
    set_tests_properties(runtime_smoke_vulkan_mesa PROPERTIES RUN_SERIAL TRUE)
  endif()
else()
  add_test(
    NAME runtime_smoke_opengl_mesa_fallback
    COMMAND
      ${Python3_EXECUTABLE}
      ${CMAKE_CURRENT_SOURCE_DIR}/runtime_smoke.py
      $<TARGET_FILE:window>
      ${PROJECT_SOURCE_DIR}
      opengl
      mesa
      "CG_GL_RUNTIME=mesa was requested, but this build does not have OSMesa support."
      "RHI Runtime API :"
      "Scene Renderer API :"
  )
  set_tests_properties(runtime_smoke_opengl_mesa_fallback PROPERTIES RUN_SERIAL TRUE)

  if(TARGET cg_rhi_vulkan)
    add_test(
      NAME runtime_smoke_vulkan_mesa_fallback
      COMMAND
        ${Python3_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/runtime_smoke.py
        $<TARGET_FILE:window>
        ${PROJECT_SOURCE_DIR}
        vulkan
        mesa
        "CG_GL_RUNTIME=mesa was requested, but this build does not have OSMesa support."
        "Presentation Runtime : Vulkan"
        "RHI Runtime API : Vulkan"
        "Scene Renderer API :"
        "Offscreen GL Runtime : Hidden Native OpenGL"
    )
    set_tests_properties(runtime_smoke_vulkan_mesa_fallback PROPERTIES RUN_SERIAL TRUE)
  endif()
endif()

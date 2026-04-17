if(NOT CG_ENABLE_VULKAN)
  message(STATUS "CG_ENABLE_VULKAN=OFF, skipping Vulkan RHI backend.")
  return()
endif()

find_package(Vulkan QUIET)
if(NOT Vulkan_FOUND)
  message(STATUS "Vulkan SDK not found, skipping Vulkan RHI backend.")
  return()
endif()

add_library(
  cg_rhi_vulkan
  STATIC
    device.cc
    presenter.cc
    presenter.h
    ${PROJECT_SOURCE_DIR}/thirdparty/imgui/backends/imgui_impl_vulkan.cpp
)
target_link_libraries(
  cg_rhi_vulkan
  PUBLIC
    cg_rhi
  PRIVATE
    cg_base
    cg_proto
    cg::rhi_gl
    cg::imgui
    glfw
    Vulkan::Vulkan
)
add_library(cg::rhi_vulkan ALIAS cg_rhi_vulkan)

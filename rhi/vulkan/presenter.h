#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

struct GLFWwindow;

namespace cg::rhi::vulkan {

struct PresenterFrame {
  int width = 0;
  int height = 0;
  const std::uint8_t* rgba_pixels = nullptr;
  std::size_t size_in_bytes = 0;
};

class Presenter {
 public:
  Presenter(int width, int height, const char* title);
  ~Presenter();

  Presenter(const Presenter&) = delete;
  Presenter& operator=(const Presenter&) = delete;

  GLFWwindow* window() const;
  bool ShouldClose() const;
  void Present(const PresenterFrame& frame);
  const char* runtime_name() const;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

bool IsSupported();

}  // namespace cg::rhi::vulkan

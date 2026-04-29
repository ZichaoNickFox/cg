#include "rhi/device.h"

#include "base/debug.h"

namespace cg::rhi {
namespace {
std::unique_ptr<Device> g_device;

const char* BackendTypeLabel(BackendType backend) {
  switch (backend) {
    case BackendType::kUnknown:
      return "Unknown";
    case BackendType::kOpenGL:
      return "OpenGL";
    case BackendType::kVulkan:
      return "Vulkan";
    case BackendType::kMetal:
      return "Metal";
  }
  return "Unknown";
}
}

bool HasDevice() {
  return g_device != nullptr;
}

void SetDevice(std::unique_ptr<Device> device) {
  if (device == nullptr) {
    CGLOG(ERROR) << "RHI SetDevice: clearing active device";
    g_device = nullptr;
    return;
  }
  const Capabilities& caps = device->capabilities();
  CGLOG(ERROR) << "RHI SetDevice: backend=" << BackendTypeLabel(caps.backend)
               << " graphics_api=" << (caps.graphics_api_name.empty() ? "Unknown" : caps.graphics_api_name)
               << " scene_backend=" << BackendTypeLabel(caps.scene_backend == BackendType::kUnknown ? caps.backend
                                                                                                     : caps.scene_backend)
               << " scene_api=" << (caps.scene_api_name.empty() ? caps.graphics_api_name : caps.scene_api_name)
               << " shader_language=" << (caps.shader_language_name.empty() ? "Unknown" : caps.shader_language_name)
               << " glsl450=" << (caps.supports_glsl_450 ? "yes" : "no")
               << " compute=" << (caps.supports_compute ? "yes" : "no")
               << " storage_buffers=" << (caps.supports_storage_buffers ? "yes" : "no");
  g_device = std::move(device);
}

Device& GetDevice() {
  return *CGCHECK_NOTNULL(g_device.get());
}

const Capabilities& GetCapabilities() {
  return GetDevice().capabilities();
}

BackendType GetSceneBackendType() {
  const Capabilities& caps = GetCapabilities();
  return caps.scene_backend == BackendType::kUnknown ? caps.backend : caps.scene_backend;
}

std::string GetSceneApiName() {
  const Capabilities& caps = GetCapabilities();
  return caps.scene_api_name.empty() ? caps.graphics_api_name : caps.scene_api_name;
}

}  // namespace cg::rhi

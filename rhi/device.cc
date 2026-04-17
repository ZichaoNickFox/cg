#include "rhi/device.h"

#include "base/debug.h"

namespace cg::rhi {
namespace {
std::unique_ptr<Device> g_device;
}

bool HasDevice() {
  return g_device != nullptr;
}

void SetDevice(std::unique_ptr<Device> device) {
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

#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <vector>

#include "renderer/texture.h"
#include "test/test_support/fake_device.h"

namespace cg::test {

class FakeRendererDevice final : public FakeDevice {
 public:
  using FakeDevice::FakeDevice;

  void EnsureTextureUploaded(Texture* texture) override {
    std::shared_ptr<Texture::Storage> storage = texture->storage();
    if (storage == nullptr) {
      return;
    }
    if (storage->id == std::numeric_limits<uint32_t>::max()) {
      storage->id = ++next_texture_id_;
    }
    storage->uploaded_to_gl = true;
  }

  void ReleaseTexture(Texture* texture) override {
    std::shared_ptr<Texture::Storage> storage = texture->storage();
    if (storage == nullptr) {
      return;
    }
    storage->id = std::numeric_limits<uint32_t>::max();
    storage->uploaded_to_gl = false;
  }

  void ReadTextureData(const Texture& texture, int level, void* data, size_t size_in_bytes) override {
    std::shared_ptr<Texture::Storage> storage = texture.storage();
    if (storage == nullptr || data == nullptr || level < 0 ||
        storage->cpu_levels.size() <= static_cast<size_t>(level)) {
      return;
    }
    const std::vector<uint8_t>& source = storage->cpu_levels[static_cast<size_t>(level)];
    const size_t bytes_to_copy = std::min(size_in_bytes, source.size());
    if (bytes_to_copy > 0) {
      std::memcpy(data, source.data(), bytes_to_copy);
    }
    if (size_in_bytes > bytes_to_copy) {
      std::memset(static_cast<uint8_t*>(data) + bytes_to_copy, 0, size_in_bytes - bytes_to_copy);
    }
  }

 private:
  uint32_t next_texture_id_ = 0;
};

class ScopedFakeRendererDevice {
 public:
  explicit ScopedFakeRendererDevice(rhi::Capabilities capabilities = FakeDevice::DefaultCapabilities()) {
    auto device = std::make_unique<FakeRendererDevice>(std::move(capabilities));
    device_ = device.get();
    rhi::SetDevice(std::move(device));
  }

  ~ScopedFakeRendererDevice() { rhi::SetDevice(std::unique_ptr<rhi::Device>()); }

  FakeRendererDevice& device() { return *device_; }
  const FakeRendererDevice& device() const { return *device_; }

 private:
  FakeRendererDevice* device_ = nullptr;
};

}  // namespace cg::test

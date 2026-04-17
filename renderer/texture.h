#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "glm/glm.hpp"

#include "base/debug.h"
#include "renderer/config.h"
#include "rhi/types.h"

namespace cg {
/*
struct Texture2DData {
  Texture2DData() = default;
  Texture2DData(int level_num, int level0_buffer_size);
  void UpdateData(int level, const std::vector<GLubyte>& data) { data_[level] = data; }
  GLubyte* mutable_data(int level) { return data_[level].data(); }
  const void* data(int level) const { return data_[level].data(); }
  void resize(int level_num, int level0_buffer_size);
  uint32_t level_num() const { return data_.size(); }

 private:
  std::vector<std::vector<GLubyte>> data_;
};

struct CubemapData {
  CubemapData(int level_num, int level0_buffer_size);
  void UpdateData(int face, int level, const std::vector<GLubyte>& data) { data_[face].UpdateData(level, data); }
  GLubyte* mutable_data(int face, int level) { return data_[face].mutable_data(level); }
 private:
  std::array<Texture2DData, 6> data_;
};
*/

class Texture {
 public:
  enum Type{
    kUnknown = 0,
    kTexture2D = 1,
    kCubemap = 2,
    kTexture2DArray = 3,
  };
  // filter && wrap : https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexParameter.xhtml
  struct Meta {
    Texture::Type type = Texture::kUnknown;
    int width = -1;
    int height = -1;
    int channel_num = -1; // Force 4 channels
    bool hdr = false;
    int level_num = -1;
    int depth = -1; // for texture3d

    rhi::TextureFormat format = rhi::TextureFormat::kUnknown;
    rhi::PixelFormat pixel_format = rhi::PixelFormat::kUnknown;
    rhi::PixelType pixel_type = rhi::PixelType::kUnknown;
    rhi::FilterMode min_filter = rhi::FilterMode::kNearest;
    rhi::FilterMode mag_filter = rhi::FilterMode::kNearest;
    rhi::WrapMode wrap_s = rhi::WrapMode::kRepeat;
    rhi::WrapMode wrap_t = rhi::WrapMode::kRepeat;

    bool Varify() const;
    int data_size_in_byte(int level = 0) const;
    int resized_data_size_in_byte(int resized_width, int resized_height, int level = 0) const;
    glm::vec2 Resolution() const { return glm::vec2{width, height}; }

    bool operator==(const Meta& other) const = default;
  };

  struct Storage {
    mutable uint32_t id = std::numeric_limits<uint32_t>::max();
    mutable bool uploaded_to_gl = false;
    bool owns_gl_texture = true;
    std::vector<std::vector<uint8_t>> cpu_levels;
  };

  Texture() = default;
  explicit Texture(const Texture::Meta& meta);
  Texture(uint32_t id, const Texture::Meta& meta);
  Texture(uint32_t id, const Texture::Meta& meta, const std::string& info);

  bool empty() const;
  uint32_t id() const;
  void SetInfo(const std::string& info) { info_ = info; }
  std::string info() const { return info_; }
  template<typename ChannelType>
  std::vector<ChannelType> GetData() const;
  const Meta& meta() const { return meta_; }
  const std::shared_ptr<Storage>& storage() const { return storage_; }
  int data_size_in_byte(int level = 0) const { return meta_.data_size_in_byte(level); }
  bool Varify() const;
  void ReleaseGpuResources() const;
  void SetCpuLevelData(int level, const void* data, size_t size_in_bytes);
  void SetCpuFaceLevelData(int face, int level, const void* data, size_t size_in_bytes);

  bool operator==(const Texture& other) const = default;

 private:
  void EnsureUploadedToActiveBackend() const;

  std::shared_ptr<Storage> storage_;
  Meta meta_;
  std::string info_;
};

Texture CreateTexture2D(const Texture::Meta& meta, const std::vector<void*>& datas);
Texture CreateCubemap(const Texture::Meta& meta, const std::vector<void*>& datas);
Texture CreateTexture2D(int width, int height, const std::vector<glm::vec4>& data,
                        rhi::FilterMode min_filter = rhi::FilterMode::kLinear,
                        rhi::FilterMode mag_filter = rhi::FilterMode::kLinear,
                        rhi::WrapMode wrap_s = rhi::WrapMode::kRepeat,
                        rhi::WrapMode wrap_t = rhi::WrapMode::kRepeat);
void SaveTexture(const std::string& file_name, const Texture& texture);
Texture ReadTexture(const std::string& fullpath);

class TextureRepo {
  // Maybe deprecated
 public:
  void Init(const Config& config);
  // void SaveTexture2D(const std::string& name);
  // void SaveCubemap(const std::string& name);
  // void ResetTexture2D(const std::string& name, const TextureParam& param);
  // void ResetCubemap(const std::string& name, const CubemapParam& param);
  // Texture CreateCubemapPreviewTexture2D(const CubemapParam& param);

 public:
  int AddUnique(const std::string& full_path);
  int AddUnique(const std::string& name, const Texture& texture);
  bool Has(const std::string& path) const;
  bool Has(int index) const;
  int GetIndex(const std::string& name) const;
  const Texture& GetTexture(const std::string& name) const;
  const Texture& GetTexture(int index) const;
  Texture AsTexture2DArray(int width = 512, int height = 512) const;
  int size() const;
  void MergeIamge() const;

 private:
  void EnsureTextureLoaded(const std::string& name) const;
  Texture LoadTextureFromConfig(const TextureConfig& texture_config) const;

  mutable std::unordered_map<std::string, int> name_2_index_;
  mutable std::unordered_map<int, Texture> index_2_texture_;
  mutable std::unordered_map<int, Texture> dirty_index_2_texture_;

  const Config* config_ = nullptr;
  mutable Texture texture_2d_array_;
};
} // namespace cg

#pragma once

#include <array>
#include <limits>
#include <string>
#include <vector>

#include "glm/glm.hpp"

#include "renderer/config.h"
#include "renderer/debug.h"
#include "renderer/gl.h"

namespace renderer {
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

    int gl_internal_format = -1; // GL_RGBA32F
    int gl_format = -1;   // GL_RGBA
    int gl_type = -1;     // GL_FLOAT
    int gl_min_filter = -1; // GL_LINEAR GL_NEAREST
    int gl_mag_filter = -1;
    int gl_wrap_s = -1;  // GL_REPEAT GL_MIRRORED_REPEAT GL_CLAMP_TO_EDGE GL_CLAMP_TO_BORDER
    int gl_wrap_t = -1;

    bool Varify() const;
    int data_size_in_byte(int level = 0) const;
    int resized_data_size_in_byte(int resized_width, int resized_height, int level = 0) const;
    glm::vec2 Resolution() const { return glm::vec2{width, height}; }

    bool operator==(const Meta& other) const = default;
  };

  Texture() {}
  Texture(const Texture::Meta& meta) : meta_(meta) {}
  Texture(const GLuint& id, const Texture::Meta& meta) : id_(id), meta_(meta){}
  Texture(const GLuint& id, const Texture::Meta& meta, const std::string& info) : id_(id), meta_(meta), info_(info) {}

  bool empty() const { return id_ == std::numeric_limits<GLuint>::max(); }
  const GLuint& id() const;
  GLuint* mutable_id() { return &id_; }
  void SetInfo(const std::string& info) { info_ = info; }
  std::string info() const { return info_; }
  template<typename ChannelType>
  std::vector<ChannelType> GetData() const;
  const Meta& meta() const { return meta_; }
  int data_size_in_byte(int level = 0) const { return meta_.data_size_in_byte(); }
  bool Varify() const;

  bool operator==(const Texture& other) const = default;

 private:
  GLuint id_ = std::numeric_limits<GLuint>::max();
  Meta meta_;
  std::string info_;
};

template<typename ChannelType>
std::vector<ChannelType> Texture::GetData() const {
  meta_.Varify();
  std::vector<ChannelType> data;
  glBindTexture_(GL_TEXTURE_2D, id_);
  data.resize(data_size_in_byte());
  glGetTexImage_(GL_TEXTURE_2D, 0, meta_.gl_format, meta_.gl_type, data.data());
  return data;
}

Texture CreateTexture2D(const Texture::Meta& meta, const std::vector<void*>& datas);
Texture CreateTexture2D(int width, int height, const std::vector<glm::vec4>& data, GLuint min_filter = GL_LINEAR,
                        GLuint mag_filter = GL_LINEAR, GLuint wrap_s = GL_REPEAT, GLuint wrap_t = GL_REPEAT);

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
  Texture AsTextureRepo(int width = 512, int height = 512) const;
  int size() const;
  void MergeIamge() const;

 private:
  std::unordered_map<std::string, int> name_2_index_;
  std::unordered_map<int, Texture> index_2_texture_;
  mutable std::unordered_map<int, Texture> dirty_index_2_texture_;

  const Config* config_;
  mutable Texture texture_repo_;
};
} // namespace renderer
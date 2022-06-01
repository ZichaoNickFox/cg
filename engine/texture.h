#pragma once

#include <array>
#include <limits>
#include <string>
#include <vector>

#include "glm/glm.hpp"

#include "engine/debug.h"
#include "engine/gl.h"

namespace engine {
void FlipVertically(GLubyte* pixels, int width, int height, int channel, int byte_per_channel);
void GetInternalFormatSize(int internal_format, int* channel, int* byte_per_channel, int* format, int* type);

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

struct TextureParam {
 public:
  TextureParam(int in_width, int in_height, const std::vector<glm::vec3>& in_raw_data,
               GLuint in_min_filter = GL_LINEAR, GLuint in_mag_filter = GL_LINEAR,
               GLuint in_wrap_s = GL_REPEAT, GLuint in_wrap_t = GL_REPEAT);
  TextureParam(int in_width, int in_height, const std::vector<glm::vec4>& in_raw_data,
               GLuint in_min_filter = GL_LINEAR, GLuint in_mag_filter = GL_LINEAR,
               GLuint in_wrap_s = GL_REPEAT, GLuint in_wrap_t = GL_REPEAT);
  const void* texture_data(int level) const;

  int level_num;
  int width;
  int height;
  GLuint internal_format = GL_RGBA8;
  GLuint format = GL_RGBA;
  GLuint type = GL_UNSIGNED_BYTE;
  GLuint min_filter = GL_LINEAR;
  GLuint mag_filter = GL_LINEAR;
  GLuint wrap_s = GL_REPEAT;
  GLuint wrap_t = GL_REPEAT;
  std::vector<const void*> raw_data;
  const Texture2DData* full_data;
};

struct CubemapParam {
  int level_num;
  int width;
  int height;
  CubemapData* data;
  GLuint internal_format = GL_RGBA8;
  GLuint format = GL_RGBA;
  GLuint type = GL_UNSIGNED_BYTE;
  GLuint min_filter = GL_LINEAR;
  GLuint mag_filter = GL_LINEAR;
  GLuint wrap_s = GL_REPEAT;
  GLuint wrap_t = GL_REPEAT;
};

class Texture {
 public:
  enum Type{
    Texture2D = 0,
    Cubemap = 1,
  };

  Texture() {}
  Texture(GLuint inid, Type in) : id_(inid), textureType_(in){}
  Texture(GLuint inid, Type in, const std::string& info) : id_(inid), textureType_(in), info_(info) {}

  GLuint id() const;
  GLuint* mutable_id() { return &id_; }
  Type type() const  {return textureType_;}
  void SetInfo(const std::string& info) { info_ = info; }
  std::string info() const { return info_; }
  GLuint internal_format() const;

 private:
  GLuint id_ = std::numeric_limits<GLuint>::max();
  Type textureType_ = Texture2D;
  std::string info_;
};
} // namespace engine
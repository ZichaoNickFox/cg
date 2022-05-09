#include "engine/texture.h"

#include <glog/logging.h>
#include <math.h>
#include <unordered_map>

#include "engine/debug.h"

namespace engine {

GLuint Texture::id() const {
  CGCHECK(id_ != std::numeric_limits<GLuint>::max()) << "Texture used before assign id";
  return id_;
}

Texture2DData::Texture2DData(int level_num, int level0_buffer_size) {
  resize(level_num, level0_buffer_size);
}

std::vector<GLubyte>* Texture2DData::mutable_vector(int level) {
  CGCHECK(level < data_.size());
  return &data_[level];
}

GLubyte* Texture2DData::mutable_data(int level) {
  CGCHECK(level < data_.size());
  return data_[level].data();
}

void Texture2DData::resize(int level_num, int level0_buffer_size) {
  data_.resize(level_num);
  for (int level = 0; level < level_num; ++level) {
    int level_size = level0_buffer_size / int(std::pow(4, level));
    data_[level].resize(level_size);
    data_[level].assign(level_size, 0);
  }
}

CubemapData::CubemapData(int level_num, int level0_buffer_size) {
  for (Texture2DData& face : data_) {
    face.resize(level_num, level0_buffer_size);
  }
}

std::vector<GLubyte>* CubemapData::mutable_vector(int face, int level) {
  return data_[face].mutable_vector(level);
}

GLubyte* CubemapData::mutable_data(int face, int level) {
  return data_[face].mutable_data(level);
}

void FlipVertically(GLubyte* pixels, int width, int height, int channel, int byte_per_channel) {
  int byte_per_pixel = channel * byte_per_channel;
  int byte_per_line = width * byte_per_pixel;
  int h1 = height - 1;
  int h2 = 0;
  while (h2 < h1) {
    int h1_first_type = h1 * byte_per_line;
    int h2_first_type = h2 * byte_per_line;
    for (int byte_in_line = 0; byte_in_line < byte_per_line; ++ byte_in_line) {
      std::swap(pixels[h1_first_type + byte_in_line], pixels[h2_first_type + byte_in_line]);
    }
    h2++;
    h1--;
  }
}
void GetInternalFormatSize(int internal_format, int* channel, int* byte_per_channel, int* format, int* type) {
  struct Info {
    int channel;
    int byte_per_channel;
    int format;
    int type;
    std::string log;
  };
  // GL Format
  // https://www.cs.uregina.ca/Links/class-info/315/WWW/Lab5/InternalFormats_OGL_Core_3_2.html
  std::unordered_map<GLuint, Info> info_map = {
    {GL_DEPTH_COMPONENT, {1, 1, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, "GL_DEPTH_COMPONENT"}},
    {GL_DEPTH_STENCIL, {2, 1, GL_DEPTH_STENCIL, GL_UNSIGNED_BYTE, "GL_DEPTH_STENCIL"}},
    {GL_RED, {1, 1, GL_RED, GL_UNSIGNED_BYTE, "GL_RED"}},
    {GL_RG, {2, 2, GL_RG, GL_UNSIGNED_BYTE, "GL_RG"}},
    {GL_RGB, {2, 2, GL_RGB, GL_UNSIGNED_BYTE, "GL_RGB"}},
    {GL_RGB32F, {3, 4, GL_RGB, GL_FLOAT, "GL_RGB32F"}},
    {GL_RGB8, {3, 1, GL_RGB, GL_UNSIGNED_BYTE, "GL_RGB8"}},
    {GL_RGBA, {4, 4, GL_RGBA, GL_UNSIGNED_BYTE, "GL_RGBA"}},
    {GL_RGBA8, {4, 1, GL_RGBA, GL_UNSIGNED_BYTE, "GL_RGBA8"}},
    {GL_RGBA32F, {4, 4, GL_RGBA, GL_FLOAT, "GL_RGBA32F"}},
    {GL_DEPTH_COMPONENT32F, {4, 4, GL_DEPTH_COMPONENT, GL_FLOAT, "GL_DEPTH_COMPONENT32F"}},
  };
  CGCHECK(info_map.count(internal_format) > 0)
      << "Get Internal Format Size Failed : internal_format - " << internal_format;
  const Info info = info_map[internal_format];
  *channel = info.channel;
  *byte_per_channel = info.byte_per_channel;
  *format = info.format;
  *type = info.type;
}
} // namespace engine
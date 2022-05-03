#pragma once

#include <array>
#include <string>
#include <vector>
#include "GL/glew.h"

namespace engine {
void FlipVertically(GLubyte* pixels, int width, int height, int channel, int byte_per_channel);
void GetInternalFormatSize(int internal_format, int* channel, int* byte_per_channel, int* format, int* type);

struct Texture2DData {
  Texture2DData() = default;
  Texture2DData(int level_num, int level0_buffer_size);
  std::vector<GLubyte>* mutable_vector(int level);
  GLubyte* mutable_data(int level);
  void resize(int level_num, int level0_buffer_size);
 private:
  std::vector<std::vector<GLubyte>> data_;
};
struct CubemapData {
  CubemapData(int level_num, int level0_buffer_size);
  std::vector<GLubyte>* mutable_vector(int face, int level);
  GLubyte* mutable_data(int face, int level);
 private:
  std::array<Texture2DData, 6> data_;
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

  GLuint id() const { return id_; }
  GLuint* mutable_id() { return &id_; }
  Type type() const  {return textureType_;}
  void SetInfo(const std::string& info) { info_ = info; }
  std::string info() const { return info_; }

 private:
  GLuint id_;
  Type textureType_ = Texture2D;
  std::string info_;
};
} // namespace engine
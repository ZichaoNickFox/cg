#pragma once

#include <unordered_map>
#include <vector>

#include "renderer/config.h"
#include "renderer/gl.h"

/*
namespace renderer {
class Texture {
 public:
  Texture(int width, int height, const std::vector<unsigned char>& data);
  int width() { return width_; }
  int height() { return height_; }
  int channels() { return channels_; }
  std::vector<unsigned char> data();

 private:
  int width_;
  int height_;
  int channels_;
  GLuint buffer_;
};

class TextureRepo {
 public:
  ~TextureRepo();
  void Init(const Config& config);
  int AddUnique(const std::string& path);
  int AddUnique(const std::string& name, const Texture& texture);
  bool Has(const std::string& path) const;
  bool Has(int index) const;
  int AsTexture3D(int width = 2048, int height = 2048) const;
  int TextureNum() const;
  void MergeIamge() const;

 private: 
  std::unordered_map<std::string, int> name_2_index_;
  std::unordered_map<int, Texture> index_2_texture_;

  int tbo_ = -1;
  bool dirty_ = false;

  Config config_;
};

} // namespace renderer
*/
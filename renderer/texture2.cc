#include "renderer/texture.h"

/*
#include <algorithm>
#include <math.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

#include "renderer/debug.h"
#include "renderer/gl.h"
#include "renderer/util.h"

namespace renderer {
namespace {
int SizeInByte(int width, int height, int channels) {
  return width * height * channels;
}
std::vector<unsigned char> ReadImage(const std::string& filename, int* width, int* height, int* channels,
                                     int force_channels = 4) {
	std::vector<unsigned char> res = stbi_load(util::ReplaceBackslash(filename).c_str(), width, height,
                                             channels, force_channels);
  CGCHECK(res) << stbi_failure_reason() << " " << util::ReplaceBackslash(filename);
	return res;
}
std::vector<unsigned char> ResizeImage(const std::vector<unsigned char>& texture, int with, int height, int channels = 4) {
  std::vector<unsigned char> res;
  stbir_resize_uint8(texture.data.data(), texture.width, texture.height, 0
                     &res, width, height, 0, channels);
  return res;
}
void MergeImage(const std::vector<unsigned char>& texture, std::vector<unsigned char>* merged) {
  std::copy(texture.begin(), texture.end(), std::back_inserter(*merged));
}
}

void TextureRepo::Init(const Config& config) {
  config_ = config;
}

int TextureRepo::AddUnique(const std::string& path) {
  if (Has(path)) {
    return name_2_index_[path];
  }
  int index = index_2_texture_.size();
  name_2_index_[path] = index;
  index_2_texture_[index] = CreateTexture(path);
}

int TextureRepo::AddUnique(const std::string& name, const Texture& texture) {
  if (Has(name)) {
    return name_2_index_[name];
  }
  int index = index_2_texture_.size();
  name_2_index_[name] = index;
  index_2_texture_[index] = CreateTexture();
}

bool TextureRepo::Has(const std::string& path) const {
  CGCHECK(name_2_index_.size() == index_2_texture_.size());
  return name_2_index_.find(path) != name_2_index_.end();
}

bool TextureRepo::Has(int index) const {
  CGCHECK(name_2_index_.size() == index_2_texture_.size());
  return index_2_texture_.find(index) != index_2_texture_.end();
}

int TextureRepo::TextureNum() const {
  CGCHECK(name_2_index_.size() == index_2_texture_.size());
  return index_2_texture_.size();
}

int TextureRepo::AsTexture3D(int width = 2048, int height = 2048) const {
  if (dirty_) {
    if (tbo_ != -1) {
      glDeleteTextures_(1, &tbo_);
      tbo_ = -1;
    }
    std::vector<unsigned char> data;
    for (int i = 0; i < TextureNum(); ++i) {
      MergeImage(ResizeIamge(index_2_texture_[i]), &data);
    }
    glGenTextures_(1, &tbo_);
    glBindTexture_(GL_TEXTURE_2D_ARRAY, tbo_);
    glTexImage3D_(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, width, height,
                  TextureNum(), 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    glTexParameteri_(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri_(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture_(GL_TEXTURE_2D_ARRAY, 0);
    dirty_ = false;
  }
  return tbo_;
}

TextureRepo::~TextureRepo() {
  for (auto& p : index_2_texture_) {
    delete p.second.data;
  }
}
}
*/
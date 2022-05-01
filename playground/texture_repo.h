#pragma once

#include <array>
#include <unordered_map>

#include "engine/texture.h"
#include "playground/proto/config.pb.h"

namespace texture {
struct CreateTexture2DParam {
  int levels = 1;
  int width;
  int height;
  std::vector<GLubyte*> datas;
  int internal_format = GL_RGBA8;
  int format = GL_RGBA;
  int type = GL_UNSIGNED_BYTE;
};
struct CreateCubemapParam {
  int levels = 1;
  int width;
  int height;
  std::array<std::vector<GLubyte*>, 6> datas;
  int internal_format = GL_RGBA8;
  int format = GL_RGBA;
  int type = GL_UNSIGNED_BYTE;
};
engine::Texture LoadTexture2D(const std::string& path_with_ext, bool flip_vertically = true,
                              int mipmap_level_count = 1, bool equirectangular = false);
}

class TextureRepo {
 public:
  void Init(const Config& config);
  engine::Texture GetOrLoadTexture(const std::string& name, bool flip_vertically = false, int mipmap_level_count = 1);
  void SaveTexture(const std::string& name, const engine::Texture& texture);
  void SaveCubemap(const std::string& name, int face, const engine::Texture& cubemap_face_texture2d);
  void CreateTexture2D(const std::string& name, const texture::CreateTexture2DParam& param);
  void CreateCubemap(const std::string& name, const texture::CreateCubemapParam& param);

 private:
  struct State {
    engine::Texture texture;
    bool loaded = false;
    std::vector<std::string> paths;
    engine::Texture::Type texture_type;
  };
  std::unordered_map<std::string, State> textures_;
};
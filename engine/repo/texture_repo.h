#pragma once

#include <array>
#include "google/protobuf/map.h"
#include <unordered_map>

#include "engine/proto/config.pb.h"
#include "engine/texture.h"

namespace engine {
struct ResetTexture2DParam {
  int level_num;
  int width;
  int height;
  Texture2DData* data;
  int internal_format = GL_RGBA8;
  int format = GL_RGBA;
  int type = GL_UNSIGNED_BYTE;
};
struct ResetCubemapParam {
  int level_num;
  int width;
  int height;
  CubemapData* data;
  int internal_format = GL_RGBA8;
  int format = GL_RGBA;
  int type = GL_UNSIGNED_BYTE;
};
engine::Texture LoadModelTexture2D(const std::string& full_path, bool flip_vertically = false);

class TextureRepo {
 public:
  void Init(const Config& config);
  engine::Texture GetOrLoadTexture(const std::string& name, bool flip_vertically = false);
  void SaveTexture2D(const std::string& name);
  void SaveCubemap(const std::string& name);
  void ResetTexture2D(const std::string& name, const ResetTexture2DParam& param);
  void ResetCubemap(const std::string& name, const ResetCubemapParam& param);

 private:
  struct State {
    engine::Texture texture;
    bool loaded = false;
    int level_num = 1;
    std::unordered_map<std::string, std::string> paths;
    engine::Texture::Type texture_type;
  };
  std::unordered_map<std::string, State> textures_;
};
} // namespace engine
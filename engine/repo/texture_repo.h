#pragma once

#include <array>
#include "google/protobuf/map.h"
#include <unordered_map>

#include "engine/proto/config.pb.h"
#include "engine/texture.h"

namespace engine {
struct CreateTexture2DParam {
  int level_num;
  int width;
  int height;
  union Data {
    Data(const std::vector<void*>& in_raw_data) : raw_data(in_raw_data) {}
    Data(const Texture2DData& in_full_data) : full_data(&in_full_data) {}
    ~Data() {}
    std::vector<void*> raw_data;
    const Texture2DData* full_data;
  } data;
  int internal_format = GL_RGBA8;
  int format = GL_RGBA;
  int type = GL_UNSIGNED_BYTE;

  const void* texture_data(int level) const;
};
struct CreateCubemapParam {
  int level_num;
  int width;
  int height;
  CubemapData* data;
  int internal_format = GL_RGBA8;
  int format = GL_RGBA;
  int type = GL_UNSIGNED_BYTE;
};
engine::Texture LoadModelTexture2D(const std::string& full_path, bool flip_vertically = true);

class TextureRepo {
 public:
  void Init(const Config& config);
  Texture GetOrLoadTexture(const std::string& name, bool flip_vertically = true);
  void SaveTexture2D(const std::string& name);
  void SaveCubemap(const std::string& name);
  void ResetTexture2D(const std::string& name, const CreateTexture2DParam& param);
  void ResetCubemap(const std::string& name, const CreateCubemapParam& param);
  Texture CreateTempTexture2D(const CreateTexture2DParam& param);
  Texture CreateCubemapPreviewTexture2D(const CreateCubemapParam& param);

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
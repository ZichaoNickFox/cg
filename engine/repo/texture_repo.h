#pragma once

#include <array>
#include <glm/glm.hpp>
#include "google/protobuf/map.h"
#include <unordered_map>
#include <vector>

#include "engine/proto/config.pb.h"
#include "engine/texture.h"

namespace engine {
engine::Texture LoadModelTexture2D(const std::string& full_path, bool flip_vertically = true);

class TextureRepo {
 public:
  void Init(const Config& config);
  Texture GetOrLoadTexture(const std::string& name, bool flip_vertically = true);
  void SaveTexture2D(const std::string& name);
  void SaveCubemap(const std::string& name);
  void ResetTexture2D(const std::string& name, const TextureParam& param);
  void ResetCubemap(const std::string& name, const CubemapParam& param);
  Texture CreateTexture(const TextureParam& param);
  Texture CreateCubemapPreviewTexture2D(const CubemapParam& param);

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
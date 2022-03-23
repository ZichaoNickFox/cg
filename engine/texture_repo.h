#pragma once

#include <unordered_map>

#include "engine/config.pb.h"
#include "engine/texture.h"

namespace engine {
class TextureRepo {
 public:
  void Init(const Config& config);
  std::shared_ptr<Texture> GetOrLoadTexture(const std::string& name);

 private:
  struct State {
    std::shared_ptr<Texture> texture;
    bool loaded = false;
    std::vector<std::string> paths;
    Texture::Type texture_type;
  };
  std::unordered_map<std::string, State> textures_;
};
}
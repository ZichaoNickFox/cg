#pragma once

#include <vector>

#include <glog/logging.h>

#include "engine/shader.h"
#include "engine/texture.h"

namespace engine {

class Material {
 public:
  struct TextureLocation {
    std::shared_ptr<Texture> texture;
    std::string location;
  };
  void AddTextureLocation(const TextureLocation& texture_location);
  void SetShader(std::shared_ptr<Shader> shader) { shader_ = shader; }
  const std::vector<TextureLocation>& texture_location() { return texture_locations_; }
  std::shared_ptr<Shader> shader() const { return CHECK_NOTNULL(shader_); }

 private:
  std::vector<TextureLocation> texture_locations_;
  std::shared_ptr<Shader> shader_;
};

}
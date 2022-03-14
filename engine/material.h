#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>
#include <glog/logging.h>

#include "engine/shader.h"
#include "engine/texture.h"

namespace engine {

class Material {
 public:
  void SetShader(std::shared_ptr<Shader> shader) { shader_ = shader; }
  std::shared_ptr<Shader> shader() const { return CHECK_NOTNULL(shader_); }

  void SetLocationValue(const std::string& location, float value);
  void SetLocationValue(const std::string& location, const glm::mat4& value);
  void SetLocationValue(const std::string& location, int texture_unit, std::shared_ptr<Texture> value);
  void SetLocationValue(const std::string& location, const glm::vec4& value);
  void SetLocationValue(const std::string& location, const glm::vec3& value);

  void PrepareShader();

 private:
  std::shared_ptr<Shader> shader_;
  std::unordered_map<std::string, float> location_float_;
  std::unordered_map<std::string, glm::mat4> location_mat4_;
  struct TextureData {
    int texture_unit;
    std::shared_ptr<Texture> texture;
  };
  std::unordered_map<std::string, TextureData> location_texture_;
  std::unordered_map<std::string, glm::vec4> location_vec4_;
  std::unordered_map<std::string, glm::vec3> location_vec3_;
};

}
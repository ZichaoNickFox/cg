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
  void SetShader(Shader shader) { shader_ = shader; }
  const Shader& shader() const { return shader_; }

  void SetFloat(const std::string& location, float value);
  void SetMat4(const std::string& location, const glm::mat4& value);
  // return texture unit
  int SetTexture(const std::string& location, Texture value);
  void SetVec4(const std::string& location, const glm::vec4& value);
  void SetVec3(const std::string& location, const glm::vec3& value);

  void PrepareShader();

 private:
  Shader shader_;
  std::unordered_map<std::string, float> location_float_;
  std::unordered_map<std::string, glm::mat4> location_mat4_;
  struct TextureData {
    int texture_unit;
    Texture texture;
  };
  std::unordered_map<std::string, TextureData> location_texture_;
  std::unordered_map<std::string, glm::vec4> location_vec4_;
  std::unordered_map<std::string, glm::vec3> location_vec3_;
};

}
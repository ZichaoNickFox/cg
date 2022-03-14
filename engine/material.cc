#include "engine/material.h"

namespace engine {

void Material::SetLocationValue(const std::string& location, float value) {
  location_float_[location] = value;
}

void Material::SetLocationValue(const std::string& location, const glm::mat4& value) {
  location_mat4_[location] = value;
}

void Material::SetLocationValue(const std::string& location, int texture_unit, std::shared_ptr<Texture> value) {
  location_texture_[location] = TextureData{texture_unit, value};
}

void Material::PrepareShader() {
  CHECK(shader_);
  shader_->Use();
  for (const auto& pair : location_float_) {
    shader_->SetFloat(pair.first, pair.second);
  }
  for (const auto& pair : location_mat4_) {
    shader_->SetMat4(pair.first, pair.second);
  }
  for (const auto& pair : location_texture_) {
    // Attention : Here set texture unit rather than texture_id
    shader_->SetInt(pair.first, pair.second.texture_unit);
    glActiveTexture(GL_TEXTURE0 + pair.second.texture_unit);
    glBindTexture(GL_TEXTURE_2D, pair.second.texture->id());
  }
  for (const auto& pair : location_vec4_) {
    shader_->SetVec4(pair.first, pair.second);
  }
  for (const auto& pair : location_vec3_) {
    shader_->SetVec3(pair.first, pair.second);
  }
}

void Material::SetLocationValue(const std::string& location, const glm::vec4& value) {
  location_vec4_[location] = value;
}

void Material::SetLocationValue(const std::string& location, const glm::vec3& value) {
  location_vec3_[location] = value;
}

}
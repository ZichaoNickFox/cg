#include "engine/material.h"

#include "engine/debug.h"

namespace engine {

void Material::PushShader(Shader shader) {
  shader_datas_.push(shader);
}

void Material::PopShader() {
  CHECK(!shader_datas_.empty());
  shader_datas_.pop();
}

Material::ShaderData* Material::mutable_shader_data() {
  CHECK(!shader_datas_.empty());
  return &shader_datas_.top();
}

const Shader& Material::shader() const {
  CHECK(!shader_datas_.empty());
  return shader_data().shader_;
}

const Material::ShaderData& Material::shader_data() const {
  return shader_datas_.top();
}

void Material::SetFloat(const std::string& location, float value) {
  mutable_shader_data()->location_float_[location] = value;
}

void Material::SetMat4(const std::string& location, const glm::mat4& value) {
  mutable_shader_data()->location_mat4_[location] = value;
}

int Material::SetTexture(const std::string& location, Texture value) {
  std::unordered_map<std::string, ShaderData::TextureData>* texture_data = &mutable_shader_data()->location_texture_;
  int texture_unit = -1;
  if (texture_data->count(location) == 0) {
    texture_unit = (*texture_data).size();
    (*texture_data)[location] = ShaderData::TextureData{texture_unit, value};
  } else {
    texture_unit = (*texture_data)[location].texture_unit;
  }
  return texture_unit;
}

void Material::PrepareShader() {
  ShaderData* shader_data = mutable_shader_data();
  shader_data->shader_.Use();
  for (const auto& pair : shader_data->location_float_) {
    shader_data->shader_.SetFloat(pair.first, pair.second);
  }
  for (const auto& pair : shader_data->location_mat4_) {
    shader_data->shader_.SetMat4(pair.first, pair.second);
  }
  for (const auto& pair : shader_data->location_texture_) {
    // Attention : Here set texture unit rather than texture_id
    shader_data->shader_.SetInt(pair.first, pair.second.texture_unit);
    glActiveTexture(GL_TEXTURE0 + pair.second.texture_unit);
    if (pair.second.texture.type() == Texture::Texture2D) {
      glBindTexture(GL_TEXTURE_2D, pair.second.texture.id());
    } else if (pair.second.texture.type() == Texture::CubeMap) {
      glBindTexture(GL_TEXTURE_CUBE_MAP, pair.second.texture.id());
    }
  }
  for (const auto& pair : shader_data->location_vec4_) {
    shader_data->shader_.SetVec4(pair.first, pair.second);
  }
  for (const auto& pair : shader_data->location_vec3_) {
    shader_data->shader_.SetVec3(pair.first, pair.second);
  }
  for (const auto& pair : shader_data->location_int_) {
    shader_data->shader_.SetInt(pair.first, pair.second);
  }
}

void Material::SetVec4(const std::string& location, const glm::vec4& value) {
  mutable_shader_data()->location_vec4_[location] = value;
}

void Material::SetVec3(const std::string& location, const glm::vec3& value) {
  mutable_shader_data()->location_vec3_[location] = value;
}

void Material::SetInt(const std::string& location, int value) {
  mutable_shader_data()->location_int_[location] = value;
}

}
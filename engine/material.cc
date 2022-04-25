#include "engine/material.h"

#include "engine/debug.h"

namespace engine {

void Material::SetShader(Shader shader) {
  shader_ = shader;
  Clear();
}

const Shader& Material::shader() const {
  CGCHECK(HasShader());
  return shader_.value();
}

void Material::SetFloat(const std::string& location, float value) {
  location_float_[location] = value;
}

void Material::SetMat4(const std::string& location, const glm::mat4& value) {
  location_mat4_[location] = value;
}

int Material::SetTexture(const std::string& location, Texture value) {
  int texture_unit = -1;
  if (location_texture_.count(location) == 0) {
    int valid_texture2d_unit = 0;
    bool found_valid_texture_unit = true;
    for (; valid_texture2d_unit <= 31; ++valid_texture2d_unit) {
      found_valid_texture_unit = true;
      for (auto& p : location_texture_) {
        if (p.second.texture_unit == valid_texture2d_unit) {
          found_valid_texture_unit = false;
          break;
        }
      }
      if (found_valid_texture_unit)
        break;
    }
    int valid_cubemap_unit = 31;
    bool found_valid_cubemap_unit = true;
    for (; valid_cubemap_unit >= 0; --valid_cubemap_unit) {
      found_valid_cubemap_unit = true;
      for (auto& p : location_texture_) {
        if (p.second.texture_unit == valid_cubemap_unit) {
          found_valid_cubemap_unit = false;
          break;
        }
      }
      if (found_valid_cubemap_unit)
        break;
    }
    CGCHECK(valid_cubemap_unit > valid_texture2d_unit) << "A shader must have LE than 32 textures";
    texture_unit = (value.type() == engine::Texture::Texture2D ? valid_texture2d_unit : valid_cubemap_unit);
    location_texture_[location] = TextureData{texture_unit, value};
  } else {
    texture_unit = location_texture_[location].texture_unit;
  }
  return texture_unit;
}

void Material::PrepareShader() {
  CGCHECK(HasShader());
  shader_->Use();
  for (const auto& pair : location_float_) {
    shader_->SetFloat(pair.first, pair.second);
  }
  for (const auto& pair : location_mat4_) {
    shader_->SetMat4(pair.first, pair.second);
  }
  for (const auto& pair : location_texture_) {
    // Attention : Here set texture unit rather than texture_id
    const std::string& location = pair.first;
    const TextureData& texture_data = pair.second;
    shader_->SetInt(location, texture_data.texture_unit);
    glActiveTexture(GL_TEXTURE0 + texture_data.texture_unit);
    if (texture_data.texture.type() == Texture::Texture2D) {
      glBindTexture(GL_TEXTURE_2D, texture_data.texture.id());
    } else if (texture_data.texture.type() == Texture::Cubemap) {
      glBindTexture(GL_TEXTURE_CUBE_MAP, texture_data.texture.id());
    }
  }
  for (const auto& pair : location_vec4_) {
    shader_->SetVec4(pair.first, pair.second);
  }
  for (const auto& pair : location_vec3_) {
    shader_->SetVec3(pair.first, pair.second);
  }
  for (const auto& pair : location_int_) {
    shader_->SetInt(pair.first, pair.second);
  }
  for (const auto& pair : location_bool_) {
    shader_->SetBool(pair.first, pair.second);
  }
}

void Material::SetVec4(const std::string& location, const glm::vec4& value) {
  location_vec4_[location] = value;
}

void Material::SetVec3(const std::string& location, const glm::vec3& value) {
  location_vec3_[location] = value;
}

void Material::SetInt(const std::string& location, int value) {
  location_int_[location] = value;
}

void Material::SetBool(const std::string& location, bool value) {
  location_bool_[location] = value;
}

void Material::Clear() {
  location_float_.clear();
  location_mat4_.clear();
  location_texture_.clear();
  location_vec4_.clear();
  location_vec3_.clear();
  location_int_.clear();
  location_bool_.clear();
  shader_->Clear();
}
}
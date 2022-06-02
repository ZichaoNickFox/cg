#include "engine/compute_shader.h"

#include <set>

namespace engine {

void ComputeShader::Run() {
  CGCHECK(textures_.size() == texture_metas_.size());
  for (int i = 0; i < textures_.size(); ++i) {
    const Texture& texture = textures_[i];
    const TextureMeta& texture_meta = texture_metas_[i];

    CheckInternalFormat(texture);
    glActiveTexture_(GL_TEXTURE0 + texture_meta.texture_unit);
    glBindTexture_(GL_TEXTURE_2D, texture.id());

    glBindImageTexture_(texture_meta.texture_unit, texture.id(), 0, GL_FALSE, 0,
                        texture_meta.read_write_type, texture_meta.internal_format);
  }
  shader_.Use();
  glDispatchCompute_(work_group_num_.x, work_group_num_.y, work_group_num_.z);
  glMemoryBarrier_(GL_ALL_BARRIER_BITS);
}

void ComputeShader::SetTextures(const std::vector<engine::Texture>& textures) {
  textures_ = textures;
}

void ComputeShader::SetTextureMetas(const std::vector<TextureMeta>& texture_metas) {
  texture_metas_ = texture_metas;
}

void ComputeShader::SetWorkGroupNum(const glm::vec3& work_group_num) {
  work_group_num_ = work_group_num;
}

void ComputeShader::CheckInternalFormat(const engine::Texture& texture) {
  GLuint internal_format = texture.internal_format();
  std::set<GLint> supported_format = { GL_RGBA, GL_RGBA32F };
  if (supported_format.count(internal_format) <= 0) {
    CGCHECK(false) << "Unsupported Internal Format : " << std::hex << internal_format << std::dec;
  }
}

};
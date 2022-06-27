#include "renderer/compute_shader.h"

#include <glm/gtx/string_cast.hpp>
#include <set>

#include "renderer/bvh.h"
#include "renderer/geometry.h"
#include "renderer/util.h"

namespace renderer {

ComputeShader::ComputeShader(const Scene& scene, const std::string& shader_name) {
  shader_ = scene.shader_repo().GetShader(shader_name);
  shader_.Use();
  if (scene.texture_repo().size() > 0) {
    shader_.SetTexture("texture_repo", scene.texture_repo().AsTextureRepo());
  }
  shader_.SetInt("light_repo_length", scene.light_repo().length());
  shader_.SetInt("material_repo_length", scene.material_repo().length());
  shader_.SetInt("bvh_length", scene.bvh().length());
}

void ComputeShader::Run() const {
  CGCHECK(textures_.size() == texture_metas_.size());
  for (int i = 0; i < textures_.size(); ++i) {
    const Texture& texture = textures_[i];
    const TextureMeta& meta = texture_metas_[i];

    CheckInternalFormat(texture);
    glActiveTexture_(GL_TEXTURE0 + meta.texture_unit);
    glBindTexture_(GL_TEXTURE_2D, texture.id());

    glBindImageTexture_(meta.texture_unit, texture.id(), 0, GL_FALSE, 0,
                        meta.read_write_type, meta.internal_format);
  }
  shader_.Use();
  glDispatchCompute_(work_group_num_.x, work_group_num_.y, work_group_num_.z);
  glMemoryBarrier_(GL_ALL_BARRIER_BITS);
}

void ComputeShader::SetTextures(const std::vector<renderer::Texture>& textures) {
  textures_ = textures;
}

void ComputeShader::SetTextureMetas(const std::vector<TextureMeta>& texture_metas) {
  texture_metas_ = texture_metas;
}

void ComputeShader::SetWorkGroupNum(const glm::vec3& work_group_num) {
  work_group_num_ = work_group_num;
}

void ComputeShader::CheckInternalFormat(const renderer::Texture& texture) const {
  GLuint internal_format = texture.meta().gl_internal_format;
  std::set<GLint> supported_format = { GL_RGBA, GL_RGBA32F };
  if (supported_format.count(internal_format) <= 0) {
    CGCHECK(false) << "Unsupported Internal Format : " << std::hex << internal_format << std::dec;
  }
}

void ComputeShader::SetCamera(Camera* camera, bool with_view, bool with_project) {
  shader_.SetVec3("camera.pos_ws", camera->transform().translation());
  shader_.SetFloat("camera.far", camera->far_clip());
  if (with_view) {
    shader_.SetMat4("view", camera->GetViewMatrix());
  }
  if (with_project) {
    shader_.SetMat4("project", camera->GetProjectMatrix());
  }
}

void ComputeShader::SetSpheres(const std::vector<Sphere>& spheres) {
  for (int i = 0; i < spheres.size(); ++i) {
    const Sphere& sphere  = spheres[i];
    shader_.SetInt(fmt::format("spheres[{}].id", i), sphere.id);
    shader_.SetVec3(fmt::format("spheres[{}].center_pos", i), sphere.translation);
    shader_.SetVec4(fmt::format("spheres[{}].color", i), sphere.color);
    shader_.SetFloat(fmt::format("spheres[{}].radius", i), sphere.radius);
  }
}

void ComputeShader::SetScreenSize(const glm::vec2& screen_size) {
  shader_.SetVec2("screen_size", screen_size);
}

void ComputeShader::SetTimeSeed(int frame_num) {
  shader_.SetInt("frame_num", frame_num);
}

};
#include "renderer/render_shader.h"

#include "renderer/bvh.h"

namespace renderer {

RenderShader::RenderShader(const Scene& scene, const std::string& shader_name) {
  shader_ = scene.shader_repo().GetShader(shader_name);
  shader_.Use();
  if (scene.texture_repo().size() > 0) {
    shader_.SetTexture("texture_repo", scene.texture_repo().AsTextureRepo());
  }
  shader_.SetInt("light_repo_length", scene.light_repo().length());
  shader_.SetInt("material_repo_length", scene.material_repo().length());
  shader_.SetInt("bvh_length", scene.bvh().length());
}

void RenderShader::SetModel(const glm::mat4& model) {
  shader_.SetMat4("model", model);
}

void RenderShader::SetModel(const Object& object) {
  shader_.SetMat4("model", object.transform.GetModelMatrix());
}

void RenderShader::SetCamera(const Camera& camera) {
  shader_.SetVec3("camera.pos_ws", camera.transform().translation());
  shader_.SetMat4("camera.view", camera.GetViewMatrix());
  shader_.SetMat4("camera.project", camera.GetProjectMatrix());
  shader_.SetFloat("camera.u_near", camera.near_clip());
  shader_.SetFloat("camera.u_far", camera.far_clip());
}

void RenderShader::SetMaterialIndex(int material_index) {
  shader_.SetInt("material_index", material_index);
}

void RenderShader::Run(const Scene& scene, const Object& object) const {
  scene.mesh_repo().GetMesh(object.mesh_index)->Submit();
}

} // namespace renderer
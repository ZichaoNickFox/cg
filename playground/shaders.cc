#include "playground/shaders.h"

#include "engine/debug.h"
#include "playground/object/point_light.h"
#include "playground/util.h"

ShaderShadowInfo::ShaderShadowInfo(const glm::mat4& light_space_vp, const engine::Texture& depth_texture) {
  light_space_vp_ = light_space_vp;
  texture_depth = depth_texture;
}

void ShaderShadowInfo::UpdateMaterial(Context* context, engine::Material* material) const {
  material->SetBool("use_shadowing", true);
  material->SetMat4("shadow_info.light_space_vp", light_space_vp_);
  material->SetTexture("shadow_info.texture_depth", texture_depth);
}

ShaderLightInfo::ShaderLightInfo(const PointLight& point_light) {
  Insert(point_light);
}

ShaderLightInfo::ShaderLightInfo(const std::vector<PointLight>& point_lights) {
  for (const PointLight& point_light : point_lights) {
    Insert(point_light);
  }
}

void ShaderLightInfo::Insert(const PointLight& point_light) {
  light_poses.push_back(point_light.transform().translation());
  light_colors.push_back(point_light.color());
  light_attenuation_metres.push_back(point_light.attenuation_metre());
}

void ShaderLightInfo::UpdateMaterial(Context* context, engine::Material* material) const {
  CGCHECK(light_poses.size() == light_colors.size())
      << light_poses.size() << " " << light_colors.size();
  CGCHECK(light_poses.size() == light_attenuation_metres.size())
      << light_poses.size() << " " << light_attenuation_metres.size();
  for (int i = 0; i < light_poses.size(); ++i) {
    material->SetVec3(util::Format("lights[{}].color", i).c_str(), light_colors[i]);
    material->SetVec3(util::Format("lights[{}].pos", i).c_str(), light_poses[i]);
    int light_attenuation_metre = light_attenuation_metres[i];
    material->SetFloat(util::Format("lights[{}].constant", i).c_str(),
                                    context->light_attenuation_constant(light_attenuation_metre));
    material->SetFloat(util::Format("lights[{}].linear", i).c_str(),
                                    context->light_attenuation_linear(light_attenuation_metre));
    material->SetFloat(util::Format("lights[{}].quadratic", i).c_str(),
                                    context->light_attenuation_quadratic(light_attenuation_metre));
  }
  CHECK(light_poses.size()) << ": Need at least 1 light";
  material->SetInt("light_count", light_poses.size());
}

PhongShader::PhongShader(const Param& phong, Context* context, Object* object) {
  engine::Material* material = CGCHECK_NOTNULL(object->mutable_material(0));
  material->SetShader(context->GetShader("phong"));

  const engine::Camera& camera = context->camera();
  glm::mat4 project = camera.GetProjectMatrix();
  glm::mat4 view = camera.GetViewMatrix();
  material->SetMat4("project", project);
  material->SetMat4("view", view);
  material->SetMat4("model", object->GetModelMatrix());
  material->SetVec3("view_pos", context->camera().transform().translation());

  material->SetVec3("material.ambient", phong.ambient);
  material->SetVec3("material.diffuse", phong.diffuse);
  material->SetVec3("material.specular", phong.specular);
  material->SetFloat("material.shininess", phong.shininess);
  if (phong.texture_normal) {
    material->SetBool("material.use_texture_normal", true);
    material->SetTexture("material.texture_normal0", phong.texture_normal.value());
  } else {
    material->SetBool("material.use_texture_normal", false);
  }

  if (phong.texture_specular) {
    material->SetBool("material.use_texture_specular", true);
    material->SetTexture("material.texture_specular0", phong.texture_specular.value());
  } else {
    material->SetBool("material.use_texture_specular", false);
  }
  
  if (phong.texture_ambient) {
    material->SetBool("material.use_texture_ambient", true);
    material->SetTexture("material.texture_ambient0", phong.texture_ambient.value());
  } else {
    material->SetBool("material.use_texture_ambient", false);
  }
  
  if (phong.texture_diffuse) {
    material->SetBool("material.use_texture_diffuse", true);
    material->SetTexture("material.texture_diffuse0", phong.texture_diffuse.value());
  } else {
    material->SetBool("material.use_texture_diffuse", false);
  }

  phong.light_info.UpdateMaterial(context, material);
  if (phong.shadow_info) {
    phong.shadow_info->UpdateMaterial(context, material);
  }

  material->SetInt("blinn_phong", phong.use_blinn_phong);
}

PbrShader::PbrShader(const Param& pbr, Context* context, Object* object) {
  engine::Material* material = CGCHECK_NOTNULL(object->mutable_material(0));
  material->SetShader(context->GetShader("pbr"));

  const engine::Camera& camera = context->camera();
  glm::mat4 project = camera.GetProjectMatrix();
  glm::mat4 view = camera.GetViewMatrix();
  material->SetMat4("project", project);
  material->SetMat4("view", view);
  material->SetMat4("model", object->GetModelMatrix());
  material->SetVec3("view_pos", context->camera().transform().translation());

  material->SetVec3("material.albedo", pbr.albedo);
  material->SetFloat("material.roughness", pbr.roughness);
  material->SetFloat("material.metallic", pbr.metallic);

  material->SetFloat("ao", pbr.ao);

  pbr.light_info.UpdateMaterial(context, material);
}

NormalShader::NormalShader(const Param& normal, Context* context, Object* object) {
  engine::Material* material = CGCHECK_NOTNULL(object->mutable_material(0));
  material->SetShader(context->GetShader("normal"));
  const engine::Camera& camera = context->camera();
  glm::mat4 project = camera.GetProjectMatrix();
  glm::mat4 view = camera.GetViewMatrix();
  material->SetMat4("project", project);
  material->SetMat4("view", view);
  material->SetMat4("model", object->GetModelMatrix());
  material->SetFloat("line_length", normal.length);
  material->SetFloat("line_width", normal.width);
  material->SetVec3("view_pos", camera.transform().translation());
  material->SetBool("show_vertex_normal", normal.show_vertex_normal);
  material->SetBool("show_TBN", normal.show_TBN);
  material->SetBool("show_triangle", normal.show_triangle);
  material->SetBool("show_texture_normal", normal.show_texture_normal && normal.texture_normal);
  if (normal.texture_normal) {
    material->SetTexture("texture_normal", normal.texture_normal.value());
  }
}

LinesShader::LinesShader(const Param& param, Context* context, Object* object) {
  engine::Material* material = CGCHECK_NOTNULL(object->mutable_material(0));
  material->SetShader(context->GetShader("lines"));

  const engine::Camera& camera = context->camera();
  glm::mat4 project = camera.GetProjectMatrix();
  glm::mat4 view = camera.GetViewMatrix();
  glm::mat4 model = glm::mat4(1);
  material->SetMat4("project", project);
  material->SetMat4("view", view);
  material->SetMat4("model", model);
  material->SetVec3("view_pos", camera.transform().translation());
  material->SetFloat("line_width", param.line_width);
}

ColorShader::ColorShader(const Param& param, Context* context, Object* object) {
  engine::Material* material = CGCHECK_NOTNULL(object->mutable_material(0));
  material->SetShader(context->GetShader("color"));

  const engine::Camera& camera = context->camera();
  glm::mat4 project = camera.GetProjectMatrix();
  glm::mat4 view = camera.GetViewMatrix();
  glm::mat4 model = object->GetModelMatrix();
  material->SetMat4("project", project);
  material->SetMat4("view", view);
  material->SetMat4("model", model);
  material->SetVec4("color", param.color);
}

Texture0Shader::Texture0Shader(const Param& param, Context* context, Object* object) {
  engine::Material* material = CGCHECK_NOTNULL(object->mutable_material(0));
  material->SetShader(context->GetShader("texture0"));

  const engine::Camera& camera = context->camera();
  glm::mat4 project = camera.GetProjectMatrix();
  glm::mat4 view = camera.GetViewMatrix();
  glm::mat4 model = object->GetModelMatrix();
  material->SetMat4("project", project);
  material->SetMat4("view", view);
  material->SetMat4("model", model);
  material->SetTexture("texture0", param.texture0);
}

DepthBufferShader::DepthBufferShader(const engine::Shader& depth_buffer_shader,
                                     std::shared_ptr<const engine::Camera> camera, Object* object) {
  engine::Material* material = CGCHECK_NOTNULL(object->mutable_material(0));
  material->SetShader(depth_buffer_shader);

  glm::mat4 project = camera->GetProjectMatrix();
  glm::mat4 view = camera->GetViewMatrix();
  glm::mat4 model = object->GetModelMatrix();
  material->SetMat4("project", project);
  material->SetMat4("view", view);
  material->SetMat4("model", model);
}

SkyboxShader::SkyboxShader(const Param& param, Context* context, Object* object) {
  engine::Material* material = CGCHECK_NOTNULL(object->mutable_material(0));
  material->SetShader(context->GetShader("skybox"));

  glm::mat4 project = context->camera().GetProjectMatrix();
  glm::mat4 view = context->camera().GetViewMatrix();
  glm::mat4 model = object->GetModelMatrix();
  material->SetMat4("project", project);
  material->SetMat4("view", view);
  material->SetMat4("model", model); 
  material->SetTexture("Texture0", param.cube_texture);
}

FullScreenQuadShader::FullScreenQuadShader(const Param& param, Context* context, Object* object) {
  engine::Material* material = CGCHECK_NOTNULL(object->mutable_material(0));
  material->SetShader(context->GetShader("fullscreen_quad"));
  material->SetTexture("texture0", param.texture0); 
}
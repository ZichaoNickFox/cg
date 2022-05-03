#include "playground/shaders.h"

#include "engine/debug.h"
#include "playground/object/point_light.h"
#include "engine/util.h"

ShaderShadowInfo::ShaderShadowInfo(const glm::mat4& light_space_vp, engine::Texture depth_texture) {
  light_space_vp_ = light_space_vp;
  texture_depth_ = depth_texture;
}

void ShaderShadowInfo::UpdateMaterial(Context* context, engine::Material* material) const {
  material->SetBool("use_shadowing", true);
  material->SetMat4("shadow_info.light_space_vp", light_space_vp_);
  material->SetTexture("shadow_info.texture_depth", texture_depth_);
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

  material->SetVec3("phong_material.ambient", phong.ambient);
  material->SetVec3("phong_material.diffuse", phong.diffuse);
  material->SetVec3("phong_material.specular", phong.specular);
  material->SetFloat("phong_material.shininess", phong.shininess);
  if (phong.texture_normal) {
    material->SetBool("phong_material.use_texture_normal", true);
    material->SetTexture("phong_material.texture_normal0", phong.texture_normal.value());
  } else {
    material->SetBool("phong_material.use_texture_normal", false);
  }

  if (phong.texture_specular) {
    material->SetBool("phong_material.use_texture_specular", true);
    material->SetTexture("phong_material.texture_specular0", phong.texture_specular.value());
  } else {
    material->SetBool("phong_material.use_texture_specular", false);
  }

  if (phong.texture_ambient) {
    material->SetBool("phong_material.use_texture_ambient", true);
    material->SetTexture("phong_material.texture_ambient0", phong.texture_ambient.value());
  } else {
    material->SetBool("phong_material.use_texture_ambient", false);
  }
  
  if (phong.texture_diffuse) {
    material->SetBool("phong_material.use_texture_diffuse", true);
    material->SetTexture("phong_material.texture_diffuse0", phong.texture_diffuse.value());
  } else {
    material->SetBool("phong_material.use_texture_diffuse", false);
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

  material->SetVec3("pbr_material.albedo", pbr.albedo);
  material->SetFloat("pbr_material.roughness", pbr.roughness);
  material->SetFloat("pbr_material.metallic", pbr.metallic);
  material->SetVec3("pbr_material.ao", pbr.ao);

  if (pbr.texture_normal.has_value()) {
    material->SetBool("pbr_material.use_texture_normal", true);
    material->SetTexture("pbr_material.texture_normal0", pbr.texture_normal.value());
  } else {
    material->SetBool("pbr_material.use_texture_normal", false);
  }

  if (pbr.texture_albedo.has_value()) {
    material->SetBool("pbr_material.use_texture_albedo", true);
    material->SetTexture("pbr_material.texture_albedo0", pbr.texture_albedo.value());
  } else {
    material->SetBool("pbr_material.use_texture_albedo", false);
  }

  if (pbr.texture_metallic.has_value()) {
    material->SetBool("pbr_material.use_texture_metallic", true);
    material->SetTexture("pbr_material.texture_metallic0", pbr.texture_metallic.value());
  } else {
    material->SetBool("pbr_material.use_texture_metallic", false);
  }

  if (pbr.texture_roughness.has_value()) {
    material->SetBool("pbr_material.use_texture_roughness", true);
    material->SetTexture("pbr_material.texture_roughness0", pbr.texture_roughness.value());
  } else {
    material->SetBool("pbr_material.use_texture_roughness", false);
  }

  if (pbr.texture_ao.has_value()) {
    material->SetBool("pbr_material.use_texture_ao", true);
    material->SetTexture("pbr_material.texture_ao0", pbr.texture_ao.value());
  } else {
    material->SetBool("pbr_material.use_texture_ao", false);
  }

  material->SetTexture("texture_irradiance_cubemap", pbr.texture_irradiance_cubemap);
  material->SetTexture("texture_prefiltered_color_cubemap", pbr.texture_prefiltered_color_cubemap);
  material->SetTexture("texture_BRDF_integrate_map", pbr.texture_BRDF_integrate_map);

  pbr.light_info.UpdateMaterial(context, material);
  if (pbr.shadow_info) {
    pbr.shadow_info->UpdateMaterial(context, material);
  }
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

TextureShader::TextureShader(const Param& param, Context* context, Object* object) {
  engine::Material* material = CGCHECK_NOTNULL(object->mutable_material(0));
  material->SetShader(context->GetShader("texture"));

  const engine::Camera& camera = context->camera();
  glm::mat4 project = camera.GetProjectMatrix();
  glm::mat4 view = camera.GetViewMatrix();
  glm::mat4 model = object->GetModelMatrix();
  material->SetMat4("project", project);
  material->SetMat4("view", view);
  material->SetMat4("model", model);
  material->SetTexture("texture0", param.texture0);
}

Texture2DLodShader::Texture2DLodShader(const Param& param, Context* context, Object* object) {
  engine::Material* material = CGCHECK_NOTNULL(object->mutable_material(0));
  material->SetShader(context->GetShader("texture2d_lod"));

  const engine::Camera& camera = context->camera();
  glm::mat4 project = camera.GetProjectMatrix();
  glm::mat4 view = camera.GetViewMatrix();
  glm::mat4 model = object->GetModelMatrix();
  material->SetMat4("project", project);
  material->SetMat4("view", view);
  material->SetMat4("model", model);
  CGCHECK(param.texture2D0.type() == engine::Texture::Texture2D);
  material->SetTexture("texture2D0", param.texture2D0);
  material->SetVec3("view_pos_ws", param.view_pos_ws);
}

CubemapLodShader::CubemapLodShader(const Param& param, Context* context, Object* object) {
  engine::Material* material = CGCHECK_NOTNULL(object->mutable_material(0));
  material->SetShader(context->GetShader("cubemap_lod"));

  const engine::Camera& camera = context->camera();
  glm::mat4 project = camera.GetProjectMatrix();
  glm::mat4 view = camera.GetViewMatrix();
  glm::mat4 model = object->GetModelMatrix();
  material->SetMat4("project", project);
  material->SetMat4("view", view);
  material->SetMat4("model", model);
  CGCHECK(param.cubemap.type() == engine::Texture::Cubemap);
  material->SetTexture("texture_cubemap", param.cubemap);
  material->SetVec3("view_pos_ws", param.view_pos_ws);
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
  material->SetTexture("texture0", param.cubemap);
}

FullscreenQuadShader::FullscreenQuadShader(const Param& param, Context* context, EmptyObject* empty_object) {
  engine::Material* material = CGCHECK_NOTNULL(empty_object->mutable_material(0));
  material->SetShader(context->GetShader("fullscreen_quad"));
  material->SetTexture("texture0", param.texture0); 
}

PbrEnvironmentCubemapGerneratorShader::PbrEnvironmentCubemapGerneratorShader(const Param& param, Context* context, Object* object) {
  engine::Material* material = CGCHECK_NOTNULL(object->mutable_material(0));
  material->SetShader(context->GetShader("pbr_environment_cubemap_generator"));

  glm::mat4 model = object->GetModelMatrix();
  glm::mat4 view = param.camera->GetViewMatrix();
  glm::mat4 project = param.camera->GetProjectMatrix();
  material->SetMat4("model", model); 
  material->SetMat4("view", view);
  material->SetMat4("project", project);
  material->SetTexture("texture0", param.texture0); 
}

TexcoordShader::TexcoordShader(const Param& param, Context* context, Object* object) {
  engine::Material* material = CGCHECK_NOTNULL(object->mutable_material(0));
  material->SetShader(context->GetShader("texcoord"));

  glm::mat4 model = object->GetModelMatrix();
  glm::mat4 view = context->camera().GetViewMatrix();
  glm::mat4 project = context->camera().GetProjectMatrix();
  material->SetMat4("model", model); 
  material->SetMat4("view", view);
  material->SetMat4("project", project);
}

PbrIrradianceCubemapGeneratorShader::PbrIrradianceCubemapGeneratorShader(const Param& param, Context* context, Object* object) {
  engine::Material* material = CGCHECK_NOTNULL(object->mutable_material(0));
  material->SetShader(context->GetShader("pbr_irradiance_cubemap_generator"));

  glm::mat4 model = object->GetModelMatrix();
  glm::mat4 view = param.camera->GetViewMatrix();
  glm::mat4 project = param.camera->GetProjectMatrix();
  material->SetMat4("model", model); 
  material->SetMat4("view", view);
  material->SetMat4("project", project); 
  material->SetTexture("cubemap", param.environment_map);
}

SampleShader::SampleShader(const Param& param, Context* context, Object* object) {
  engine::Material* material = CGCHECK_NOTNULL(object->mutable_material(0));
  material->SetShader(context->GetShader("sample"));

  glm::mat4 model = object->GetModelMatrix();
  glm::mat4 view = context->camera().GetViewMatrix();
  glm::mat4 project = context->camera().GetProjectMatrix();
  material->SetMat4("model", model); 
  material->SetMat4("view", view);
  material->SetMat4("project", project); 
}

PbrPrefilteredColorCubemapGeneratorShader::PbrPrefilteredColorCubemapGeneratorShader(
    const Param& param, Context* context, Object* object) {
  engine::Material* material = CGCHECK_NOTNULL(object->mutable_material(0));
  material->SetShader(context->GetShader("pbr_prefiltered_color_cubemap_generator"));

  glm::mat4 model = object->GetModelMatrix();
  glm::mat4 view = param.camera->GetViewMatrix();
  glm::mat4 project = param.camera->GetProjectMatrix();
  material->SetMat4("model", model); 
  material->SetMat4("view", view);
  material->SetMat4("project", project); 
  material->SetTexture("environment_map", param.environment_map); 
  material->SetFloat("roughness", param.roughness); 
}

PbrBRDFIntegrationMapGeneratorShader::PbrBRDFIntegrationMapGeneratorShader(const Param& param, Context* context,
                                                                           EmptyObject* empty_object) {
  engine::Material* material = empty_object->mutable_material();
  material->SetShader(context->GetShader("pbr_BRDF_integration_map_generator"));
  material->PrepareShader();
}
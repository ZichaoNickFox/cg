#include "playground/shaders.h"

#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"
#include <set>

#include "engine/constants.h"
#include "engine/debug.h"
#include "playground/object/point_light.h"
#include "engine/util.h"

void UpdateMaterial(const engine::SceneLightInfo& scene_light_info, engine::Material* material) {
  for (int i = 0; i < scene_light_info.infos.size(); ++i) {
    const engine::SceneLightInfo::LightInfo& light_info = scene_light_info.infos[i];
    material->SetVec3(util::Format("lights[{}].color", i).c_str(), light_info.color);
    material->SetVec3(util::Format("lights[{}].pos", i).c_str(), light_info.pos);
    uint32_t attenuation_metre = light_info.attenuation_metre;
    const engine::LightAttenuation& light_attenuation = engine::kLightAttenuation.at(attenuation_metre);
    material->SetFloat(util::Format("lights[{}].constant", i).c_str(), light_attenuation.constant);
    material->SetFloat(util::Format("lights[{}].linear", i).c_str(), light_attenuation.linear);
    material->SetFloat(util::Format("lights[{}].quadratic", i).c_str(), light_attenuation.quadratic);
  }
  material->SetInt("light_count", scene_light_info.infos.size());
}

void UpdateMaterial(const engine::SceneShadowInfo& scene_shadow_info, engine::Material* material) {
  material->SetBool("use_shadowing", true);
  material->SetMat4("shadow_info.light_space_vp", scene_shadow_info.infos.light_space_vp);
  material->SetTexture("shadow_info.texture_depth", scene_shadow_info.infos.texture_depth);
}

engine::SceneLightInfo::LightInfo AsLightInfo(const PointLight& point_light) {
  engine::SceneLightInfo::LightInfo info;
  info.type = engine::SceneLightInfo::LightInfo::kPointLight;
  info.pos = point_light.transform().translation();
  info.color = point_light.color();
  info.attenuation_metre = point_light.attenuation_metre();
  return info;
}

engine::SceneLightInfo AsSceneLightInfo(const std::vector<PointLight>& point_lights) {
  engine::SceneLightInfo scene_light_info;
  for (const PointLight& point_light : point_lights) {
    scene_light_info.Insert(AsLightInfo(point_light));
  }
  return scene_light_info;
}

void PhongShader::Param::Gui() {
  ImGui::Separator();
  ImGui::Text("Phong Shader");
  ImGui::SliderFloat3(util::Format("{}##{}", "ambient", (const char*)this).c_str(), glm::value_ptr(ambient_), 0, 1);
  ImGui::SliderFloat3(util::Format("{}##{}", "diffuse", (const char*)this).c_str(), glm::value_ptr(diffuse_), 0, 1);
  ImGui::SliderFloat3(util::Format("{}##{}", "specular", (const char*)this).c_str(), glm::value_ptr(specular_), 0, 1);
  ImGui::SliderFloat(util::Format("{}##{}", "roughness", (const char*)this).c_str(), &shininess_, 0, 100);
  ImGui::Checkbox(util::Format("{}##{}", "use_blinn_phong", (const char*)this).c_str(), &use_blinn_phong_);
  ImGui::Separator();
}

PhongShader::PhongShader(Param* phong, Context* context, Object* object) {
  phong->Gui();
  engine::Material* material = CGCHECK_NOTNULL(object->mutable_material(0));
  material->SetShader(context->GetShader("phong"));

  const engine::Camera& camera = context->camera();
  glm::mat4 project = camera.GetProjectMatrix();
  glm::mat4 view = camera.GetViewMatrix();
  material->SetMat4("project", project);
  material->SetMat4("view", view);
  material->SetMat4("model", object->GetModelMatrix());
  material->SetVec3("view_pos", context->camera().transform().translation());

  material->SetVec3("phong_material.ambient", phong->ambient_);
  material->SetVec3("phong_material.diffuse", phong->diffuse_);
  material->SetVec3("phong_material.specular", phong->specular_);
  material->SetFloat("phong_material.shininess", phong->shininess_);
  if (phong->texture_normal) {
    material->SetBool("phong_material.use_texture_normal", true);
    material->SetTexture("phong_material.texture_normal0", phong->texture_normal.value());
  } else {
    material->SetBool("phong_material.use_texture_normal", false);
  }

  if (phong->texture_specular) {
    material->SetBool("phong_material.use_texture_specular", true);
    material->SetTexture("phong_material.texture_specular0", phong->texture_specular.value());
  } else {
    material->SetBool("phong_material.use_texture_specular", false);
  }

  if (phong->texture_ambient) {
    material->SetBool("phong_material.use_texture_ambient", true);
    material->SetTexture("phong_material.texture_ambient0", phong->texture_ambient.value());
  } else {
    material->SetBool("phong_material.use_texture_ambient", false);
  }
  
  if (phong->texture_diffuse) {
    material->SetBool("phong_material.use_texture_diffuse", true);
    material->SetTexture("phong_material.texture_diffuse0", phong->texture_diffuse.value());
  } else {
    material->SetBool("phong_material.use_texture_diffuse", false);
  }

  UpdateMaterial(phong->scene_light_info, material);
  if (phong->scene_shadow_info) {
    UpdateMaterial(phong->scene_shadow_info.value(), material);
  }

  material->SetInt("blinn_phong", phong->use_blinn_phong_);
}

void PbrShader::Param::Gui() {
  ImGui::Separator();
  ImGui::Text("Pbr Shader");
  ImGui::ColorEdit3(util::Format("{}##{}", "albedo", (const char*)this).c_str(), glm::value_ptr(albedo_));
  ImGui::SliderFloat(util::Format("{}##{}", "metallic", (const char*)this).c_str(), &metallic_, 0, 1);
  ImGui::SliderFloat(util::Format("{}##{}", "roughness", (const char*)this).c_str(), &roughness_, 0, 1);
  ImGui::SliderFloat3(util::Format("{}##{}", "ao", (const char*)this).c_str(), glm::value_ptr(ao_), 0, 1);
  ImGui::Separator();
}

PbrShader::PbrShader(Param* pbr, Context* context, Object* object) {
  pbr->Gui();
  engine::Material* material = CGCHECK_NOTNULL(object->mutable_material(0));
  material->SetShader(context->GetShader("pbr"));

  const engine::Camera& camera = context->camera();
  glm::mat4 project = camera.GetProjectMatrix();
  glm::mat4 view = camera.GetViewMatrix();
  material->SetMat4("project", project);
  material->SetMat4("view", view);
  material->SetMat4("model", object->GetModelMatrix());
  material->SetVec3("view_pos", context->camera().transform().translation());

  material->SetVec3("pbr_material.albedo", pbr->albedo_);
  material->SetFloat("pbr_material.roughness", pbr->roughness_);
  material->SetFloat("pbr_material.metallic", pbr->metallic_);
  material->SetVec3("pbr_material.ao", pbr->ao_);

  if (pbr->texture_normal.has_value()) {
    material->SetBool("pbr_material.use_texture_normal", true);
    material->SetTexture("pbr_material.texture_normal0", pbr->texture_normal.value());
  } else {
    material->SetBool("pbr_material.use_texture_normal", false);
  }

  if (pbr->texture_albedo.has_value()) {
    material->SetBool("pbr_material.use_texture_albedo", true);
    material->SetTexture("pbr_material.texture_albedo0", pbr->texture_albedo.value());
  } else {
    material->SetBool("pbr_material.use_texture_albedo", false);
  }

  if (pbr->texture_metallic.has_value()) {
    material->SetBool("pbr_material.use_texture_metallic", true);
    material->SetTexture("pbr_material.texture_metallic0", pbr->texture_metallic.value());
  } else {
    material->SetBool("pbr_material.use_texture_metallic", false);
  }

  if (pbr->texture_roughness.has_value()) {
    material->SetBool("pbr_material.use_texture_roughness", true);
    material->SetTexture("pbr_material.texture_roughness0", pbr->texture_roughness.value());
  } else {
    material->SetBool("pbr_material.use_texture_roughness", false);
  }

  if (pbr->texture_ao.has_value()) {
    material->SetBool("pbr_material.use_texture_ao", true);
    material->SetTexture("pbr_material.texture_ao0", pbr->texture_ao.value());
  } else {
    material->SetBool("pbr_material.use_texture_ao", false);
  }

  material->SetTexture("texture_irradiance_cubemap", pbr->texture_irradiance_cubemap);
  material->SetTexture("texture_prefiltered_color_cubemap", pbr->texture_prefiltered_color_cubemap);
  material->SetTexture("texture_BRDF_integration_map", pbr->texture_BRDF_integration_map);

  UpdateMaterial(pbr->scene_light_info, material);
  if (pbr->scene_shadow_info) {
    UpdateMaterial(pbr->scene_shadow_info.value(), material);
  }
}

NormalShader::NormalShader(Param* normal, Context* context, Object* object) {
  normal->Gui();
  engine::Material* material = CGCHECK_NOTNULL(object->mutable_material(0));
  material->SetShader(context->GetShader("normal"));
  const engine::Camera& camera = context->camera();
  glm::mat4 project = camera.GetProjectMatrix();
  glm::mat4 view = camera.GetViewMatrix();
  material->SetMat4("project", project);
  material->SetMat4("view", view);
  material->SetMat4("model", object->GetModelMatrix());
  material->SetFloat("line_length", normal->length_);
  material->SetFloat("line_width", normal->width_);
  material->SetVec3("view_pos", camera.transform().translation());
  material->SetBool("show_vertex_normal", normal->show_vertex_normal_);
  material->SetBool("show_TBN", normal->show_TBN_);
  material->SetBool("show_triangle", normal->show_triangle_);
  material->SetBool("show_texture_normal", normal->show_texture_normal_ && normal->texture_normal);
  if (normal->texture_normal) {
    material->SetTexture("texture_normal", normal->texture_normal.value());
  }
}

void NormalShader::Param::Gui() {
  ImGui::Separator();
  ImGui::Text("Normal Shader");
  ImGui::Checkbox(util::Format("{}##{}", "vertex_normal", (const char*)this).c_str(), &show_vertex_normal_);
  ImGui::Checkbox(util::Format("{}##{}", "TBN", (const char*)this).c_str(), &show_TBN_);
  ImGui::Checkbox(util::Format("{}##{}", "triangle", (const char*)this).c_str(), &show_triangle_);
  ImGui::Checkbox(util::Format("{}##{}", "texture_normal", (const char*)this).c_str(), &show_texture_normal_);
  ImGui::SliderFloat(util::Format("{}##{}", "length", (const char*)this).c_str(), &length_, 0, 1);
  ImGui::SliderFloat(util::Format("{}##{}", "width", (const char*)this).c_str(), &width_, 0, 1);
}

LinesShader::LinesShader(const Param& param, Context* context, Object* object) {
  engine::Material* material = CGCHECK_NOTNULL(object->mutable_material(0));
  material->SetShader(context->GetShader("lines"));
  const engine::Camera& camera = context->camera();
  glm::mat4 project = camera.GetProjectMatrix();
  glm::mat4 view = camera.GetViewMatrix();
  glm::mat4 model = object->transform().GetModelMatrix();
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

DepthBufferShader::DepthBufferShader(const DepthBufferShader::Param& param, Object* object) {
  engine::Material* material = CGCHECK_NOTNULL(object->mutable_material(0));
  material->SetShader(param.depth_buffer_shader);

  glm::mat4 project = param.camera->GetProjectMatrix();
  glm::mat4 view = param.camera->GetViewMatrix();
  glm::mat4 model = object->GetModelMatrix();
  material->SetMat4("project", project);
  material->SetMat4("view", view);
  material->SetMat4("model", model);
  material->SetFloat("u_near", param.camera->near_clip());
  material->SetFloat("u_far", param.camera->far_clip());
}

CubemapShader::CubemapShader(const Param& param, Context* context, Object* object) {
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

PbrEnvironmentCubemapGerneratorShader::PbrEnvironmentCubemapGerneratorShader(const Param& param, Context* context,
                                                                             Object* object) {
  engine::Material* material = CGCHECK_NOTNULL(object->mutable_material(0));
  material->SetShader(context->GetShader("equirectangular_2_cubemap_tool"));

  glm::mat4 model = object->GetModelMatrix();
  glm::mat4 view = param.camera->GetViewMatrix();
  glm::mat4 project = param.camera->GetProjectMatrix();
  material->SetMat4("model", model); 
  material->SetMat4("view", view);
  material->SetMat4("project", project);
  material->SetTexture("texture2D0", param.texture2D0); 
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
}

SSAOShader::SSAOShader(const ParamGBuffer& param_g_buffer, Context* context, Object* object) {
  engine::Material* material = object->mutable_material();
  material->SetShader(context->GetShader("SSAO_g_buffer"));

  const engine::Camera& camera = context->camera();
  glm::mat4 project = camera.GetProjectMatrix();
  glm::mat4 view = camera.GetViewMatrix();
  material->SetMat4("project", project);
  material->SetMat4("view", view);
  material->SetMat4("model", object->GetModelMatrix());
}

SSAOShader::SSAOShader(const ParamSSAO& param, Context* context, Object* object) {
  engine::Material* material = object->mutable_material();
  material->SetShader(context->GetShader("SSAO_SSAO"));

  const engine::Camera& camera = context->camera();
  material->SetMat4("u_projection", camera.GetProjectMatrix());
  material->SetTexture("ut_position_vs", param.texture_position_vs);
  material->SetTexture("ut_normal_vs", param.texture_normal_vs);
  material->SetTexture("ut_noise", param.texture_noise);
  for (int i = 0; i < 64; ++i) {
    material->SetVec3(util::Format("u_samples_ts[{}]", i).c_str(), param.sampler_ts[i]);
  }
}

BlurShader::BlurShader(const Param& param, Context* context, Object* object) {
  engine::Material* material = object->mutable_material();
  material->SetShader(context->GetShader("blur"));

  material->SetTexture("u_texture_input", param.texture);
  material->SetVec2("u_viewport_size", param.viewport_size);
}

ComputeShader::ComputeShader(const Param& param, Context* context) : param_(param) {}

void ComputeShader::CheckInternalFormat(engine::Texture texture) {
  GLuint internal_format = texture.internal_format();
  std::set<GLint> supported_format = { GL_RGBA, GL_RGBA32F };
  if (supported_format.count(internal_format) <= 0) {
    CGCHECK(false) << "Unsupported Internal Format : " << std::hex << internal_format << std::dec;
  }
}

void ComputeShader::Run(Context* context) {
  CheckInternalFormat(param_.input_texture);
  CheckInternalFormat(param_.output_texture);

  glActiveTexture_(GL_TEXTURE0);
  glBindTexture_(GL_TEXTURE_2D, param_.input_texture.id());

  glActiveTexture_(GL_TEXTURE1);
  glBindTexture_(GL_TEXTURE_2D, param_.output_texture.id());

  glBindImageTexture_(0, param_.input_texture.id(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
  glBindImageTexture_(1, param_.output_texture.id(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

  context->GetShader(param_.shader_name).Use();
  glDispatchCompute_(param_.work_group_x, param_.work_group_y, param_.work_group_z);
  glMemoryBarrier_(GL_ALL_BARRIER_BITS);
}

SimpleModelShader::SimpleModelShader(Context* context, Model* model) {
  for (int i = 0; i < model->model_part_num(); ++i) {
    ModelPart* model_part = model->mutable_model_part(i);
    model_part->mutable_material()->SetShader(context->GetShader("simple_model"));
    model_part->mutable_material()->SetMat4("model", model_part->transform().GetModelMatrix());
    model_part->mutable_material()->SetMat4("view", context->camera().GetViewMatrix());
    model_part->mutable_material()->SetMat4("project", context->camera().GetProjectMatrix());
    for (auto& pair : model_part->model_part_data().uniform_2_texture) {
      const std::string uniform_name = pair.first;
      const std::vector<engine::Texture>& textures = pair.second;
      if (textures.size() > 0) {
        model_part->mutable_material(0)->SetTexture(uniform_name, textures[0]);
        std::string use_uniform_name = util::Format("use_{}", uniform_name);
        model_part->mutable_material(0)->SetBool(use_uniform_name, true);
      }
    }
  }
}

InstanceSceneShader::InstanceSceneShader(Context* context, Model* model) {
  for (int i = 0; i < model->model_part_num(); ++i) {
    ModelPart* model_part = model->mutable_model_part(i);
    model_part->mutable_material()->SetShader(context->GetShader("instance_scene"));
    model_part->mutable_material()->SetMat4("view", context->camera().GetViewMatrix());
    model_part->mutable_material()->SetMat4("project", context->camera().GetProjectMatrix());
    for (auto& pair : model_part->model_part_data().uniform_2_texture) {
      const std::string uniform_name = pair.first;
      const std::vector<engine::Texture>& textures = pair.second;
      if (textures.size() > 0) {
        model_part->mutable_material(0)->SetTexture(uniform_name, textures[0]);
        std::string use_uniform_name = util::Format("use_{}", uniform_name);
        model_part->mutable_material(0)->SetBool(use_uniform_name, true);
      }
    }
  }
}

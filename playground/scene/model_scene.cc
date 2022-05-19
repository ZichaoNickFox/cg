#include "playground/scene/model_scene.h"

#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>
#include <vector>

#include "engine/gl.h"
#include "engine/repo/model_repo.h"
#include "engine/util.h"
#include "playground/scene/common.h"
#include "playground/shaders.h"

void ModelScene::OnEnter(Context *context)
{
  glEnable_(GL_DEPTH_TEST);

  camera_->mutable_transform()->SetTranslation(glm::vec3(0.87, 4.87, 3.87));
  camera_->mutable_transform()->SetRotation(glm::quat(0.94, -0.14, 0.13, 0.014));
  context->SetCamera(camera_.get());

  nanosuit_.Init(context, "nanosuit1", "nanosuit");
  point_lights_.push_back(PointLight());
  point_lights_[0].mutable_transform()->SetTranslation(glm::vec3(0, 3, -5));
}

void ModelScene::OnUpdate(Context *context)
{
  OnUpdateCommon _(context, "ModelScene");

  nanosuit_.ModelInspector();

  point_lights_[0].SetAttenuationMetre(600);
  
  ImGui::Checkbox("texture_ambient", &use_texture_ambient_);
  ImGui::Checkbox("texture_diffuse", &use_texture_diffuse_);
  ImGui::Checkbox("texture_normal", &use_texture_normal_);
  ImGui::Checkbox("texture_height", &use_texture_height_);
  ImGui::Checkbox("texture_specular", &use_texture_specular_);
  ImGui::Checkbox("show_vertex_normal", &show_vertex_normal_);
  ImGui::Checkbox("show_texture_normal", &show_texture_normal_);
  ImGui::Checkbox("show_TBN", &show_TBN_);
  ImGui::Checkbox("show_triangle", &show_triangle_);
  ImGui::SliderFloat("shininess", &shininess_, 0, 1);
  ImGui::Checkbox("use_blinn_phong", &use_blinn_phong_);

  ImGui::SliderFloat("rotate speed", &rotate_speed_, 0.0, 0.1);
  
  ImGui::ColorEdit3("light color", point_lights_[0].mutable_color());

  ImGui::Separator();
  ImGui::Checkbox("Cull", &enable_cull_face_);
  if (ImGui::Button("Back")) {
    call_face_ = GL_BACK;
  }
  if (ImGui::Button("Front")) {
    call_face_ = GL_FRONT;
  }
  if (ImGui::Button("Front And Back")) {
    call_face_ = GL_FRONT_AND_BACK;
  }
  if (ImGui::Button("CW")) {
    cw_ = GL_CW;
  }
  if (ImGui::Button("CCW")) {
    cw_ = GL_CCW;
  }

  glm::quat rotate = glm::angleAxis(rotate_speed_, glm::vec3(0, 1, 0));
  for (int i = 0; i < nanosuit_.model_part_num(); ++i) {
    ModelPart* model_part = nanosuit_.mutable_model_part(i);
    model_part->mutable_transform()->Rotate(rotate);
  }
}

void ModelScene::OnRender(Context *context) {
  if (enable_cull_face_) {
    glEnable_(GL_CULL_FACE);
  } else {
    glDisable_(GL_CULL_FACE);
  }
  glCullFace_(call_face_);
  glFrontFace_(cw_);

  static PhongShader::Param phong;
  phong.light_info = ShaderLightInfo(point_lights_);
  for (int i = 0; i < nanosuit_.model_part_num(); ++i) {
    ModelPart* model_part = nanosuit_.mutable_model_part(i);
    model_part->mutable_transform()->SetScale(glm::vec3(0.3, 0.3, 0.3));

    if (use_texture_ambient_) {
      phong.texture_ambient = model_part->texture_ambient(0);
    }
    if (use_texture_normal_) {
      phong.texture_normal = model_part->texture_normal(0);
    }
    // TODO assimp may has some error
    if (use_texture_height_) {
      phong.texture_normal = model_part->texture_height(0);
    }
    if (use_texture_specular_) {
      phong.texture_specular = model_part->texture_specular(0);
    }
    if (use_texture_diffuse_) {
      phong.texture_diffuse = model_part->texture_diffuse(0);
    }
    PhongShader(&phong, context, model_part);
    model_part->OnRender(context);
    
    static NormalShader::Param normal_param{false, false, false, false, 0.1, 1, phong.texture_normal};
    NormalShader(&normal_param, context, model_part);
    model_part->OnRender(context);
  }

  ColorShader({point_lights_[0].color()}, context, &point_lights_[0]);
  point_lights_[0].OnRender(context);

  LinesShader lines_shader({10.0}, context, &coord_);
  coord_.OnRender(context);
}

void ModelScene::OnExit(Context *context)
{
  context->SetCamera(nullptr);
}
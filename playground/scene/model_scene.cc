#include "playground/scene/model_scene.h"

#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>
#include <vector>

#include "playground/model_repo.h"
#include "playground/scene/common.h"
#include "playground/shader_param.h"
#include "playground/util.h"

void ModelScene::OnEnter(Context *context)
{
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND_COLOR);

  camera_->mutable_transform()->SetTranslation(glm::vec3(0.87, 4.87, 3.87));
  camera_->mutable_transform()->SetRotation(glm::quat(0.94, -0.14, 0.13, 0.014));
  context->PushCamera(camera_);

  nanosuit_.Init(context, "nanosuit1", "nanosuit");
  point_lights_.push_back(PointLight());
  // point_lights_.push_back(PointLight());
  point_lights_[0].mutable_transform()->SetTranslation(glm::vec3(0, 3, -5));
  // point_lights_[1].mutable_transform()->SetTranslation(glm::vec3(-6, 3, 0));
}

void ModelScene::OnUpdate(Context *context)
{
  OnUpdateCommon _(context, "ModelScene");

  nanosuit_.ModelInspector();

  point_lights_[0].SetAttenuationMetre(600);
  // point_lights_[1].SetAttenuationMetre(600);
  
  ImGui::Checkbox("texture ambient", &texture_ambient_);
  ImGui::Checkbox("texture diffuse", &texture_diffuse_);
  ImGui::Checkbox("texture normal", &texture_normal_);
  ImGui::Checkbox("texture height", &texture_height_);
  ImGui::Checkbox("texture specular", &texture_specular_);
  ImGui::Checkbox("show normal", &show_normal_);
  ImGui::Checkbox("show TBN", &show_TBN_);
  ImGui::Checkbox("show triangle", &show_triangle_);
  ImGui::SliderFloat("shininess", &shininess_, 0, 50);

  ImGui::SliderFloat("rotate speed", &rotate_speed_, 0.0, 0.1);
  
  ImGui::ColorEdit3("light color", point_lights_[0].mutable_color());


  glm::quat rotate = glm::angleAxis(rotate_speed_, glm::vec3(0, 1, 0));
  for (int i = 0; i < nanosuit_.model_part_num(); ++i) {
    ModelPart* model_part = nanosuit_.mutable_model_part(i);
    model_part->mutable_transform()->Rotate(rotate);
  }
}

void ModelScene::OnRender(Context *context) {
  PhongShader::Param phong;
  phong.light_info = ShaderLightInfo(point_lights_);
  phong.use_blinn_phong = false;
  for (int i = 0; i < nanosuit_.model_part_num(); ++i) {
    ModelPart* model_part = nanosuit_.mutable_model_part(i);
    model_part->mutable_transform()->SetScale(glm::vec3(0.3, 0.3, 0.3));

    if (texture_ambient_) {
      phong.texture_ambient = model_part->texture_ambient(0);
    }
    if (texture_normal_) {
      phong.texture_normal = model_part->texture_normal(0);
    }
    // TODO assimp may has some error
    if (texture_height_) {
      phong.texture_normal = model_part->texture_height(0);
    }
    if (texture_specular_) {
      phong.texture_specular = model_part->texture_specular(0);
    }
    if (texture_diffuse_) {
      phong.texture_diffuse = model_part->texture_diffuse(0);
    }
    phong.shininess = shininess_;
    PhongShader(phong, context, model_part);
    model_part->OnRender(context);
    
    NormalShader({0.1, show_normal_, show_TBN_, show_triangle_}, context, model_part);
    model_part->OnRender(context);
  }

  point_lights_[0].OnRender(context);
  // point_lights_[1].OnRender(context);
  coord_.OnRender(context);
}

void ModelScene::OnExit(Context *context)
{
  context->PopCamera();
}
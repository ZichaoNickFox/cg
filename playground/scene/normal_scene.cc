#include "playground/scene/normal_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <math.h>
#include <memory>

#include "engine/transform.h"
#include "playground/scene/common.h"

void NormalScene::OnEnter(Context *context)
{
  const glm::vec3 kLightColor = glm::vec3(1.0, 1.0, 1.0);
  const glm::vec3 kLightPos = glm::vec3(2, 2, 0);
  const glm::vec3 kLightScale = glm::vec3(0.4, 0.4, 0.4);
  point_light_.mutable_transform()->SetTranslation(kLightPos);
  point_light_.mutable_transform()->SetScale(kLightScale);
  point_light_.SetColor(kLightColor);

  plane_.mutable_transform()->SetScale(glm::vec3(3, 3, 3));
  // plane_.mutable_transform()->SetRotation(glm::angleAxis(float(M_PI) / 2, glm::vec3(1.0, 0.0, -1.0)));

  sphere_.mutable_transform()->SetTranslation(glm::vec3(3, 1, 1));

  camera_->mutable_transform()->SetTranslation(glm::vec3(-4.8, 6.1, 5.8));
  camera_->mutable_transform()->SetRotation(glm::quat(0.88, -0.30, -0.32, -0.11));
  context->PushCamera(camera_);

  struct Data {
    std::vector<glm::vec3> points;
    std::vector<glm::vec3> colors;
    GLuint primitive_mode;    // line_style : GL_LINES, GL_LINE_STRIP, GL_LINE_LOOP
    int line_width = 1;
  };

  Lines::Mesh line_data{{kLineFrom, kLineTo},
                        {glm::vec3(1, 0, 0), glm::vec3(1, 0, 0)}, GL_LINES};
  line_.SetMesh(line_data);

  glEnable(GL_DEPTH_TEST);
}

void NormalScene::OnUpdate(Context *context)
{
  OnUpdateCommon _(context, "NormalScene");

  ImGui::ColorEdit3("light_color", (float*)&light_color_);
  if (ImGui::Button("gold")) {
    material_property_name_ = "gold";
  }
  ImGui::SameLine();
  if (ImGui::Button("silver")) {
    material_property_name_ = "silver";
  }
  ImGui::SameLine();
  if (ImGui::Button("jade")) {
    material_property_name_ = "jade";
  }
  ImGui::SameLine();
  if (ImGui::Button("rube")) {
    material_property_name_ = "ruby";
  }
  ImGui::Checkbox("blinn_phong", &use_blinn_phong_);

  point_light_.SetColor(light_color_);
  point_light_.OnUpdate(context);

  intersect_line_.reset();
  Object::IntersectResult intersect_result;
  if (plane_.Intersect(context, kLineFrom, kLineTo - kLineFrom, &intersect_result)) {
    intersect_line_ = std::make_unique<Lines>();
    intersect_line_->SetMesh({{intersect_result.pos_ws, intersect_result.pos_ws + intersect_result.normal_ws * 10.0f},
                              {glm::vec3(1, 0, 0), glm::vec3(1, 0, 0)}, GL_LINES});
  }

  coord_.OnUpdate(context);
}

void NormalScene::OnRender(Context *context)
{
  PhongShader::Param phong;
  phong.light_info = ShaderLightInfo(point_light_);
  phong.use_blinn_phong = use_blinn_phong_;
  phong.texture_normal = context->GetTexture("brickwall_normal");
  phong.texture_diffuse = context->GetTexture("brickwall");

  PhongShader(phong, context, &plane_);
  plane_.OnRender(context);
  NormalShader({0.1}, context, &plane_);
  plane_.OnRender(context);

  PhongShader(phong, context, &sphere_);
  sphere_.OnRender(context);
  NormalShader({0.1}, context, &sphere_);
  sphere_.OnRender(context);

  if (intersect_line_) {
    LinesShader lines_shader({1.0}, context, intersect_line_.get());
    intersect_line_->OnRender(context);
  }

  LinesShader lines({1.0}, context, &coord_); 
  coord_.OnRender(context);
}

void NormalScene::OnExit(Context *context)
{
  point_light_.OnDestory(context);
  plane_.OnDestory(context);
  sphere_.OnDestory(context);
  coord_.OnDestory(context);
  context->PopCamera();
}
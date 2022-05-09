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
  const glm::vec4 kLightColor = glm::vec4(1.0, 1.0, 1.0, 1.0);
  const glm::vec3 kLightPos = glm::vec3(5, 5, 0);
  const glm::vec3 kLightScale = glm::vec3(0.4, 0.4, 0.4);
  point_light_.mutable_transform()->SetTranslation(kLightPos);
  point_light_.mutable_transform()->SetScale(kLightScale);
  point_light_.SetColor(kLightColor);

  plane_.mutable_transform()->SetScale(glm::vec3(3, 3, 3));
  // plane_.mutable_transform()->SetRotation(glm::angleAxis(float(M_PI) / 2, glm::vec3(1.0, 0.0, -1.0)));
  plane_.mutable_transform()->SetTranslation(glm::vec3(1, 1, 1));

  sphere_.mutable_transform()->SetTranslation(glm::vec3(3, 1, 0));

  camera_->mutable_transform()->SetTranslation(glm::vec3(5.83, 4.11, 0.10));
  camera_->mutable_transform()->SetRotation(glm::quat(-0.64, 0.28, -0.65, -0.28));
  context->SetCamera(camera_.get());

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
  ImGui::Checkbox("blinn_phong", &use_blinn_phong_);

  ImGui::Checkbox("show_vertex_normal", &show_vertex_normal_);
  ImGui::Checkbox("show_texture_normal", &show_texture_normal_);
  ImGui::Checkbox("show_TBN", &show_TBN_);
  ImGui::Checkbox("show_triangle", &show_triangle_);

  ImGui::SliderFloat("shininess", &shininess_, 0, 1);

  point_light_.SetColor(light_color_);
  point_light_.OnUpdate(context);

  glm::vec2 normalized_cursor_screen_pos(context->io().normalized_cursor_screen_pos_x(),
                                         context->io().normalized_cursor_screen_pos_y());
  glm::vec3 cursor_world_pos_near, cursor_world_pos_far;
  context->camera().GetPickRay(normalized_cursor_screen_pos, &cursor_world_pos_near, &cursor_world_pos_far);
  kLineFrom = cursor_world_pos_near;
  kLineTo = cursor_world_pos_far;

  intersect_line_.reset();
  Object::IntersectResult intersect_result;
  if (sphere_.Intersect(context, kLineFrom, kLineTo - kLineFrom, &intersect_result)) {
    intersect_line_ = std::make_unique<Lines>();
    intersect_line_->SetMesh({{intersect_result.position_ws, intersect_result.position_ws + intersect_result.normal_ws * 10.0f},
                              {glm::vec3(1, 0, 0), glm::vec3(1, 0, 0)}, GL_LINES});
  }

  coord_.OnUpdate(context);
}

void NormalScene::OnRender(Context *context)
{
  static PhongShader::Param phong_param;
  phong_param.light_info = ShaderLightInfo(point_light_);
  phong_param.texture_normal = context->GetTexture("brickwall_normal");
  phong_param.texture_diffuse = context->GetTexture("brickwall");

  PhongShader(&phong_param, context, &plane_);
  plane_.OnRender(context);

  static NormalShader::Param normal_param{false, false, false, false, 0.1, 1.0, context->GetTexture("brickwall_normal")};
  NormalShader(&normal_param, context, &plane_);
  plane_.OnRender(context);

  PhongShader(&phong_param, context, &sphere_);
  sphere_.OnRender(context);
  NormalShader(&normal_param, context, &sphere_);
  sphere_.OnRender(context);

  if (intersect_line_) {
    LinesShader lines_shader({1.0}, context, intersect_line_.get());
    intersect_line_->OnRender(context);
  }

  LinesShader lines({1.0}, context, &coord_); 
  coord_.OnRender(context);

  ColorShader({}, context, &point_light_);
  point_light_.OnRender(context);
}

void NormalScene::OnExit(Context *context)
{
  point_light_.OnDestory(context);
  plane_.OnDestory(context);
  sphere_.OnDestory(context);
  coord_.OnDestory(context);
  context->SetCamera(nullptr);
}
#include "playground/scene/share_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <math.h>
#include <memory>

#include "renderer/transform.h"
#include "renderer/util.h"
#include "playground/object/empty_object.h"
#include "playground/scene/common.h"

void ShareScene::OnEnter(Scene *context)
{
  const glm::vec3 kLightPos = glm::vec3(5, 3, 0);
  const glm::vec3 kLightScale = glm::vec3(0.4, 0.4, 0.4);
  point_light_.mutable_transform()->SetTranslation(kLightPos);
  point_light_.mutable_transform()->SetScale(kLightScale);
  point_light_.SetColor(kLightColor);

  plane_.mutable_transform()->SetScale(glm::vec3(3, 3, 3));
  plane_.mutable_transform()->SetTranslation(glm::vec3(1, 1, 1));

  sphere_.mutable_transform()->SetTranslation(glm::vec3(3, 1, 0));

  camera_->mutable_transform()->SetTranslation(glm::vec3(5.95, 5.34, 2.24));
  camera_->mutable_transform()->SetRotation(glm::quat(-0.81, 0.36, -0.40, -0.17));
  context->SetCamera(camera_.get());

  directional_light_.mutable_transform()->SetTranslation(kLightPos);
  directional_light_.mutable_transform()->SetRotation(glm::quat(-0.67, 0.16, -0.70, -0.15));

  struct Data {
    std::vector<glm::vec3> points;
    std::vector<glm::vec3> colors;
    GLuint primitive_mode;    // line_style : GL_LINES, GL_LINE_STRIP, GL_LINE_LOOP
    int line_width = 1;
  };

  LinesObject::Mesh line_data{{kLineFrom, kLineTo},
                        {glm::vec3(1, 0, 0), glm::vec3(1, 0, 0)}, GL_LINES};
  line_.SetMesh(line_data);

  glEnable_(GL_DEPTH_TEST);

  depth_buffer_pass_.Init({context->framebuffer_size()}, directional_light_.transform());
  forward_pass_.Init({context->framebuffer_size(), 1, context->clear_color()});
}

void ShareScene::OnUpdate(Scene *context)
{
  OnUpdateCommon _(context, "ShareScene");
  if (ImGui::Button("101.1 color")) {
    step_ = 1011;
  }
  if (step_ == 1011) {
    step_ = 1011;
  }
  if (ImGui::Button("101.2 texcoord")) {
    step_ = 1012;
  }
  if (ImGui::Button("101.3 phong")) {
    step_ = 1013;
  }
  if (ImGui::Button("101.4 normal texture")) {
    step_ = 1014;
  }
  if (ImGui::Button("101.5 Shadow")) {
    step_ = 1015;
  }
  if (ImGui::Button("101.6 pbr")) {
    step_ = 1016;
  }
  if (ImGui::Button("101.7")) {
    step_ = 1017;
  }
  if (step_ == 1016) {
    ImGui::SliderFloat("metallic", &metallic_, 0.0, 1.0);
    ImGui::SliderFloat("roughness", &roughness_, 0.0, 1.0);
  }


  ImGui::ColorEdit3("light_color", (float*)&light_color_);
  use_texture_normal_ = step_ > 1013;
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
  ObjectDeprecated::IntersectResult intersect_result;
  if (sphere_.Intersect(context, kLineFrom, kLineTo - kLineFrom, &intersect_result)) {
    intersect_line_ = std::make_unique<LinesObject>();
    intersect_line_->SetMesh({{intersect_result.position_ws, intersect_result.position_ws + intersect_result.normal_ws * 10.0f},
                              {glm::vec3(1, 0, 0), glm::vec3(1, 0, 0)}, GL_LINES});
  }

  coord_.OnUpdate(context);
}

void ShareScene::OnRender(Scene *context)
{
  if (step_ < 1015) {
    if (step_ == 1011) {
      ColorShader({plane_color_1011_}, context, &plane_);
      ColorShader({sphere_color_1011_}, context, &sphere_);
      sphere_.OnRender(context);
      plane_.OnRender(context);
    } else if (step_ == 1012) {
      TextureShader({context->GetTexture("brickwall")}, context, &plane_);
      TextureShader({context->GetTexture("brickwall")}, context, &sphere_);
      sphere_.OnRender(context);
      plane_.OnRender(context);
    } else if (step_ > 1012) {
      PhongShader::Param phong_param;
      phong_param.scene_light_info = SceneLightInfo(point_light_);
      if (use_texture_normal_) {
        phong_param.texture_normal = context->GetTexture("brickwall_normal");
      }
      phong_param.texture_diffuse = context->GetTexture("brickwall");
      PhongShader(&phong_param, context, &sphere_);
      PhongShader(&phong_param, context, &plane_);

      sphere_.OnRender(context);
      plane_.OnRender(context);

      NormalShader::Param normal_param{false, false, false, false, 0.1, 1.0, context->GetTexture("brickwall_normal")};
      NormalShader(&normal_param, context, &sphere_);
      NormalShader(&normal_param, context, &plane_);

      sphere_.OnRender(context);
      plane_.OnRender(context);
    }

    LinesShader lines({1.0}, context, &coord_); 
    coord_.OnRender(context);

    ColorShader({kLightColor}, context, &point_light_);
    point_light_.OnRender(context);
  } else if (step_ == 1015) {
    RunDepthBufferPass(context, &depth_buffer_pass_);

    forward_pass_.Update(depth_buffer_pass_.shader_shadow_info());
    RunForwardPass_Deprecated(context, &forward_pass_);

    EmptyObject quad;
    FullscreenQuadShader({forward_pass_.GetColorTexture()}, context, &quad);
    quad.OnRender(context);
  } else if (step_ == 1016) {
    RunDepthBufferPass2(context, &depth_buffer_pass_);

    forward_pass_.Update(depth_buffer_pass_.shader_shadow_info());
    RunForwardPass2(context, &forward_pass_);

    EmptyObject quad;
    FullscreenQuadShader({forward_pass_.GetColorTexture()}, context, &quad);
    quad.OnRender(context);
  }
}

void ShareScene::RunDepthBufferPass(Scene* context, DepthBufferPass* depth_buffer_pass) {
  depth_buffer_pass->Begin();

  DepthBufferShader{context->GetShader("depth_buffer"), depth_buffer_pass->camera(), &sphere_};
  sphere_.OnRender(context);
  DepthBufferShader{context->GetShader("depth_buffer"), depth_buffer_pass->camera(), &plane_};
  plane_.OnRender(context);

  depth_buffer_pass->End();
}

void ShareScene::RunForwardPass_Deprecated(Scene* context, ForwardPass* forward_pass) {
  forward_pass->Begin();

  PhongShader::Param phong;
  phong.infos = forward_pass->prepass_shadow_info();
  phong.scene_light_info = {point_light_};
  phong.texture_normal = context->GetTexture("brickwall_normal");
  phong.texture_diffuse = context->GetTexture("brickwall");
  PhongShader(&phong, context, &sphere_);
  sphere_.OnRender(context);
  PhongShader(&phong, context, &plane_);
  plane_.OnRender(context);

  LinesShader({}, context, directional_light_.mutable_lines());
  TextureShader({context->GetTexture("directional_light")}, context, directional_light_.mutable_billboard());
  directional_light_.OnRender(context);

  ColorShader({kLightColor}, context, &point_light_);
  point_light_.OnRender(context);

  forward_pass->End();
}

void ShareScene::RunDepthBufferPass2(Scene* context, DepthBufferPass* depth_buffer_pass) {
  depth_buffer_pass->Begin();

  DepthBufferShader{context->GetShader("depth_buffer"), depth_buffer_pass->camera(), &sphere_};
  sphere_.OnRender(context);
  DepthBufferShader{context->GetShader("depth_buffer"), depth_buffer_pass->camera(), &plane_};
  plane_.OnRender(context);

  depth_buffer_pass->End();
}

void ShareScene::RunForwardPass2(Scene* context, ForwardPass* forward_pass) {
  forward_pass->Begin();

  PbrShader::Param pbr{albedo_, metallic_, roughness_};
  pbr.texture_albedo = context->GetTexture("brickwall");
  pbr.texture_normal = context->GetTexture("brickwall_normal");
  pbr.scene_light_info = SceneLightInfo({point_light_});
  pbr.infos = forward_pass->prepass_shadow_info();
  PbrShader(&pbr, context, &sphere_);
  sphere_.OnRender(context);
  PbrShader(&pbr, context, &plane_);
  plane_.OnRender(context);

  LinesShader({}, context, directional_light_.mutable_lines());
  TextureShader({context->GetTexture("directional_light")}, context, directional_light_.mutable_billboard());
  directional_light_.OnRender(context);

  ColorShader({kLightColor}, context, &point_light_);
  point_light_.OnRender(context);

  forward_pass->End();
}

void ShareScene::OnExit(Scene *context)
{
  point_light_.OnDestory(context);
  plane_.OnDestory(context);
  sphere_.OnDestory(context);
  coord_.OnDestory(context);
  context->SetCamera(nullptr);
}
#pragma once

#include <memory>

#include "renderer/camera.h"
#include "renderer/pass.h"
#include "renderer/scene.h"
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "playground/context.h"
#include "playground/object/cube_object.h"
#include "playground/object/directional_light_object.h"
#include "playground/object/lines_object.h"
#include "playground/object/plane_object.h"
#include "playground/object/point_light_object.h"
#include "playground/object/sphere_object.h"

class ShareScene : public Scene {
 public:
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

  void RunDepthBufferPass(Scene* context, DepthBufferPass* depth_buffer_pass);
  void RunForwardPass_Deprecated(Scene* context, ForwardPass* forward_pass);
  
  void RunDepthBufferPass2(Scene* context, DepthBufferPass* depth_buffer_pass);
  void RunForwardPass2(Scene* context, ForwardPass* forward_pass);

 private:
  int step_ = 1016;
  glm::vec4 plane_color_1011_ = glm::vec4(1, 0, 0, 1);
  glm::vec4 sphere_color_1011_ = glm::vec4(0, 1, 0, 1);
  
  glm::vec4 light_color_ = glm::vec4(1, 1, 1, 1);

  bool use_blinn_phong_ = false;
  bool use_texture_normal_ = false;
  bool show_vertex_normal_ = false;
  bool show_texture_normal_ = false;
  bool show_TBN_ = false;
  bool show_triangle_ = false;
  float shininess_ = 0.0;

  PointLightObject point_light_;
  PlaneObject plane_;
  SphereObject sphere_;
  std::shared_ptr<renderer::Camera> camera_ = std::make_shared<renderer::Camera>();
  DirectionalLightObject directional_light_;

  DepthBufferPass depth_buffer_pass_;
  ForwardPass forward_pass_;
  ShadowPass shadow_pass_;

  std::string material_property_name_ = "gold";

  CoordObject coord_;

  glm::vec3 kLineFrom = glm::vec3(0.8, 1, 0.8);
  glm::vec3 kLineTo = glm::vec3(0.1, -1, 0.1);
  LinesObject line_;
  std::unique_ptr<LinesObject> intersect_line_;

  const glm::vec4 kLightColor = glm::vec4(1.0, 1.0, 1.0, 1.0);

  // pbr
  float metallic_ = 1.0;
  float roughness_ = 0.247;
  glm::vec3 albedo_ = glm::vec3(1, 1, 1);
};
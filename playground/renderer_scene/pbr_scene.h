#pragma once

#include "renderer/camera.h"
#include "renderer/scene.h"
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "playground/context.h"
#include "playground/object/cube_object.h"
#include "playground/object/lines_object.h"
#include "playground/object/plane_object.h"
#include "playground/object/point_light_object.h"
#include "playground/object/skybox_object.h"
#include "playground/object/sphere_object.h"

class PbrScene : public Scene {
 public:
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender(Scene* contexnt);
  void OnExit() override;

 private:
  glm::vec4 light_color_ = glm::vec4(1, 1, 1, 1);
  glm::vec3 albedo_ = glm::vec3(1, 0, 0);

  int light_num = 4;
  std::vector<PointLightObject> point_lights_;
  SphereObject sphere_;
  CoordObject coord_;
  SkyboxObject skybox_;
  PlaneObject plane_;
  CubeObject cube_;

  ModelObject teapot_;

  std::shared_ptr<cg::Camera> camera_ = std::make_shared<cg::Camera>();

  float metallic_ = 0.5;
  float roughness_ = 0.5;
  float ao_ = 1.0;
};
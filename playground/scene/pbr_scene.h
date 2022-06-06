#pragma once

#include "engine/camera.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/cube_object.h"
#include "playground/object/lines_object.h"
#include "playground/object/model_object.h"
#include "playground/object/plane_object.h"
#include "playground/object/point_light_object.h"
#include "playground/object/skybox_object.h"
#include "playground/object/sphere_object.h"
#include "playground/scene.h"
#include "playground/shaders.h"

class PbrScene : public Scene {
 public:
  void OnEnter(Context* context);
  void OnUpdate(Context* context);
  void OnRender(Context* contexnt);
  void OnExit(Context* context);

 private:
  glm::vec4 light_color_ = glm::vec4(1, 1, 1, 1);
  glm::vec3 albedo_ = glm::vec3(1, 0, 0);

  int light_num = 4;
  std::vector<PointLightObject> point_lights_;
  SphereObject sphere_;
  Coord coord_;
  SkyboxObject skybox_;
  PlaneObject plane_;
  CubeObject cube_;

  ModelObject teapot_;

  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();

  float metallic_ = 0.5;
  float roughness_ = 0.5;
  float ao_ = 1.0;
};
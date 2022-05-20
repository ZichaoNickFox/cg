#pragma once

#include "engine/camera.h"
#include "engine/framebuffer.h"
#include "engine/framebuffer_attachment.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/cube.h"
#include "playground/object/directional_light.h"
#include "playground/object/lines.h"
#include "playground/object/plane.h"
#include "playground/object/point_light.h"
#include "playground/pass.h"
#include "playground/scene.h"
#include "playground/shaders.h"

class ForwardShadingScene : public Scene {
 public:
  void OnEnter(Context* context);
  void OnUpdate(Context* context);
  void OnRender(Context* contexnt);
  void OnExit(Context* context);

 private:
  void RunDepthBufferPass(Context* context, DepthBufferPass* depth_buffer_pass);
  void RunForwardPass(Context* context, ForwardPass* forward_pass);

  std::string material_name_ = "gold";

  std::vector<Cube> cubes_;
  std::vector<engine::Transform> cube_transforms_;

  int point_lights_num_ = 10;
  std::vector<PointLight> point_lights_;

  Coord coord_;
  Plane plane_;
  DirectionalLight directional_light_;

  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();

  engine::Framebuffer depth_framebuffer_;
  engine::Framebuffer forward_framebuffer_;

  DepthBufferPass depth_buffer_pass_;
  ForwardPass forward_pass_;
};
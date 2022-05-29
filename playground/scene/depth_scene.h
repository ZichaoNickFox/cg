#pragma once

#include "engine/camera.h"
#include "engine/framebuffer.h"
#include "engine/framebuffer_attachment.h"
#include "engine/pass.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/cube.h"
#include "playground/object/directional_light.h"
#include "playground/object/lines.h"
#include "playground/object/model.h"
#include "playground/object/plane.h"
#include "playground/object/point_light.h"
#include "playground/scene.h"
#include "playground/shaders.h"

class DepthScene : public Scene {
 public:
  void OnEnter(Context* context);
  void OnUpdate(Context* context);
  void OnRender(Context* contexnt);
  void OnExit(Context* context);

 private:
  void RunDepthBufferPass(Context* context, engine::DepthBufferPass* depth_buffer_pass);
  void RunForwardPass_Deprecated(Context* context, engine::ForwardPass* forward_pass);

  int point_lights_num_ = 10;
  std::vector<PointLight> point_lights_;

  Coord coord_;
  Plane plane_;

  Model nanosuit_;

  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();

  engine::Framebuffer depth_framebuffer_;
  engine::DepthBufferPass depth_buffer_pass_;
};
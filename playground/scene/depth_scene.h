#pragma once

#include "renderer/camera.h"
#include "renderer/framebuffer.h"
#include "renderer/framebuffer_attachment.h"
#include "renderer/pass.h"
#include "renderer/scene.h"
#include "renderer/shader.h"
#include "renderer/shaders.h"
#include "renderer/texture.h"
#include "playground/context.h"
#include "playground/object/cube_object.h"
#include "playground/object/directional_light_object.h"
#include "playground/object/lines_object.h"
#include "playground/object/plane_object.h"
#include "playground/object/point_light_object.h"

class DepthScene : public Scene {
 public:
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  void RunDepthBufferPass(Scene* context, renderer::DepthBufferPass* depth_buffer_pass);
  void RunForwardPass_Deprecated(Scene* context, renderer::ForwardPass* forward_pass);

  int point_lights_num_ = 10;
  std::vector<PointLightObject> point_lights_;

  CoordObject coord_;
  PlaneObject plane_;

  ModelObject nanosuit_;

  std::shared_ptr<renderer::Camera> camera_ = std::make_shared<renderer::Camera>();

  renderer::Framebuffer depth_framebuffer_;
  renderer::DepthBufferPass depth_buffer_pass_;
};

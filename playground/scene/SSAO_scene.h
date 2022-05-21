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

class SSAOScene : public Scene {
 public:
  void OnEnter(Context* context);
  void OnUpdate(Context* context);
  void OnRender(Context* contexnt);
  void OnExit(Context* context);

 private:
  void RunForwardPass_Deprecated(Context* context, engine::ForwardPass* forward_pass);

  void SetupBufferAndPass(Context* context);

  void RunGBufferPass(Context* context, engine::GBufferPass* g_buffer_pass);;
  void RunSSAOPass(Context* context, engine::SSAOPass* SSAO_pass);
  void RunBlurPass(Context* context, engine::BlurPass* blue_pass);
  void RunLightingPass(Context* context, engine::LightingPass* lighting_pass);

  engine::Framebuffer g_buffer_;
  engine::Framebuffer SSAO_buffer_;
  engine::Framebuffer blur_buffer_;
  engine::Framebuffer lighting_buffer_;

  engine::GBufferPass g_buffer_pass_;
  engine::SSAOPass SSAO_pass_;
  engine::BlurPass blur_pass_;
  engine::LightingPass lighting_pass_;

  engine::Texture texture_SSAO_noice_;

  int point_lights_num_ = 10;
  std::vector<PointLight> point_lights_;

  Coord coord_;
  Plane plane_;
  Model nanosuit_;

  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();
};

#pragma once

#include "renderer/camera.h"
#include "renderer/framebuffer.h"
#include "renderer/framebuffer_attachment.h"
#include "renderer/pass.h"
#include "renderer/scene.h"
#include "renderer/shader.h"
#include "renderer/shaders.h"
#include "renderer/texture.h"

class SSAOScene : public Scene {
 public:
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  void SetupBufferAndPass(Scene* context);

  void RunGBufferPass(Scene* context, renderer::GBufferPass* g_buffer_pass);;
  void RunSSAOPass(Scene* context, renderer::SSAOPass* SSAO_pass);
  void RunBlurPass(Scene* context, renderer::BlurPass* blue_pass);

  renderer::Framebuffer g_buffer_;
  renderer::Framebuffer SSAO_buffer_;
  renderer::Framebuffer blur_buffer_;
  renderer::Framebuffer lighting_buffer_;

  renderer::GBufferPass g_buffer_pass_;
  renderer::SSAOPass SSAO_pass_;
  renderer::BlurPass blur_pass_;
  renderer::LightingPass lighting_pass_;

  renderer::Texture texture_noise_;

  int point_lights_num_ = 10;
  std::vector<PointLightObject> point_lights_;

  CoordObject coord_;
  PlaneObject plane_;
  ModelObject nanosuit_;

  std::array<glm::vec3, 64> samples_ts_;

  std::shared_ptr<renderer::Camera> camera_ = std::make_shared<renderer::Camera>();
};

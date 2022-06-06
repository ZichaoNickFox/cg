#include "playground/scene/AA_test_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "engine/util.h"
#include "playground/scene/common.h"

void AATestScene::OnEnter(Context *context)
{
  engine::ColorFramebuffer::Option option{context->framebuffer_size(), 1, {glm::vec4(1,0,0,1)}};
  color_framebuffer_.Init(option);

  engine::MSFramebuffer::Option ms_option{context->framebuffer_size(), 1, {glm::vec4(1,0,0,1)}, 4};
  ms_framebuffer_.Init(ms_option);

  cube_positions_[0] = glm::vec3(0.0f, 0.0f, 0.0f);

  for (int i = 0; i < kCubeNum; ++i) {
    std::unique_ptr<CubeObject> cube = std::make_unique<CubeObject>();
    cube->mutable_transform()->SetTranslation(cube_positions_[i]);
    cube->mutable_material()->SetShader(context->GetShader("aa_test_scene"));
    cubes_.push_back(std::move(cube));
  }

  glEnable_(GL_DEPTH_TEST);

  camera_->mutable_transform()->SetTranslation(glm::vec3(0, 0, 10));
  context->SetCamera(camera_.get());
}

void AATestScene::OnUpdate(Context *context)
{
  OnUpdateCommon _(context, "AATestScene");
}

void AATestScene::OnRender(Context *context)
{
  ms_framebuffer_.Bind();
  for (const std::unique_ptr<CubeObject>& cube : cubes_) {
    cube->OnRender(context);
  }
  ms_framebuffer_.Unbind();
  // ms_framebuffer_.Blit(&color_framebuffer_);

  engine::Texture texture = ms_framebuffer_.GetColorTexture();

  EmptyObject full_screen_quad;
  full_screen_quad.mutable_material()->SetShader(context->GetShader("fullscreen_quad"));
  full_screen_quad.mutable_material()->SetTexture("texture0", texture);
  full_screen_quad.OnRender(context);
}

void AATestScene::OnExit(Context *context)
{
  context->SetCamera(nullptr);
}
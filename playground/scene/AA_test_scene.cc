#include "playground/scene/AA_test_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "playground/scene/common.h"
#include "playground/util.h"

void AATestScene::OnEnter(Context *context)
{
  engine::ColorFrameBuffer::Option option{
      "test_scene", context->screen_width(), context->screen_height(), 1, {glm::vec4(1,0,0,1)}};
  color_frame_buffer_.Init(option);

  engine::MSFrameBuffer::Option ms_option{
      "ms_test_scene", context->screen_width(), context->screen_height(), 1, {glm::vec4(1,0,0,1)}, 4};
  ms_frame_buffer_.Init(ms_option);

  cube_positions_[0] = glm::vec3(0.0f, 0.0f, 0.0f);

  for (int i = 0; i < kCubeNum; ++i) {
    std::unique_ptr<Cube> cube = std::make_unique<Cube>();
    cube->mutable_transform()->SetTranslation(cube_positions_[i]);
    cube->mutable_material()->PushShader(context->GetShader("aa_test_scene"));
    cubes_.push_back(std::move(cube));
  }

  glEnable(GL_DEPTH_TEST);

  camera_->mutable_transform()->SetTranslation(glm::vec3(0, 0, 10));
  context->PushCamera(camera_);
}

void AATestScene::OnUpdate(Context *context)
{
  OnUpdateCommon _(context, "AATestScene");
}

void AATestScene::OnRender(Context *context)
{
  ms_frame_buffer_.Bind();
  for (const std::unique_ptr<Cube>& cube : cubes_) {
    cube->OnRender(context);
  }
  ms_frame_buffer_.Unbind();
  // ms_frame_buffer_.Blit(&color_frame_buffer_);

  engine::Texture texture = ms_frame_buffer_.GetTexture();

  FullscreenQuad full_screen_quad;
  full_screen_quad.mutable_material()->PushShader(context->GetShader("fullscreen_quad"));
  full_screen_quad.mutable_material()->SetTexture("texture0", texture);
  full_screen_quad.OnRender(context);
}

void AATestScene::OnExit(Context *context)
{
  context->PopCamera();
}
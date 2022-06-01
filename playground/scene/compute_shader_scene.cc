#include "playground/scene/compute_shader_scene.h"

#include "engine/transform.h"
#include "engine/math.h"
#include "playground/object/empty_object.h"
#include "playground/scene/common.h"
#include "playground/shaders.h"

void ComputeShaderScene::OnEnter(Context* context) {
  glm::ivec2 screen_size = context->io().screen_size();

  std::vector<glm::vec4> input_data(screen_size.x * screen_size.y);
  input_texture_ = context->CreateTexture({screen_size.x, screen_size.y, input_data});

  std::vector<glm::vec4> output_data(screen_size.x * screen_size.y, glm::vec4(1.0, 0.0, 0.0, 1.0));
  output_texture_ = context->CreateTexture({screen_size.x, screen_size.y, output_data});

  context->SetCamera(camera_.get());
}

void ComputeShaderScene::OnUpdate(Context* context) {
  OnUpdateCommon _(context, "ComputeShaderScene");
}

void ComputeShaderScene::OnRender(Context* context) {
  glm::ivec2 screen_size = context->io().screen_size();
  RandomShader({"random", screen_size.x / 16 + 1, screen_size.y / 16 + 1, 1, input_texture_,
                 output_texture_}, context).Run(context);

  EmptyObject e;
  FullscreenQuadShader({output_texture_}, context, &e);
  e.OnRender(context);

  OnRenderCommon _(context);
}

void ComputeShaderScene::OnExit(Context* context) {
}

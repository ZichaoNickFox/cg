#include "playground/scene/random_test_scene.h"

#include "renderer/geometry.h"
#include "renderer/math.h"
#include "renderer/transform.h"
#include "playground/object/empty_object.h"
#include "playground/scene/common.h"
#include "playground/shaders.h"

void RandomTestScene::OnEnter(Scene* context) {
  glm::ivec2 screen_size = context->io().screen_size();

  std::vector<glm::vec4> input_data(screen_size.x * screen_size.y);
  input_ = context->CreateTexture({screen_size.x, screen_size.y, input_data});

  std::vector<glm::vec4> output_data(screen_size.x * screen_size.y, glm::vec4(1.0, 0.0, 0.0, 1.0));
  output_ = context->CreateTexture({screen_size.x, screen_size.y, output_data});

  context->SetCamera(camera_.get());
}

void RandomTestScene::OnUpdate() {
  OnUpdateCommon _(scene, "RandomScene");
}

void RandomTestScene::OnRender() {
  glm::ivec2 screen_size = scene->io().screen_size();
  RandomShader({screen_size, input_, output_, scene->frame_stat().frame_num()}, scene);

  EmptyObject e;
  FullscreenQuadShader({output_}, scene, &e);
  e.OnRender(scene);

  OnRenderCommon _(scene);
}

void RandomTestScene::OnExit(Scene* context) {
}

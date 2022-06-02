#include "playground/scene/random_scene.h"

#include "engine/transform.h"
#include "engine/math.h"
#include "playground/object/empty_object.h"
#include "playground/scene/common.h"
#include "playground/shaders.h"

void RandomScene::OnEnter(Context* context) {
  glm::ivec2 screen_size = context->io().screen_size();

  std::vector<glm::vec4> input_data(screen_size.x * screen_size.y);
  input_ = context->CreateTexture({screen_size.x, screen_size.y, input_data});

  std::vector<glm::vec4> output_data(screen_size.x * screen_size.y, glm::vec4(1.0, 0.0, 0.0, 1.0));
  output_ = context->CreateTexture({screen_size.x, screen_size.y, output_data});

  context->SetCamera(camera_.get());
}

void RandomScene::OnUpdate(Context* context) {
  OnUpdateCommon _(context, "RandomScene");
}

void RandomScene::OnRender(Context* context) {
  glm::ivec2 screen_size = context->io().screen_size();
  RandomShader({screen_size, input_, output_}, context).Run();

  EmptyObject e;
  FullscreenQuadShader({output_}, context, &e);
  e.OnRender(context);

  OnRenderCommon _(context);
}

void RandomScene::OnExit(Context* context) {
}

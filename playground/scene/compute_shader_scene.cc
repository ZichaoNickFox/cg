#include "playground/scene/path_tracing_scene.h"

#include "engine/transform.h"
#include "engine/math.h"
#include "playground/scene/common.h"
#include "playground/shaders.h"

void ComputeShaderScene::OnEnter(Context* context) {
}

void ComputeShaderScene::OnUpdate(Context* context) {
  OnUpdateCommon _(context, "ComputeShaderScene");
  // Add Code Here
}

void ComputeShaderScene::OnRender(Context* context) {
  // Add Code Here
  

  OnRenderCommon _(context);
}

void ComputeShaderScene::OnExit(Context* context) {
  ground_.OnDestory(context);
}

#include "playground/scene/sample_test_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "renderer/transform.h"
#include "playground/scene/common.h"

void SampleTestScene::OnEnter(Scene *context)
{
  camera_->mutable_transform()->SetTranslation(glm::vec3(2.97, 3.95, 6.76));
  camera_->mutable_transform()->SetRotation(glm::quat(0.95, -0.21, 0.18, 0.04));
  context->SetCamera(camera_.get());

  struct Samples {
    Samples() {
      for (int i = 0; i < 500; ++i) {
        samples[i] = glm::vec4(0.5, 0.5, 0.5, 1.0);
      }
    }
    glm::vec4 samples[500];
  };
  Samples samples;
  samples_ssbo_.Init(0, sizeof(samples), &samples);

  glEnable_(GL_DEPTH_TEST);
}

void SampleTestScene::OnUpdate(Scene *context) {
  OnUpdateCommon(context, "SampleTestScene");

  sphere_object_.OnUpdate(context);
}

void SampleTestScene::OnRender(Scene *context)
{
  SampleShader({}, context, &sphere_object_);
  sphere_object_.OnRender(context);

  OnRenderCommon _(context);
}

void SampleTestScene::OnExit(Scene *context)
{
  sphere_object_.OnDestory(context);
}
#include "playground/renderer_scene/sample_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "renderer/inspector.h"
#include "renderer/transform.h"

using namespace renderer;

class SampleSceneShader : public renderer::RenderShader {
 public:
  SampleSceneShader(const Scene& scene, const Object& object)
      : RenderShader(scene, "sample_scene") {
    SetModel(object);
    SetCamera(scene.camera());
    Run(scene, object);
  }
};

void SampleScene::OnEnter()
{
  camera_->mutable_transform()->SetTranslation(glm::vec3(2.97, 3.95, 6.76));
  camera_->mutable_transform()->SetRotation(glm::quat(0.95, -0.21, 0.18, 0.04));

  std::vector<glm::vec4> samples(500);
  for (int i = 0; i < 500; ++i) {
    samples[i] = glm::vec4(0.5, 0.5, 0.5, 1.0);
  }
  samples_ssbo_.SetData(util::VectorSizeInByte(samples), samples.data());

  object_repo_.AddOrReplace({object_metas_});
}

void SampleScene::OnUpdate() {
}

void SampleScene::OnRender()
{
  SampleSceneShader(*this, object_repo_.GetObject("sphere"));
}

void SampleScene::OnExit()
{
}
#include "playground/object/point_light.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

#include "engine/pass.h"

void PointLight::OnUpdate(Context *context) {

}

void PointLight::OnRender(Context *context)
{
  if (context->pass() != pass::kPassDeferredShadingLighting) {
    const engine::Camera& camera = context->camera();
    glm::mat4 project = camera.GetProjectMatrix();
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model = transform_.GetModelMatrix();
    material_.SetMat4("project", project);
    material_.SetMat4("view", view);
    material_.SetMat4("model", model);
  }
  material_.PrepareShader();

  std::shared_ptr<engine::Mesh> mesh = context->GetMesh("cube");
  mesh->Submit();
}

void PointLight::OnDestory(Context *context) {
}
#include "playground/object/sphere.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

void Sphere::OnUpdate(Context *context) {
}

void Sphere::OnRender(Context *context) {
  const engine::Camera& camera = context->camera();
  glm::mat4 project = camera.GetProjectMatrix();
  glm::mat4 view = camera.GetViewMatrix();
  glm::mat4 model = transform_.GetModelMatrix();
  material_.SetMat4("project", project);
  material_.SetMat4("view", view);
  material_.SetMat4("model", model);
  material_.PrepareShader();

  std::shared_ptr<engine::Mesh> mesh = context->GetMesh("sphere");
  mesh->Submit();
}

void Sphere::OnDestory(Context *context) {
}
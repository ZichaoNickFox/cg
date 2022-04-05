#include "playground/object/cube.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

void Cube::OnUpdate(Context *context) {
}

void Cube::OnRender(Context *context) {
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

  mesh_ = context->GetMesh("cube");
  mesh_->Submit(); 
}

void Cube::OnDestory(Context *context) {
}
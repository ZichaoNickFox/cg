#include "playground/object/plane.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

void Plane::OnUpdate(Context *context) {
}

void Plane::OnRender(Context *context) {
  material_.PrepareShader();

  std::shared_ptr<engine::Mesh> mesh = context->GetMesh("plane");
  mesh->Submit();
}

void Plane::OnDestory(Context *context) {
}
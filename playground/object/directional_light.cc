#include "playground/object/directional_light.h"

#include <glm/gtx/string_cast.hpp>

#include "playground/util.h"

int DirectionalLight::directional_light_num_ = 0;

void DirectionalLight::Init(Context* context) {
  billboard_.Init(context, Billboard::Data{"directional_light"});
}

void DirectionalLight::OnUpdate(Context* context) {
  billboard_.SetTransform(transform_);
  billboard_.OnUpdate(context);

  glm::vec3 from = transform_.translation();
  glm::vec3 to = transform_.translation() + transform_.rotation() * glm::vec3(0, 0, -10);
  lines_.SetMesh(Lines::Mesh{{from, to}, {glm::vec3(1, 1, 1), glm::vec3(1, 1, 1)}, GL_LINES});
}

void DirectionalLight::OnRender(Context* context) {
  billboard_.mutable_material()->PrepareShader();
  billboard_.OnRender(context);
  lines_.mutable_material()->PrepareShader();
  lines_.OnRender(context);
}

void DirectionalLight::OnDestory(Context* context) {
  billboard_.OnDestory(context);
  lines_.OnDestory(context);
}
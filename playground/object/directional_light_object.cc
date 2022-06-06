#include "playground/object/directional_light_object.h"

#include <glm/gtx/string_cast.hpp>

#include "engine/util.h"

int DirectionalLightObject::directional_light_num_ = 0;

void DirectionalLightObject::Init(Context* context) {
  billboard_.Init(context, BillboardObject::Data{"directional_light"});
}

void DirectionalLightObject::OnUpdate(Context* context) {
  billboard_.SetTransform(transform_);
  billboard_.OnUpdate(context);

  glm::vec3 from = transform_.translation();
  glm::vec3 to = transform_.translation() + transform_.rotation() * glm::vec3(0, 0, -10);
  lines_.SetMesh(LinesObject::Mesh{{from, to}, {glm::vec3(1, 1, 1), glm::vec3(1, 1, 1)}, GL_LINES});
}

void DirectionalLightObject::OnRender(Context* context, int instance_num) {
  billboard_.mutable_material()->PrepareShader();
  billboard_.OnRender(context);
  lines_.mutable_material()->PrepareShader();
  lines_.OnRender(context);
}

void DirectionalLightObject::OnDestory(Context* context) {
  billboard_.OnDestory(context);
  lines_.OnDestory(context);
}
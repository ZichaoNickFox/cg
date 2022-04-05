#include "playground/object/billboard.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

void Billboard::Init(Context *context, const Data& data) {
  plane_.mutable_material()->PushShader(context->GetShader("texture0"));
  plane_.mutable_material()->SetTexture("texture0", context->GetTexture(data.texture));
  plane_.mutable_transform()->SetScale(glm::vec3(0.1, 0.1, 0.1));
}

void Billboard::OnUpdate(Context *context) {
  glm::vec3 plane_translate = transform_.translation();
  plane_.mutable_transform()->SetTranslation(plane_translate);

  glm::vec3 camera_pos = context->camera().transform().translation();
  glm::vec3 pos = transform().translation();
  glm::vec3 lookat = glm::normalize(camera_pos - pos);
  glm::quat plane_rotation = glm::rotation(glm::vec3(0 ,1, 0), lookat);
  plane_.mutable_transform()->SetRotation(plane_rotation);
}

void Billboard::OnRender(Context *context) {
  plane_.OnRender(context);
}

void Billboard::OnDestory(Context *context) {

}
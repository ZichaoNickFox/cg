#include "playground/object/billboard_object.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

void BillboardObject::Init(Context *context, const Data& data) {
  mutable_transform()->SetScale(glm::vec3(0.1, 0.1, 0.1));
}

void BillboardObject::OnUpdate(Context *context) {
  glm::vec3 plane_translate = transform_.translation();
  mutable_transform()->SetTranslation(plane_translate);

  glm::vec3 camera_pos = context->camera().transform().translation();
  glm::vec3 pos = transform().translation();
  glm::vec3 lookat = glm::normalize(camera_pos - pos);
  glm::quat plane_rotation = glm::rotation(glm::vec3(0 ,1, 0), lookat);
  mutable_transform()->SetRotation(plane_rotation);
}
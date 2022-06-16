#include "playground/object/sphere_object.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

void SphereObject::OnUpdate(Context *context) {
}

void SphereObject::OnRender(Context *context, int instance_num) {
  material_.PrepareShader();
  GetMesh(context)->Submit();
}

void SphereObject::OnDestory(Context *context) {
}
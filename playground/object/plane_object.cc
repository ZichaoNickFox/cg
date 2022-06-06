#include "playground/object/plane_object.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

void PlaneObject::OnUpdate(Context *context) {
}

void PlaneObject::OnRender(Context *context, int instance_num) {
  material_.PrepareShader();
  mesh(context)->Submit();
}

void PlaneObject::OnDestory(Context *context) {
}
#include "playground/object/plane.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

void Plane::OnUpdate(Context *context) {
}

void Plane::OnRender(Context *context, int instance_num) {
  material_.PrepareShader();
  mesh(context)->Submit();
}

void Plane::OnDestory(Context *context) {
}
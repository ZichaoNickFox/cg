#include "playground/object/sphere.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

void Sphere::OnUpdate(Context *context) {
}

void Sphere::OnRender(Context *context, int instance_num) {
  material_.PrepareShader();
  mesh(context)->Submit();
}

void Sphere::OnDestory(Context *context) {
}
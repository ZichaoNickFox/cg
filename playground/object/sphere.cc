#include "playground/object/sphere.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

void Sphere::OnUpdate(Context *context) {
}

void Sphere::OnRender(Context *context) {
    CGCHECKGL();
  material_.PrepareShader();
    CGCHECKGL();
  mesh(context)->Submit();
    CGCHECKGL();
}

void Sphere::OnDestory(Context *context) {
}
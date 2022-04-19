#include "playground/object/point_light.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

#include "engine/pass.h"

void PointLight::OnUpdate(Context *context) {

}

void PointLight::OnRender(Context *context)
{
  material_.PrepareShader();
  mesh(context)->Submit();
}

void PointLight::OnDestory(Context *context) {
}
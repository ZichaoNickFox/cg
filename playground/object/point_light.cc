#include "playground/object/point_light.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

void PointLight::OnUpdate(Context *context) {

}

void PointLight::OnRender(Context *context, int instance_num)
{
  material_.PrepareShader();
  mesh(context)->Submit();
}

void PointLight::OnDestory(Context *context) {
}
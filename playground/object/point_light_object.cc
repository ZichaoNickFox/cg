#include "playground/object/point_light_object.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

void PointLightObject::OnUpdate(Context *context) {

}

void PointLightObject::OnRender(Context *context, int instance_num)
{
  material_.PrepareShader();
  mesh(context)->Submit();
}

void PointLightObject::OnDestory(Context *context) {
}
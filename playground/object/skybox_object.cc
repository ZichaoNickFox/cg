#include "playground/object/skybox_object.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

void SkyboxObject::OnUpdate(Context *context) {
}

void SkyboxObject::OnRender(Context *context, int instance_num) {
  material_.PrepareShader();
  mesh(context)->Submit(instance_num); 
}

void SkyboxObject::OnDestory(Context *context) {
}
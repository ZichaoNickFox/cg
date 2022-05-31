#include "playground/object/skybox.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

void Skybox::OnUpdate(Context *context) {
}

void Skybox::OnRender(Context *context, int instance_num) {
  material_.PrepareShader();
  mesh(context)->Submit(instance_num); 
}

void Skybox::OnDestory(Context *context) {
}
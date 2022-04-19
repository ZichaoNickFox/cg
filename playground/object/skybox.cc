#include "playground/object/skybox.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

void Skybox::OnUpdate(Context *context) {
}

void Skybox::OnRender(Context *context) {
  material_.PrepareShader();
  mesh(context)->Submit(); 
}

void Skybox::OnDestory(Context *context) {
}
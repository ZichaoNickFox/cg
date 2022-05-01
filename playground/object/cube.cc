#include "playground/object/cube.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

void Cube::OnUpdate(Context *context) {
}

void Cube::OnRender(Context *context) {
  material_.PrepareShader();
  mesh(context)->Submit(); 
}

void Cube::OnDestory(Context *context) {
}
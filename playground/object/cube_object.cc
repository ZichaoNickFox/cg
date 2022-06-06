#include "playground/object/cube_object.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

void CubeObject::OnUpdate(Context *context) {
}

void CubeObject::OnRender(Context *context, int instance_num) {
  material_.PrepareShader();
  mesh(context)->Submit(); 
}

void CubeObject::OnDestory(Context *context) {
}
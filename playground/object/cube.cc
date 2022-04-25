#include "playground/object/cube.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

void Cube::OnUpdate(Context *context) {
}

void Cube::OnRender(Context *context) {
    CGCHECKGL();
  material_.PrepareShader();
    CGCHECKGL();
  mesh(context)->Submit(); 
    CGCHECKGL();
}

void Cube::OnDestory(Context *context) {
}
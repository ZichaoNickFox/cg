#include "playground/object/empty_object.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

EmptyObject::EmptyObject() {
  glGenVertexArrays(1, &empty_vao_);
}

void EmptyObject::OnUpdate(Context *context) {

}

void EmptyObject::OnRender(Context *context)
{
  material_.PrepareShader();
  glBindVertexArray(empty_vao_);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void EmptyObject::OnDestory(Context *context) {
}
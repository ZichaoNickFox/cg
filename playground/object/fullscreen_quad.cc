#include "playground/object/fullscreen_quad.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

FullscreenQuad::FullscreenQuad() {
  glGenVertexArrays(1, &empty_vao_);
}

void FullscreenQuad::OnUpdate(Context *context) {

}

void FullscreenQuad::OnRender(Context *context)
{
  material_.PrepareShader();
  glBindVertexArray(empty_vao_);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void FullscreenQuad::OnDestory(Context *context) {
}
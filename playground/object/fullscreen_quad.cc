#include "playground/object/fullscreen_quad.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

void FullscreenQuad::OnUpdate(Context *context) {

}

void FullscreenQuad::OnRender(Context *context)
{
  if (empty_vao_ == std::numeric_limits<GLuint>::max()) {
    glGenVertexArrays(1, &empty_vao_);
  }
  if (!material_.HasShader()) {
    material_.PushShader(context->GetShader("fullscreen_quad"));
  }
  material_.PrepareShader();

  glBindVertexArray(empty_vao_);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void FullscreenQuad::OnDestory(Context *context) {
}
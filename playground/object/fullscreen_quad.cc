#include "playground/object/fullscreen_quad.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

void FullscreenQuad::OnUpdate(Context *context) {

}

void FullscreenQuad::OnRender(Context *context)
{
  material_.PrepareShader();

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void FullscreenQuad::OnDestory(Context *context) {
}
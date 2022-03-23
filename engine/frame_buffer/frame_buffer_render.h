#pragma once

#include "GL/glew.h"

#include "engine/shader.h"

namespace engine{
class FrameBufferRender {
 public:
  void Init(int width, int height, std::shared_ptr<Shader> shader);
  void Render(GLuint texture);

 private:
  float screen_vertices_[24] = {
    // position  // texCoord
    -1.0f, 1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
    1.0f, -1.0f, 1.0f, 0.0f,
    -1.0f, 1.0f, 0.0f, 1.0f,
    1.0f, -1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 1.0f
  };
  GLuint screen_vao_;
  GLuint screen_vbo_;

  std::shared_ptr<Shader> shader_;
};
}
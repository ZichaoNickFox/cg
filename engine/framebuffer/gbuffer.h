#pragma once

#include <vector>

#include "engine/framebuffer/depth_framebuffer.h"
#include "engine/framebuffer/framebuffer.h"
#include "engine/framebuffer/framebuffer_attachment.h"
#include "engine/shader.h"

namespace engine {
static const std::vector<FramebufferAttachment> kGBufferMRTLayout = {
  {"position", GL_RGBA, GL_FLOAT, GL_RGBA32F, 4, 16, GL_COLOR_ATTACHMENT0, true,
      GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT},
  {"normal", GL_RGBA, GL_FLOAT, GL_RGBA32F, 4, 16, GL_COLOR_ATTACHMENT1, true,
      GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT},
  {"texcoord", GL_RGBA, GL_FLOAT, GL_RGBA32F, 4, 16, GL_COLOR_ATTACHMENT2, true,
      GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT},
  {"frag_world_pos", GL_RGBA, GL_FLOAT, GL_RGBA32F, 4, 16, GL_COLOR_ATTACHMENT3, true,
      GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT},
  {"depth", GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_COMPONENT32F, 4, 4, GL_DEPTH_ATTACHMENT, false,
      GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE},
};

class GBuffer : public Framebuffer {
 public:
  struct Option {
    glm::ivec2 size;
  };
  void Init(const Option& option);

  void OnBind() override;
  void Clear() override;
  void OnUnbind() override;
  Texture GetTexture(const std::string& layout);

  GLuint fbo() { return fbo_; }
  
  void BlitDepth(DepthFramebuffer* depth_framebuffer);

 private:
  Option option_;
};
}
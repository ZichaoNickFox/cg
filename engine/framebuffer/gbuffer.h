#pragma once

#include <vector>

#include "engine/framebuffer/depth_framebuffer.h"
#include "engine/framebuffer.h"
#include "engine/framebuffer_attachment.h"
#include "engine/shader.h"

namespace engine {
static const std::vector<FramebufferAttachment> kGBufferMRTLayout = {
  {FramebufferAttachment::kColor, "position", GL_RGBA, GL_FLOAT, GL_RGBA32F, 4, 16,
      GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT},
  {FramebufferAttachment::kColor, "normal", GL_RGBA, GL_FLOAT, GL_RGBA32F, 4, 16,
      GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT},
  {FramebufferAttachment::kColor, "texcoord", GL_RGBA, GL_FLOAT, GL_RGBA32F, 4, 16,
      GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT},
  {FramebufferAttachment::kColor, "frag_world_pos", GL_RGBA, GL_FLOAT, GL_RGBA32F, 4, 16,
      GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT},
  {FramebufferAttachment::kDepth, "depth", GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_COMPONENT32F, 4, 4,
      GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE},
};

class GBuffer : public Framebuffer {
 public:
  struct Option {
    glm::ivec2 size;
  };
  void Init(const Option& option);

  void OnBind();
  void Clear();
  void OnUnbind();
  Texture GetTexture(const std::string& layout);

  GLuint fbo() { return fbo_; }
  
  void BlitDepth(DepthFramebuffer* depth_framebuffer);

 private:
  Option option_;
};
}
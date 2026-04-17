#pragma once

#include <cstdint>
#include "glm/glm.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

#include "base/color.h"
#include "renderer/texture.h"
#include "renderer/framebuffer_attachment.h"
#include "rhi/device.h"

namespace cg {
class Framebuffer {
public:
  struct Option {
    glm::ivec2 size;
    std::vector<FramebufferAttachment> attachments;
    glm::vec4 clear_color = kClearColor;
  };
  Framebuffer();
  ~Framebuffer();
  void Init(const Option& option);
  void Bind();
  void Unbind();
  void Clear();

  const glm::ivec2& size() { return option_.size; }
  uint32_t fbo() { return fbo_; }

  Texture GetTexture(const std::string& name);
  void Blit(Framebuffer* framebuffer = nullptr);

 private:
  Texture CreateAttachmentTexture(const FramebufferAttachment& attachment);

  Option option_;
  uint32_t fbo_;

  std::unordered_map<std::string, Texture> textures_;
  std::vector<uint32_t> draw_buffers_;

  rhi::FramebufferState resumption_state_;

  bool inited_ = false;
};
} // namespace cg

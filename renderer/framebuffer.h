#pragma once

#include <cstdint>
#include "glm/glm.hpp"
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include "base/color.h"
#include "renderer/texture.h"
#include "renderer/framebuffer_attachment.h"
#include "rhi/device.h"

namespace cg {
class Framebuffer {
public:
  class ScopedBind {
   public:
    ScopedBind() = default;
    explicit ScopedBind(Framebuffer* framebuffer) : framebuffer_(framebuffer) {
      if (framebuffer_ != nullptr) {
        framebuffer_->Bind();
      }
    }

    ~ScopedBind() {
      Reset();
    }

    ScopedBind(const ScopedBind&) = delete;
    ScopedBind& operator=(const ScopedBind&) = delete;

    ScopedBind(ScopedBind&& other) noexcept : framebuffer_(other.framebuffer_) {
      other.framebuffer_ = nullptr;
    }

    ScopedBind& operator=(ScopedBind&& other) noexcept {
      if (this != &other) {
        Reset();
        framebuffer_ = other.framebuffer_;
        other.framebuffer_ = nullptr;
      }
      return *this;
    }

    void Reset() {
      if (framebuffer_ != nullptr) {
        framebuffer_->Unbind();
        framebuffer_ = nullptr;
      }
    }

    bool active() const { return framebuffer_ != nullptr; }

   private:
    Framebuffer* framebuffer_ = nullptr;
  };

  struct Option {
    glm::ivec2 size;
    std::vector<FramebufferAttachment> attachments;
    glm::vec4 clear_color = kClearColor;
  };
  struct BlitOption {
    uint32_t read_color_attachment_index = 0;
    uint32_t draw_color_attachment_index = 0;
    rhi::ReadBuffer read_buffer = rhi::ReadBuffer::kBack;
    rhi::ReadBuffer draw_buffer = rhi::ReadBuffer::kBack;
    glm::ivec2 read_origin = glm::ivec2(0, 0);
    glm::ivec2 draw_origin = glm::ivec2(0, 0);
    glm::ivec2 size = glm::ivec2(0, 0);
    glm::ivec2 draw_size = glm::ivec2(0, 0);
    rhi::ClearMask mask = rhi::ClearMask::kColor;
    rhi::FilterMode filter = rhi::FilterMode::kNearest;
  };
  Framebuffer();
  ~Framebuffer();
  void Init(const Option& option);
  ScopedBind BindScoped();
  void Clear();

  const glm::ivec2& size() const { return option_.size; }
  uint32_t fbo() const { return fbo_; }

  const Texture& GetTexture(const std::string& name) const;
  void Blit(Framebuffer* framebuffer = nullptr);
  void Blit(Framebuffer* framebuffer, const BlitOption& option);

 private:
  void Bind();
  void Unbind();
  Texture CreateAttachmentTexture(const FramebufferAttachment& attachment);

  Option option_;
  uint32_t fbo_;

  std::unordered_map<std::string, Texture> textures_;
  std::optional<rhi::ScopedRenderPass> render_pass_;
  std::optional<rhi::ScopedFramebufferState> framebuffer_state_scope_;

  bool inited_ = false;
};
} // namespace cg

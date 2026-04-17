#include "renderer/framebuffer.h"

#include "base/debug.h"

namespace cg {
namespace {

rhi::AttachmentType ToRhiAttachmentType(FramebufferAttachment::Type type) {
  switch (type) {
    case FramebufferAttachment::kColor:
      return rhi::AttachmentType::kColor;
    case FramebufferAttachment::kDepth:
      return rhi::AttachmentType::kDepth;
    case FramebufferAttachment::kStencil:
      return rhi::AttachmentType::kStencil;
    case FramebufferAttachment::AttachmentTypeNum:
      break;
  }
  CGCHECK(false) << "Unsupported framebuffer attachment type";
  return rhi::AttachmentType::kColor;
}

}  // namespace

Framebuffer::Framebuffer()
    : fbo_(rhi::GetDevice().CreateFramebuffer()) {}

Framebuffer::~Framebuffer() {
  rhi::GetDevice().DeleteFramebuffer(fbo_);
}

void Framebuffer::Init(const Option& option) {
  option_ = option;
  textures_.clear();
  draw_buffers_.clear();

  rhi::GetDevice().BindFramebuffer(rhi::FramebufferBindPoint::kAll, fbo_);

  std::vector<uint32_t> texture_unit_indices(FramebufferAttachment::AttachmentTypeNum, 0);
  for (const FramebufferAttachment& attachment : option_.attachments) {
    CGCHECK(textures_.count(attachment.name) == 0) << "Attachments has same name : " << attachment.name;

    Texture attachment_texture = CreateAttachmentTexture(attachment);
    textures_[attachment.name] = attachment_texture;

    const uint32_t attachment_index = texture_unit_indices[attachment.type]++;
    rhi::GetDevice().AttachFramebufferTexture2D(ToRhiAttachmentType(attachment.type), attachment_index,
                                                attachment_texture);
    if (attachment.type == FramebufferAttachment::kColor) {
      draw_buffers_.push_back(attachment_index);
    }
  }

  CGCHECK(rhi::GetDevice().CheckFramebufferComplete()) << "Framebuffer Status Error";
  rhi::GetDevice().BindFramebuffer(rhi::FramebufferBindPoint::kAll, 0);

  inited_ = true;
}

void Framebuffer::Bind() {
  CGCHECK(inited_);
  resumption_state_ = rhi::GetDevice().CaptureFramebufferState();
  rhi::GetDevice().SetViewport({0, 0}, option_.size);
  rhi::GetDevice().BindFramebuffer(rhi::FramebufferBindPoint::kAll, fbo_);
  rhi::GetDevice().SetClearColor(option_.clear_color);
  rhi::GetDevice().Clear(rhi::ClearMask::kDepth | rhi::ClearMask::kStencil);

  uint32_t color_attachment_index = 0;
  for (const FramebufferAttachment& attachment : option_.attachments) {
    if (attachment.type == FramebufferAttachment::kColor &&
        attachment.clear_type == FramebufferAttachment::kClear) {
      rhi::GetDevice().ClearColorAttachment(color_attachment_index, option_.clear_color);
      ++color_attachment_index;
    } else if (attachment.type == FramebufferAttachment::kColor) {
      ++color_attachment_index;
    }
  }
  rhi::GetDevice().SetDrawBuffers(draw_buffers_);
}

void Framebuffer::Unbind() {
  rhi::GetDevice().RestoreFramebufferState(resumption_state_);
}

Texture Framebuffer::GetTexture(const std::string& name) {
  CGCHECK(textures_.count(name) == 1) << " Cannot find texture in framebuffer : " << name;
  return textures_[name];
}

void Framebuffer::Blit(Framebuffer* framebuffer) {
  rhi::GetDevice().BlitFramebuffer(fbo_, framebuffer ? framebuffer->fbo() : 0, option_.size,
                                   rhi::ClearMask::kColor, rhi::FilterMode::kNearest);
}

Texture Framebuffer::CreateAttachmentTexture(const FramebufferAttachment& attachment) {
  Texture::Meta texture_meta = attachment.texture_meta;
  texture_meta.width = option_.size.x;
  texture_meta.height = option_.size.y;
  if (attachment.texture_meta.format == rhi::TextureFormat::kRGBA32F) {
    std::vector<glm::vec4> data(texture_meta.data_size_in_byte() / sizeof(glm::vec4), kBlack);
    return CreateTexture2D(texture_meta, {(data.data())});
  } else if (attachment.texture_meta.format == rhi::TextureFormat::kDepth32F) {
    std::vector<float> data(texture_meta.data_size_in_byte() / sizeof(float), 0.0);
    return CreateTexture2D(texture_meta, {(data.data())});
  } else if (attachment.texture_meta.format == rhi::TextureFormat::kRG32F) {
    std::vector<glm::vec2> data(texture_meta.data_size_in_byte() / sizeof(glm::vec2), glm::vec2(0, 0));
    return CreateTexture2D(texture_meta, {(data.data())});
  } else if (attachment.texture_meta.format == rhi::TextureFormat::kR32UI) {
    std::vector<unsigned int> data(texture_meta.data_size_in_byte() / sizeof(unsigned int), 0);
    return CreateTexture2D(texture_meta, {(data.data())});
  } else {
    CGCHECK(false) << " Unsupported texture format"
                   << static_cast<int>(attachment.texture_meta.format);
    return Texture();
  }
}

}  // namespace cg

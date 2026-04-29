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
  textures_.reserve(option_.attachments.size());

  for (const FramebufferAttachment& attachment : option_.attachments) {
    CGCHECK(textures_.count(attachment.name) == 0) << "Attachments has same name : " << attachment.name;

    Texture attachment_texture = CreateAttachmentTexture(attachment);
    textures_[attachment.name] = attachment_texture;
  }

  rhi::FramebufferDesc framebuffer_desc;
  std::vector<uint32_t> attachment_indices(FramebufferAttachment::AttachmentTypeNum, 0);
  for (const FramebufferAttachment& attachment : option_.attachments) {
    framebuffer_desc.attachments.push_back({
        .attachment_type = ToRhiAttachmentType(attachment.type),
        .attachment_index = attachment_indices[attachment.type]++,
        .texture = &textures_.at(attachment.name),
    });
  }

  const bool configured = rhi::GetDevice().ConfigureFramebuffer(fbo_, framebuffer_desc);
  CGCHECK(configured) << "Framebuffer Status Error";

  inited_ = true;
}

Framebuffer::ScopedBind Framebuffer::BindScoped() {
  return ScopedBind(this);
}

void Framebuffer::Bind() {
  CGCHECK(inited_);
  framebuffer_state_scope_.emplace(rhi::GetDevice());
  rhi::RenderPassDesc render_pass_desc;
  render_pass_desc.framebuffer = fbo_;
  render_pass_desc.viewport_origin = {0, 0};
  render_pass_desc.viewport_size = option_.size;
  render_pass_desc.clear_depth = true;
  render_pass_desc.clear_stencil = true;
  render_pass_desc.depth_clear_value = 1.0f;
  render_pass_desc.stencil_clear_value = 0;
  uint32_t color_attachment_index = 0;
  for (const FramebufferAttachment& attachment : option_.attachments) {
    if (attachment.type == FramebufferAttachment::kColor) {
      render_pass_desc.color_attachments.push_back({
          .attachment_index = color_attachment_index,
          .clear = attachment.clear_type == FramebufferAttachment::kClear,
          .clear_color = option_.clear_color,
      });
      ++color_attachment_index;
    }
  }
  render_pass_.emplace(rhi::GetDevice(), render_pass_desc);
}

void Framebuffer::Unbind() {
  render_pass_.reset();
  framebuffer_state_scope_.reset();
}

const Texture& Framebuffer::GetTexture(const std::string& name) const {
  CGCHECK(textures_.count(name) == 1) << " Cannot find texture in framebuffer : " << name;
  return textures_.at(name);
}

void Framebuffer::Blit(Framebuffer* framebuffer) {
  Blit(framebuffer, BlitOption{});
}

void Framebuffer::Blit(Framebuffer* framebuffer, const BlitOption& option) {
  rhi::BlitFramebufferDesc desc;
  desc.read_framebuffer = fbo_;
  desc.draw_framebuffer = framebuffer ? framebuffer->fbo() : 0;
  desc.read_color_attachment_index = option.read_color_attachment_index;
  desc.draw_color_attachment_index = option.draw_color_attachment_index;
  desc.read_buffer = option.read_buffer;
  desc.draw_buffer = option.draw_buffer;
  desc.read_origin = option.read_origin;
  desc.draw_origin = option.draw_origin;
  desc.size = option.size.x > 0 || option.size.y > 0 ? option.size : option_.size;
  desc.draw_size = option.draw_size;
  desc.mask = option.mask;
  desc.filter = option.filter;
  rhi::GetDevice().BlitFramebuffer(desc);
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
  } else if (attachment.texture_meta.format == rhi::TextureFormat::kStencil8) {
    std::vector<uint8_t> data(texture_meta.data_size_in_byte(), 0u);
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

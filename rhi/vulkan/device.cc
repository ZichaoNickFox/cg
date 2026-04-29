#include "rhi/device.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.h>

#include "base/debug.h"
#include "base/util.h"
#include "renderer/texture.h"

namespace cg::rhi {
namespace {

template <class... Ts>
struct Overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

void CheckVkResult(VkResult result, const char* operation_name) {
  CGCHECK(result == VK_SUCCESS) << operation_name << " failed with VkResult=" << static_cast<int>(result);
}

std::string VkVersionString(uint32_t version) {
  return util::Format("{}.{}.{}",
                      VK_API_VERSION_MAJOR(version),
                      VK_API_VERSION_MINOR(version),
                      VK_API_VERSION_PATCH(version));
}

bool HasExtension(const std::vector<VkExtensionProperties>& extensions, const char* extension_name) {
  return std::any_of(extensions.begin(), extensions.end(),
                     [extension_name](const VkExtensionProperties& extension) {
                       return std::strcmp(extension.extensionName, extension_name) == 0;
                     });
}

std::vector<VkExtensionProperties> EnumerateInstanceExtensions() {
  uint32_t extension_count = 0;
  CheckVkResult(vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr),
                "vkEnumerateInstanceExtensionProperties(count)");
  std::vector<VkExtensionProperties> extensions(extension_count);
  CheckVkResult(vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data()),
                "vkEnumerateInstanceExtensionProperties(data)");
  return extensions;
}

std::vector<VkExtensionProperties> EnumerateDeviceExtensions(VkPhysicalDevice physical_device) {
  uint32_t extension_count = 0;
  CheckVkResult(vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr),
                "vkEnumerateDeviceExtensionProperties(count)");
  std::vector<VkExtensionProperties> extensions(extension_count);
  CheckVkResult(vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, extensions.data()),
                "vkEnumerateDeviceExtensionProperties(data)");
  return extensions;
}

uint32_t DetectVulkanApiVersion() {
  uint32_t api_version = VK_API_VERSION_1_1;
#if defined(VK_VERSION_1_1)
  auto* enumerate_instance_version =
      reinterpret_cast<PFN_vkEnumerateInstanceVersion>(vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"));
  if (enumerate_instance_version != nullptr) {
    CheckVkResult(enumerate_instance_version(&api_version), "vkEnumerateInstanceVersion");
  }
#endif
  return api_version;
}

std::vector<const char*> RequiredInstanceExtensions(const std::vector<VkExtensionProperties>& available_extensions,
                                                    bool* enable_portability_enumeration) {
  std::vector<const char*> required_extensions;
  *enable_portability_enumeration = false;

#if defined(CG_PLATFORM_MACOS)
  if (HasExtension(available_extensions, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
    required_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    *enable_portability_enumeration = true;
  }
#endif

  return required_extensions;
}

struct DeviceCandidate {
  VkPhysicalDevice physical_device = VK_NULL_HANDLE;
  uint32_t queue_family_index = std::numeric_limits<uint32_t>::max();
  bool supports_compute = false;
};

DeviceCandidate ChoosePhysicalDevice(VkInstance instance) {
  uint32_t physical_device_count = 0;
  CheckVkResult(vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr),
                "vkEnumeratePhysicalDevices(count)");
  CGCHECK(physical_device_count > 0) << "No Vulkan physical devices were found.";

  std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
  CheckVkResult(vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data()),
                "vkEnumeratePhysicalDevices(data)");

  for (VkPhysicalDevice physical_device : physical_devices) {
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

    for (uint32_t queue_family_index = 0; queue_family_index < queue_family_count; ++queue_family_index) {
      const VkQueueFlags queue_flags = queue_families[queue_family_index].queueFlags;
      if ((queue_flags & VK_QUEUE_GRAPHICS_BIT) == 0) {
        continue;
      }
      DeviceCandidate candidate;
      candidate.physical_device = physical_device;
      candidate.queue_family_index = queue_family_index;
      candidate.supports_compute = (queue_flags & VK_QUEUE_COMPUTE_BIT) != 0;
      return candidate;
    }
  }

  CGCHECK(false) << "Failed to find a Vulkan queue family with graphics support.";
  return {};
}

uint32_t FindMemoryType(VkPhysicalDevice physical_device,
                        uint32_t memory_type_bits,
                        VkMemoryPropertyFlags required_properties) {
  VkPhysicalDeviceMemoryProperties memory_properties;
  vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

  for (uint32_t memory_index = 0; memory_index < memory_properties.memoryTypeCount; ++memory_index) {
    const bool type_matches = (memory_type_bits & (1u << memory_index)) != 0;
    const bool flags_match =
        (memory_properties.memoryTypes[memory_index].propertyFlags & required_properties) == required_properties;
    if (type_matches && flags_match) {
      return memory_index;
    }
  }

  CGCHECK(false) << "Failed to find a suitable Vulkan memory type.";
  return 0;
}

VkBufferUsageFlags ToVkBufferUsage(BufferType type) {
  switch (type) {
    case BufferType::kVertex:
      return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    case BufferType::kIndex:
      return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    case BufferType::kStorage:
      return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    case BufferType::kAtomicCounter:
      return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    case BufferType::kTexture:
      return VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
  }
  CGCHECK(false) << "Unsupported Vulkan buffer type.";
  return 0;
}

size_t PixelFormatChannelCount(PixelFormat format) {
  switch (format) {
    case PixelFormat::kRGBA:
      return 4;
    case PixelFormat::kRG:
      return 2;
    case PixelFormat::kRedInteger:
      return 1;
    case PixelFormat::kDepthComponent:
      return 1;
    case PixelFormat::kStencilIndex:
      return 1;
    case PixelFormat::kUnknown:
      break;
  }
  return 0;
}

size_t PixelTypeSizeInBytes(PixelType type) {
  switch (type) {
    case PixelType::kUInt8:
      return 1;
    case PixelType::kFloat32:
      return 4;
    case PixelType::kUInt32:
      return 4;
    case PixelType::kUnknown:
      break;
  }
  return 0;
}

size_t PixelPayloadSizeInBytes(const glm::ivec2& size, PixelFormat format, PixelType type) {
  return static_cast<size_t>(std::max(size.x, 0)) * static_cast<size_t>(std::max(size.y, 0)) *
         PixelFormatChannelCount(format) * PixelTypeSizeInBytes(type);
}

int TextureFaceCount(const Texture::Meta& meta) {
  return meta.type == Texture::kCubemap ? 6 : 1;
}

size_t TextureSubresourceIndex(const Texture::Meta& meta, int face, int level) {
  CGCHECK(level >= 0 && level < meta.level_num) << level;
  const int face_count = TextureFaceCount(meta);
  CGCHECK(face >= 0 && face < face_count) << face;
  return static_cast<size_t>(level * face_count + face);
}

glm::ivec2 TextureLevelSize(const Texture::Meta& meta, int level) {
  return {
      std::max(1, meta.width >> level),
      std::max(1, meta.height >> level),
  };
}

size_t TextureRowSizeInBytes(const Texture::Meta& meta, int level) {
  return static_cast<size_t>(TextureLevelSize(meta, level).x) * PixelFormatChannelCount(meta.pixel_format) *
         PixelTypeSizeInBytes(meta.pixel_type);
}

uint8_t QuantizeFloatToUNorm8(float value) {
  const float clamped = std::clamp(value, 0.0f, 1.0f);
  return static_cast<uint8_t>(clamped * 255.0f + 0.5f);
}

bool SupportsColorAttachmentConversion(const Texture::Meta& meta) {
  if (meta.pixel_format == PixelFormat::kRGBA &&
      (meta.pixel_type == PixelType::kUInt8 || meta.pixel_type == PixelType::kFloat32)) {
    return true;
  }
  return meta.pixel_format == PixelFormat::kRG && meta.pixel_type == PixelType::kFloat32;
}

Texture::Meta SyntheticColorMeta(PixelFormat format, PixelType type) {
  Texture::Meta meta;
  meta.type = Texture::kTexture2D;
  meta.width = 1;
  meta.height = 1;
  meta.channel_num = static_cast<int>(PixelFormatChannelCount(format));
  meta.hdr = type == PixelType::kFloat32;
  meta.level_num = 1;
  meta.depth = 1;
  meta.pixel_format = format;
  meta.pixel_type = type;
  return meta;
}

glm::ivec2 RemainingExtent(const glm::ivec2& size, const glm::ivec2& origin) {
  return {
      std::max(0, size.x - origin.x),
      std::max(0, size.y - origin.y),
  };
}

struct BlitExtents {
  glm::ivec2 source_origin = glm::ivec2(0, 0);
  glm::ivec2 destination_origin = glm::ivec2(0, 0);
  glm::ivec2 source = glm::ivec2(0, 0);
  glm::ivec2 destination = glm::ivec2(0, 0);
};

struct BlitAxisWindow {
  float start = 0.0f;
  float end = 0.0f;
};

int ResolveVisibleBlitAxisExtent(int nominal_extent, int remaining_extent, float visible_fraction) {
  if (nominal_extent <= 0 || remaining_extent <= 0 || visible_fraction <= 0.0f) {
    return 0;
  }
  const int scaled_extent =
      std::max(1, static_cast<int>(std::lround(static_cast<float>(nominal_extent) * visible_fraction)));
  return std::min(scaled_extent, remaining_extent);
}

BlitAxisWindow ResolveVisibleBlitAxisWindow(int source_origin,
                                            int nominal_source_extent,
                                            int source_size,
                                            int destination_origin,
                                            int nominal_destination_extent,
                                            int destination_size) {
  if (nominal_source_extent <= 0 || nominal_destination_extent <= 0) {
    return {};
  }
  float start = 0.0f;
  float end = 1.0f;
  if (source_origin < 0) {
    start = std::max(start, static_cast<float>(-source_origin) / static_cast<float>(nominal_source_extent));
  }
  if (destination_origin < 0) {
    start = std::max(start, static_cast<float>(-destination_origin) / static_cast<float>(nominal_destination_extent));
  }
  if (source_origin + nominal_source_extent > source_size) {
    end = std::min(end,
                   static_cast<float>(source_size - source_origin) / static_cast<float>(nominal_source_extent));
  }
  if (destination_origin + nominal_destination_extent > destination_size) {
    end = std::min(end,
                   static_cast<float>(destination_size - destination_origin) /
                       static_cast<float>(nominal_destination_extent));
  }
  start = std::clamp(start, 0.0f, 1.0f);
  end = std::clamp(end, 0.0f, 1.0f);
  if (end <= start) {
    return {};
  }
  return {.start = start, .end = end};
}

int ResolveVisibleBlitAxisOrigin(int requested_origin, int nominal_extent, int framebuffer_extent, float start_fraction) {
  if (nominal_extent <= 0 || framebuffer_extent <= 0) {
    return 0;
  }
  return std::clamp(requested_origin + static_cast<int>(std::lround(static_cast<float>(nominal_extent) * start_fraction)),
                    0,
                    framebuffer_extent);
}

BlitExtents ResolveBlitExtents(const glm::ivec2& requested_source_size,
                               const glm::ivec2& requested_destination_size,
                               const glm::ivec2& source_origin,
                               const glm::ivec2& source_size,
                               const glm::ivec2& destination_origin,
                               const glm::ivec2& destination_size) {
  const glm::ivec2 nominal_source_extent = {
      requested_source_size.x > 0 ? requested_source_size.x : std::max(source_size.x - std::max(source_origin.x, 0), 0),
      requested_source_size.y > 0 ? requested_source_size.y : std::max(source_size.y - std::max(source_origin.y, 0), 0),
  };
  if (nominal_source_extent.x <= 0 || nominal_source_extent.y <= 0) {
    return {};
  }

  const glm::ivec2 nominal_destination_extent = {
      requested_destination_size.x > 0 ? requested_destination_size.x : nominal_source_extent.x,
      requested_destination_size.y > 0 ? requested_destination_size.y : nominal_source_extent.y,
  };
  if (nominal_destination_extent.x <= 0 || nominal_destination_extent.y <= 0) {
    return {};
  }

  const BlitAxisWindow x_axis = ResolveVisibleBlitAxisWindow(source_origin.x,
                                                             nominal_source_extent.x,
                                                             source_size.x,
                                                             destination_origin.x,
                                                             nominal_destination_extent.x,
                                                             destination_size.x);
  const BlitAxisWindow y_axis = ResolveVisibleBlitAxisWindow(source_origin.y,
                                                             nominal_source_extent.y,
                                                             source_size.y,
                                                             destination_origin.y,
                                                             nominal_destination_extent.y,
                                                             destination_size.y);
  if (x_axis.end <= x_axis.start || y_axis.end <= y_axis.start) {
    return {};
  }

  BlitExtents extents;
  extents.source_origin = {
      ResolveVisibleBlitAxisOrigin(source_origin.x, nominal_source_extent.x, source_size.x, x_axis.start),
      ResolveVisibleBlitAxisOrigin(source_origin.y, nominal_source_extent.y, source_size.y, y_axis.start),
  };
  extents.destination_origin = {
      ResolveVisibleBlitAxisOrigin(destination_origin.x, nominal_destination_extent.x, destination_size.x, x_axis.start),
      ResolveVisibleBlitAxisOrigin(destination_origin.y, nominal_destination_extent.y, destination_size.y, y_axis.start),
  };

  extents.source = {
      ResolveVisibleBlitAxisExtent(nominal_source_extent.x,
                                   std::max(source_size.x - extents.source_origin.x, 0),
                                   x_axis.end - x_axis.start),
      ResolveVisibleBlitAxisExtent(nominal_source_extent.y,
                                   std::max(source_size.y - extents.source_origin.y, 0),
                                   y_axis.end - y_axis.start),
  };
  extents.destination = {
      ResolveVisibleBlitAxisExtent(nominal_destination_extent.x,
                                   std::max(destination_size.x - extents.destination_origin.x, 0),
                                   x_axis.end - x_axis.start),
      ResolveVisibleBlitAxisExtent(nominal_destination_extent.y,
                                   std::max(destination_size.y - extents.destination_origin.y, 0),
                                   y_axis.end - y_axis.start),
  };
  if (extents.source.x <= 0 || extents.source.y <= 0 || extents.destination.x <= 0 || extents.destination.y <= 0) {
    return {};
  }
  return extents;
}

glm::ivec2 ResolveUninitializedBlitDestinationSize(const glm::ivec2& requested_source_size,
                                                   const glm::ivec2& requested_destination_size,
                                                   const glm::ivec2& source_origin,
                                                   const glm::ivec2& source_size,
                                                   const glm::ivec2& destination_origin) {
  const glm::ivec2 nominal_source_extent = {
      requested_source_size.x > 0 ? requested_source_size.x : std::max(source_size.x - std::max(source_origin.x, 0), 0),
      requested_source_size.y > 0 ? requested_source_size.y : std::max(source_size.y - std::max(source_origin.y, 0), 0),
  };
  if (nominal_source_extent.x <= 0 || nominal_source_extent.y <= 0) {
    return glm::ivec2(0, 0);
  }

  const glm::ivec2 nominal_destination_extent = {
      requested_destination_size.x > 0 ? requested_destination_size.x : nominal_source_extent.x,
      requested_destination_size.y > 0 ? requested_destination_size.y : nominal_source_extent.y,
  };
  if (nominal_destination_extent.x <= 0 || nominal_destination_extent.y <= 0) {
    return glm::ivec2(0, 0);
  }

  const BlitAxisWindow x_axis = ResolveVisibleBlitAxisWindow(source_origin.x,
                                                             nominal_source_extent.x,
                                                             source_size.x,
                                                             destination_origin.x,
                                                             nominal_destination_extent.x,
                                                             std::numeric_limits<int>::max());
  const BlitAxisWindow y_axis = ResolveVisibleBlitAxisWindow(source_origin.y,
                                                             nominal_source_extent.y,
                                                             source_size.y,
                                                             destination_origin.y,
                                                             nominal_destination_extent.y,
                                                             std::numeric_limits<int>::max());
  if (x_axis.end <= x_axis.start || y_axis.end <= y_axis.start) {
    return glm::ivec2(0, 0);
  }

  const int destination_x =
      std::max(destination_origin.x +
                   static_cast<int>(std::lround(static_cast<float>(nominal_destination_extent.x) * x_axis.start)),
               0);
  const int destination_y =
      std::max(destination_origin.y +
                   static_cast<int>(std::lround(static_cast<float>(nominal_destination_extent.y) * y_axis.start)),
               0);
  const int destination_width =
      std::max(1, static_cast<int>(std::lround(static_cast<float>(nominal_destination_extent.x) *
                                               (x_axis.end - x_axis.start))));
  const int destination_height =
      std::max(1, static_cast<int>(std::lround(static_cast<float>(nominal_destination_extent.y) *
                                               (y_axis.end - y_axis.start))));

  return {
      destination_x + destination_width,
      destination_y + destination_height,
  };
}

bool IsScaledBlit(const BlitExtents& extents) {
  return extents.source != extents.destination;
}

int ResolveNearestBlitSampleCoordinate(int destination_coordinate, int source_extent, int destination_extent) {
  if (source_extent <= 0 || destination_extent <= 0) {
    return 0;
  }
  const float source_coordinate =
      ((static_cast<float>(destination_coordinate) + 0.5f) * static_cast<float>(source_extent) /
       static_cast<float>(destination_extent)) -
      0.5f;
  return std::clamp(static_cast<int>(std::floor(source_coordinate + 0.5f)), 0, source_extent - 1);
}

struct LinearBlitAxisSample {
  int lower = 0;
  int upper = 0;
  float upper_weight = 0.0f;
};

LinearBlitAxisSample ResolveLinearBlitAxis(int destination_coordinate, int source_extent, int destination_extent) {
  if (source_extent <= 1 || destination_extent <= 0) {
    return {};
  }
  const float source_coordinate =
      ((static_cast<float>(destination_coordinate) + 0.5f) * static_cast<float>(source_extent) /
       static_cast<float>(destination_extent)) -
      0.5f;
  const float clamped_coordinate = std::clamp(source_coordinate, 0.0f, static_cast<float>(source_extent - 1));
  const int lower = static_cast<int>(std::floor(clamped_coordinate));
  const int upper = std::min(lower + 1, source_extent - 1);
  return {
      .lower = lower,
      .upper = upper,
      .upper_weight = clamped_coordinate - static_cast<float>(lower),
  };
}

std::vector<uint8_t> SnapshotPixelRect(const uint8_t* source_bytes,
                                       size_t source_row_size,
                                       int pixel_size,
                                       const glm::ivec2& source_origin,
                                       const glm::ivec2& source_extent) {
  const size_t row_size = static_cast<size_t>(source_extent.x) * static_cast<size_t>(pixel_size);
  std::vector<uint8_t> snapshot(row_size * static_cast<size_t>(source_extent.y));
  for (int row = 0; row < source_extent.y; ++row) {
    std::memcpy(snapshot.data() + static_cast<size_t>(row) * row_size,
                source_bytes + static_cast<size_t>(source_origin.y + row) * source_row_size +
                    static_cast<size_t>(source_origin.x) * static_cast<size_t>(pixel_size),
                row_size);
  }
  return snapshot;
}

void CopyExactPixelRect(const uint8_t* source_bytes,
                        size_t source_row_size,
                        int pixel_size,
                        const glm::ivec2& source_origin,
                        const glm::ivec2& source_extent,
                        uint8_t* destination_bytes,
                        size_t destination_row_size,
                        const glm::ivec2& destination_origin,
                        const glm::ivec2& destination_extent) {
  if (source_extent == destination_extent) {
    const size_t row_size = static_cast<size_t>(source_extent.x) * static_cast<size_t>(pixel_size);
    for (int row = 0; row < source_extent.y; ++row) {
      std::memcpy(destination_bytes + static_cast<size_t>(destination_origin.y + row) * destination_row_size +
                      static_cast<size_t>(destination_origin.x) * static_cast<size_t>(pixel_size),
                  source_bytes + static_cast<size_t>(source_origin.y + row) * source_row_size +
                      static_cast<size_t>(source_origin.x) * static_cast<size_t>(pixel_size),
                  row_size);
    }
    return;
  }

  for (int row = 0; row < destination_extent.y; ++row) {
    const int source_row_index = ResolveNearestBlitSampleCoordinate(row, source_extent.y, destination_extent.y);
    const uint8_t* source_row =
        source_bytes + static_cast<size_t>(source_origin.y + source_row_index) * source_row_size;
    uint8_t* destination_row =
        destination_bytes + static_cast<size_t>(destination_origin.y + row) * destination_row_size;
    for (int column = 0; column < destination_extent.x; ++column) {
      const int source_column_index = ResolveNearestBlitSampleCoordinate(column, source_extent.x, destination_extent.x);
      std::memcpy(destination_row +
                      static_cast<size_t>(destination_origin.x + column) * static_cast<size_t>(pixel_size),
                  source_row + static_cast<size_t>(source_origin.x + source_column_index) *
                                   static_cast<size_t>(pixel_size),
                  static_cast<size_t>(pixel_size));
    }
  }
}

glm::vec4 ReadColorPixel(const Texture::Meta& meta, const uint8_t* row_bytes, int column) {
  if (meta.pixel_format == PixelFormat::kRGBA && meta.pixel_type == PixelType::kUInt8) {
    const uint8_t* pixel = row_bytes + static_cast<size_t>(column) * 4;
    return {
        static_cast<float>(pixel[0]) / 255.0f,
        static_cast<float>(pixel[1]) / 255.0f,
        static_cast<float>(pixel[2]) / 255.0f,
        static_cast<float>(pixel[3]) / 255.0f,
    };
  }
  if (meta.pixel_format == PixelFormat::kRGBA && meta.pixel_type == PixelType::kFloat32) {
    const float* pixel = reinterpret_cast<const float*>(row_bytes) + static_cast<size_t>(column) * 4;
    return {pixel[0], pixel[1], pixel[2], pixel[3]};
  }
  if (meta.pixel_format == PixelFormat::kRG && meta.pixel_type == PixelType::kFloat32) {
    const float* pixel = reinterpret_cast<const float*>(row_bytes) + static_cast<size_t>(column) * 2;
    return {pixel[0], pixel[1], 0.0f, 1.0f};
  }
  CGCHECK(false) << "Unsupported color source format conversion";
  return glm::vec4(0.0f);
}

void WriteColorPixel(const Texture::Meta& meta, uint8_t* row_bytes, int column, const glm::vec4& color) {
  if (meta.pixel_format == PixelFormat::kRGBA && meta.pixel_type == PixelType::kUInt8) {
    uint8_t* pixel = row_bytes + static_cast<size_t>(column) * 4;
    pixel[0] = QuantizeFloatToUNorm8(color.r);
    pixel[1] = QuantizeFloatToUNorm8(color.g);
    pixel[2] = QuantizeFloatToUNorm8(color.b);
    pixel[3] = QuantizeFloatToUNorm8(color.a);
    return;
  }
  if (meta.pixel_format == PixelFormat::kRGBA && meta.pixel_type == PixelType::kFloat32) {
    float* pixel = reinterpret_cast<float*>(row_bytes) + static_cast<size_t>(column) * 4;
    pixel[0] = color.r;
    pixel[1] = color.g;
    pixel[2] = color.b;
    pixel[3] = color.a;
    return;
  }
  if (meta.pixel_format == PixelFormat::kRG && meta.pixel_type == PixelType::kFloat32) {
    float* pixel = reinterpret_cast<float*>(row_bytes) + static_cast<size_t>(column) * 2;
    pixel[0] = color.r;
    pixel[1] = color.g;
    return;
  }
  CGCHECK(false) << "Unsupported color destination format conversion";
}

glm::vec4 SampleColorPixel(const Texture::Meta& source_meta,
                           const uint8_t* source_bytes,
                           size_t source_row_size,
                           const glm::ivec2& source_origin,
                           const glm::ivec2& source_extent,
                           const glm::ivec2& destination_extent,
                           int destination_x,
                           int destination_y,
                           FilterMode filter) {
  if ((filter == FilterMode::kLinear || filter == FilterMode::kLinearMipmapLinear) && IsScaledBlit({
                                                                                                  .source = source_extent,
                                                                                                  .destination =
                                                                                                      destination_extent,
                                                                                              })) {
    const LinearBlitAxisSample x_axis = ResolveLinearBlitAxis(destination_x, source_extent.x, destination_extent.x);
    const LinearBlitAxisSample y_axis = ResolveLinearBlitAxis(destination_y, source_extent.y, destination_extent.y);
    const uint8_t* source_row0 =
        source_bytes + static_cast<size_t>(source_origin.y + y_axis.lower) * source_row_size;
    const uint8_t* source_row1 =
        source_bytes + static_cast<size_t>(source_origin.y + y_axis.upper) * source_row_size;
    const glm::vec4 c00 = ReadColorPixel(source_meta, source_row0, source_origin.x + x_axis.lower);
    const glm::vec4 c10 = ReadColorPixel(source_meta, source_row0, source_origin.x + x_axis.upper);
    const glm::vec4 c01 = ReadColorPixel(source_meta, source_row1, source_origin.x + x_axis.lower);
    const glm::vec4 c11 = ReadColorPixel(source_meta, source_row1, source_origin.x + x_axis.upper);
    const glm::vec4 top = c00 * (1.0f - x_axis.upper_weight) + c10 * x_axis.upper_weight;
    const glm::vec4 bottom = c01 * (1.0f - x_axis.upper_weight) + c11 * x_axis.upper_weight;
    return top * (1.0f - y_axis.upper_weight) + bottom * y_axis.upper_weight;
  }

  const int source_x = ResolveNearestBlitSampleCoordinate(destination_x, source_extent.x, destination_extent.x);
  const int source_y = ResolveNearestBlitSampleCoordinate(destination_y, source_extent.y, destination_extent.y);
  const uint8_t* source_row = source_bytes + static_cast<size_t>(source_origin.y + source_y) * source_row_size;
  return ReadColorPixel(source_meta, source_row, source_origin.x + source_x);
}

void CopySampledColorRect(const Texture::Meta& source_meta,
                          const uint8_t* source_bytes,
                          size_t source_row_size,
                          const glm::ivec2& source_origin,
                          const glm::ivec2& source_extent,
                          const Texture::Meta& destination_meta,
                          uint8_t* destination_bytes,
                          size_t destination_row_size,
                          const glm::ivec2& destination_origin,
                          const glm::ivec2& destination_extent,
                          FilterMode filter) {
  for (int row = 0; row < destination_extent.y; ++row) {
    uint8_t* destination_row =
        destination_bytes + static_cast<size_t>(destination_origin.y + row) * destination_row_size;
    for (int column = 0; column < destination_extent.x; ++column) {
      WriteColorPixel(destination_meta,
                      destination_row,
                      destination_origin.x + column,
                      SampleColorPixel(source_meta,
                                       source_bytes,
                                       source_row_size,
                                       source_origin,
                                       source_extent,
                                       destination_extent,
                                       column,
                                       row,
                                       filter));
    }
  }
}

uint32_t NextSyntheticProgramId() {
  static uint32_t next_id = 1;
  return next_id++;
}

enum class ProgramPipelineKind {
  kUnknown = 0,
  kRender = 1,
  kCompute = 2,
};

struct VulkanContext {
  ~VulkanContext() {
    if (device != VK_NULL_HANDLE) {
      vkDestroyDevice(device, nullptr);
    }
    if (instance != VK_NULL_HANDLE) {
      vkDestroyInstance(instance, nullptr);
    }
  }

  VkInstance instance = VK_NULL_HANDLE;
  VkPhysicalDevice physical_device = VK_NULL_HANDLE;
  VkDevice device = VK_NULL_HANDLE;
  VkQueue graphics_queue = VK_NULL_HANDLE;
  uint32_t graphics_queue_family_index = std::numeric_limits<uint32_t>::max();
  bool supports_compute = false;
};

std::shared_ptr<VulkanContext> CreateVulkanContext() {
  auto context = std::make_shared<VulkanContext>();

  const uint32_t api_version = DetectVulkanApiVersion();
  const std::vector<VkExtensionProperties> instance_extensions = EnumerateInstanceExtensions();
  bool enable_portability_enumeration = false;
  const std::vector<const char*> required_instance_extensions =
      RequiredInstanceExtensions(instance_extensions, &enable_portability_enumeration);

  VkApplicationInfo application_info{};
  application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  application_info.pApplicationName = "cg";
  application_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
  application_info.pEngineName = "cg";
  application_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
  application_info.apiVersion = api_version;

  VkInstanceCreateInfo instance_create_info{};
  instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_create_info.pApplicationInfo = &application_info;
  instance_create_info.enabledExtensionCount = static_cast<uint32_t>(required_instance_extensions.size());
  instance_create_info.ppEnabledExtensionNames = required_instance_extensions.data();
#if defined(CG_PLATFORM_MACOS)
  if (enable_portability_enumeration) {
    instance_create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
  }
#endif

  CheckVkResult(vkCreateInstance(&instance_create_info, nullptr, &context->instance), "vkCreateInstance");

  const DeviceCandidate candidate = ChoosePhysicalDevice(context->instance);
  context->physical_device = candidate.physical_device;
  context->graphics_queue_family_index = candidate.queue_family_index;
  context->supports_compute = candidate.supports_compute;

  const std::vector<VkExtensionProperties> device_extensions = EnumerateDeviceExtensions(context->physical_device);
  std::vector<const char*> required_device_extensions;
#if defined(CG_PLATFORM_MACOS)
#if defined(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)
  if (HasExtension(device_extensions, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
    required_device_extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
  }
#endif
#endif

  const float queue_priority = 1.0f;
  VkDeviceQueueCreateInfo queue_create_info{};
  queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_create_info.queueFamilyIndex = context->graphics_queue_family_index;
  queue_create_info.queueCount = 1;
  queue_create_info.pQueuePriorities = &queue_priority;

  VkPhysicalDeviceFeatures enabled_features{};

  VkDeviceCreateInfo device_create_info{};
  device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_create_info.queueCreateInfoCount = 1;
  device_create_info.pQueueCreateInfos = &queue_create_info;
  device_create_info.enabledExtensionCount = static_cast<uint32_t>(required_device_extensions.size());
  device_create_info.ppEnabledExtensionNames = required_device_extensions.data();
  device_create_info.pEnabledFeatures = &enabled_features;

  CheckVkResult(vkCreateDevice(context->physical_device, &device_create_info, nullptr, &context->device),
                "vkCreateDevice");
  vkGetDeviceQueue(context->device, context->graphics_queue_family_index, 0, &context->graphics_queue);

  return context;
}

Capabilities BuildCapabilities(const VulkanContext& context, const Capabilities* scene_caps = nullptr) {
  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(context.physical_device, &properties);

  Capabilities capabilities;
  capabilities.backend = BackendType::kVulkan;
  capabilities.graphics_api_name = util::Format("Vulkan {}", VkVersionString(properties.apiVersion));
  capabilities.shader_language_name = "SPIR-V";

  if (scene_caps == nullptr) {
    capabilities.scene_backend = BackendType::kVulkan;
    capabilities.scene_api_name = "Vulkan Synthetic RHI";
    capabilities.shader_language_name = "SPIR-V (synthetic)";
    capabilities.supports_glsl_450 = true;
    capabilities.supports_storage_buffers = true;
    capabilities.supports_storage_images = true;
    capabilities.supports_compute = context.supports_compute;
    return capabilities;
  }

  capabilities.scene_backend =
      scene_caps->scene_backend == BackendType::kUnknown ? scene_caps->backend : scene_caps->scene_backend;
  capabilities.scene_api_name = scene_caps->scene_api_name.empty() ? scene_caps->graphics_api_name
                                                                    : scene_caps->scene_api_name;
  capabilities.shader_language_name = scene_caps->shader_language_name;
  capabilities.supports_glsl_450 = scene_caps->supports_glsl_450;
  capabilities.supports_storage_buffers = scene_caps->supports_storage_buffers;
  capabilities.supports_storage_images = scene_caps->supports_storage_images;
  capabilities.supports_compute = scene_caps->supports_compute;
  return capabilities;
}

class VulkanBuffer final : public Buffer {
 public:
  VulkanBuffer(std::shared_ptr<VulkanContext> context, BufferType type)
      : context_(std::move(context)), type_(type) {}

  ~VulkanBuffer() override {
    DestroyBuffer();
  }

  BufferType type() const override {
    return type_;
  }

  void SetData(size_t size_in_bytes, const void* data, BufferUsage /*usage*/) override {
    CGCHECK(size_in_bytes > 0) << "Vulkan buffer size must be positive.";
    if (size_in_bytes != size_in_bytes_) {
      CreateBuffer(size_in_bytes);
    }
    if (data != nullptr) {
      void* mapped_ptr = MapInternal();
      std::memcpy(mapped_ptr, data, size_in_bytes);
      UnmapInternal();
    }
  }

  void UpdateData(size_t offset_in_bytes, size_t size_in_bytes, const void* data) override {
    CGCHECK(data != nullptr);
    CGCHECK(offset_in_bytes + size_in_bytes <= size_in_bytes_);
    void* mapped_ptr = MapInternal();
    std::memcpy(static_cast<std::byte*>(mapped_ptr) + offset_in_bytes, data, size_in_bytes);
    UnmapInternal();
  }

  void BindBase(uint32_t binding_point) override {
    binding_point_ = binding_point;
  }

  void ReadData(const BufferReadDesc& desc, void* data) override {
    CGCHECK(data != nullptr);
    CGCHECK(desc.offset_in_bytes + desc.size_in_bytes <= size_in_bytes_);
    void* mapped_ptr = MapInternal();
    std::memcpy(data,
                static_cast<const std::byte*>(mapped_ptr) + desc.offset_in_bytes,
                desc.size_in_bytes);
    UnmapInternal();
  }

 private:
  void* MapInternal() {
    CGCHECK(buffer_memory_ != VK_NULL_HANDLE) << "Vulkan buffer was not allocated before host read/write.";
    if (mapped_ptr_ == nullptr) {
      CheckVkResult(vkMapMemory(context_->device, buffer_memory_, 0, size_in_bytes_, 0, &mapped_ptr_),
                    "vkMapMemory");
    }
    return mapped_ptr_;
  }

  void UnmapInternal() {
    if (mapped_ptr_ != nullptr) {
      vkUnmapMemory(context_->device, buffer_memory_);
      mapped_ptr_ = nullptr;
    }
  }

  void CreateBuffer(size_t size_in_bytes) {
    DestroyBuffer();

    VkBufferCreateInfo buffer_create_info{};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size = size_in_bytes;
    buffer_create_info.usage = ToVkBufferUsage(type_);
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    CheckVkResult(vkCreateBuffer(context_->device, &buffer_create_info, nullptr, &buffer_), "vkCreateBuffer");

    VkMemoryRequirements memory_requirements{};
    vkGetBufferMemoryRequirements(context_->device, buffer_, &memory_requirements);

    VkMemoryAllocateInfo memory_allocate_info{};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex =
        FindMemoryType(context_->physical_device,
                       memory_requirements.memoryTypeBits,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    CheckVkResult(vkAllocateMemory(context_->device, &memory_allocate_info, nullptr, &buffer_memory_),
                  "vkAllocateMemory");
    CheckVkResult(vkBindBufferMemory(context_->device, buffer_, buffer_memory_, 0), "vkBindBufferMemory");
    size_in_bytes_ = size_in_bytes;
  }

  void DestroyBuffer() {
    if (mapped_ptr_ != nullptr) {
      vkUnmapMemory(context_->device, buffer_memory_);
      mapped_ptr_ = nullptr;
    }
    if (buffer_ != VK_NULL_HANDLE) {
      vkDestroyBuffer(context_->device, buffer_, nullptr);
      buffer_ = VK_NULL_HANDLE;
    }
    if (buffer_memory_ != VK_NULL_HANDLE) {
      vkFreeMemory(context_->device, buffer_memory_, nullptr);
      buffer_memory_ = VK_NULL_HANDLE;
    }
    size_in_bytes_ = 0;
  }

  std::shared_ptr<VulkanContext> context_;
  BufferType type_;
  uint32_t binding_point_ = 0;
  size_t size_in_bytes_ = 0;
  VkBuffer buffer_ = VK_NULL_HANDLE;
  VkDeviceMemory buffer_memory_ = VK_NULL_HANDLE;
  void* mapped_ptr_ = nullptr;
};

class VulkanVertexArray final : public VertexArray {
 public:
  VulkanVertexArray() = default;

  void ApplyBinding(const VertexArrayBindingDesc& desc) override {
    for (const VertexAttributeDesc& attribute : desc.attributes) {
      attributes_[attribute.index] = {
          .enabled = true,
          .component_count = attribute.component_count,
          .stride_in_bytes = attribute.stride_in_bytes,
          .offset_in_bytes = attribute.offset_in_bytes,
          .divisor = attribute.divisor,
      };
    }
  }

 private:
  void EnableAttribute(uint32_t index) {
    attributes_[index].enabled = true;
  }

  void SetFloatAttribute(uint32_t index,
                         int component_count,
                         int stride_in_bytes,
                         size_t offset_in_bytes) {
    attributes_[index].component_count = component_count;
    attributes_[index].stride_in_bytes = stride_in_bytes;
    attributes_[index].offset_in_bytes = offset_in_bytes;
  }

  void SetAttributeDivisor(uint32_t index, uint32_t divisor) {
    attributes_[index].divisor = divisor;
  }
  struct AttributeState {
    bool enabled = false;
    int component_count = 0;
    int stride_in_bytes = 0;
    size_t offset_in_bytes = 0;
    uint32_t divisor = 0;
  };

  std::unordered_map<uint32_t, AttributeState> attributes_;
};

class VulkanProgram final : public Program {
 public:
  using UseHandler = std::function<void(uint32_t program_id, ProgramPipelineKind pipeline_kind)>;
  using StorageTextureBindingHandler =
      std::function<void(uint32_t texture_unit, const Texture& texture, TextureAccess access)>;
  using BufferBindingHandler = std::function<void(const std::vector<BufferBindingDesc>& bindings)>;

  explicit VulkanProgram(std::string name,
                         ProgramPipelineKind pipeline_kind,
                         UseHandler use_handler = {},
                         StorageTextureBindingHandler storage_texture_binding_handler = {},
                         BufferBindingHandler buffer_binding_handler = {})
      : name_(std::move(name)),
        pipeline_kind_(pipeline_kind),
        use_handler_(std::move(use_handler)),
        storage_texture_binding_handler_(std::move(storage_texture_binding_handler)),
        buffer_binding_handler_(std::move(buffer_binding_handler)),
        synthetic_id_(NextSyntheticProgramId()) {}

  VulkanProgram(std::string name,
                std::shared_ptr<Program> delegate_program,
                ProgramPipelineKind pipeline_kind,
                UseHandler use_handler = {},
                StorageTextureBindingHandler storage_texture_binding_handler = {},
                BufferBindingHandler buffer_binding_handler = {})
      : delegate_program_(std::move(delegate_program)),
        name_(std::move(name)),
        pipeline_kind_(pipeline_kind),
        use_handler_(std::move(use_handler)),
        storage_texture_binding_handler_(std::move(storage_texture_binding_handler)),
        buffer_binding_handler_(std::move(buffer_binding_handler)),
        synthetic_id_(NextSyntheticProgramId()) {}

  void SetBool(const std::string& location_name, bool value) const {
    bool_uniforms_[location_name] = value;
  }

  void SetFloat(const std::string& location_name, float value) const {
    float_uniforms_[location_name] = value;
  }

  void SetInt(const std::string& location_name, int value) const {
    int_uniforms_[location_name] = value;
  }

  int BindTexture(const std::string& location_name, const Texture& value) const {
    auto it = texture_2_unit_.find(value.id());
    if (it != texture_2_unit_.end()) {
      return it->second;
    }
    const int unit = static_cast<int>(texture_2_unit_.size());
    texture_2_unit_[value.id()] = unit;
    return unit;
  }

  void SetMat4(const std::string& location_name, const glm::mat4& value) const {
    mat4_uniforms_[location_name] = value;
  }

  void SetVec4(const std::string& location_name, const glm::vec4& value) const {
    vec4_uniforms_[location_name] = value;
  }

  void SetVec3(const std::string& location_name, const glm::vec3& value) const {
    vec3_uniforms_[location_name] = value;
  }

  void SetVec2(const std::string& location_name, const glm::vec2& value) const {
    vec2_uniforms_[location_name] = value;
  }

  void ApplyBindings(const ProgramBindings& bindings) const override {
    MarkActiveProgram();
    if (delegate_program_ != nullptr) {
      delegate_program_->ApplyBindings(bindings);
      return;
    }
    for (const ProgramUniformBindingDesc& uniform : bindings.uniforms) {
      std::visit(
          Overloaded{
              [&](bool value) { SetBool(uniform.name, value); },
              [&](float value) { SetFloat(uniform.name, value); },
              [&](int value) { SetInt(uniform.name, value); },
              [&](const glm::mat4& value) { SetMat4(uniform.name, value); },
              [&](const glm::vec4& value) { SetVec4(uniform.name, value); },
              [&](const glm::vec3& value) { SetVec3(uniform.name, value); },
              [&](const glm::vec2& value) { SetVec2(uniform.name, value); },
          },
          uniform.value);
    }
    for (const ProgramTextureBindingDesc& texture_binding : bindings.textures) {
      CGCHECK(texture_binding.texture != nullptr) << texture_binding.name;
      BindTexture(texture_binding.name, *texture_binding.texture);
    }
    for (const ProgramStorageTextureBindingDesc& storage_texture_binding : bindings.storage_textures) {
      CGCHECK(storage_texture_binding.texture != nullptr) << storage_texture_binding.name;
      const int texture_unit = BindTexture(storage_texture_binding.name, *storage_texture_binding.texture);
      if (storage_texture_binding_handler_ != nullptr) {
        storage_texture_binding_handler_(texture_unit,
                                         *storage_texture_binding.texture,
                                         storage_texture_binding.access);
      }
    }
    if (buffer_binding_handler_ != nullptr && !bindings.buffers.empty()) {
      buffer_binding_handler_(bindings.buffers);
    }
  }

 private:
  void MarkActiveProgram() const {
    if (delegate_program_ == nullptr) {
      texture_2_unit_.clear();
    }
    if (use_handler_ != nullptr) {
      use_handler_(synthetic_id_, pipeline_kind_);
    }
  }

  std::shared_ptr<Program> delegate_program_;
  std::string name_;
  ProgramPipelineKind pipeline_kind_ = ProgramPipelineKind::kUnknown;
  UseHandler use_handler_;
  StorageTextureBindingHandler storage_texture_binding_handler_;
  BufferBindingHandler buffer_binding_handler_;
  uint32_t synthetic_id_ = 0;
  mutable std::unordered_map<std::string, bool> bool_uniforms_;
  mutable std::unordered_map<std::string, float> float_uniforms_;
  mutable std::unordered_map<std::string, int> int_uniforms_;
  mutable std::unordered_map<std::string, glm::mat4> mat4_uniforms_;
  mutable std::unordered_map<std::string, glm::vec4> vec4_uniforms_;
  mutable std::unordered_map<std::string, glm::vec3> vec3_uniforms_;
  mutable std::unordered_map<std::string, glm::vec2> vec2_uniforms_;
  mutable std::unordered_map<uint32_t, int> texture_2_unit_;
};

class VulkanDevice final : public Device {
 public:
  explicit VulkanDevice(bool enable_opengl_scene_compatibility)
      : context_(CreateVulkanContext()),
        scene_compatibility_device_(enable_opengl_scene_compatibility ? CreateOpenGLDevice() : nullptr),
        capabilities_(BuildCapabilities(*context_,
                                        scene_compatibility_device_ == nullptr
                                            ? nullptr
                                            : &scene_compatibility_device_->capabilities())) {}

  const Capabilities& capabilities() const override {
    return capabilities_;
  }

  std::unique_ptr<Buffer> CreateBuffer(BufferType type) override {
    if (HasSceneCompatibilityDevice()) {
      return scene_compatibility_device_->CreateBuffer(type);
    }
    return std::make_unique<VulkanBuffer>(context_, type);
  }

  std::unique_ptr<VertexArray> CreateVertexArray() override {
    if (HasSceneCompatibilityDevice()) {
      return scene_compatibility_device_->CreateVertexArray();
    }
    return std::make_unique<VulkanVertexArray>();
  }

  std::shared_ptr<Program> CreateProgram(const ProgramDesc& desc) override {
    const ProgramPipelineKind pipeline_kind =
        desc.kind == ProgramKind::kCompute ? ProgramPipelineKind::kCompute : ProgramPipelineKind::kRender;
    if (HasSceneCompatibilityDevice()) {
      return std::make_shared<VulkanProgram>(desc.name,
                                             scene_compatibility_device_->CreateProgram(desc),
                                             pipeline_kind,
                                             [this](uint32_t program_id, ProgramPipelineKind active_pipeline_kind) {
                                               active_program_id_ = program_id;
                                               active_program_kind_ = active_pipeline_kind;
                                             },
                                             VulkanProgram::StorageTextureBindingHandler{},
                                             [this](const std::vector<BufferBindingDesc>& bindings) {
                                               ApplyBufferBindings(bindings);
                                             });
    }
    return std::make_shared<VulkanProgram>(
        desc.name,
        pipeline_kind,
        [this](uint32_t program_id, ProgramPipelineKind active_pipeline_kind) {
          active_program_id_ = program_id;
          active_program_kind_ = active_pipeline_kind;
        },
        [this](uint32_t texture_unit, const Texture& texture, TextureAccess access) {
          RecordStorageTextureBinding(texture_unit, texture, access);
        },
        [this](const std::vector<BufferBindingDesc>& bindings) {
          ApplyBufferBindings(bindings);
        });
  }

  void EnsureTextureUploaded(Texture* texture) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->EnsureTextureUploaded(texture);
      return;
    }
    std::shared_ptr<Texture::Storage> storage = texture->storage();
    CGCHECK(storage != nullptr) << "Texture storage is unavailable.";
    if (storage->uploaded_to_gl) {
      RegisterTexture(*texture);
      return;
    }
    if (storage->id == std::numeric_limits<uint32_t>::max()) {
      storage->id = next_texture_id_++;
    }
    storage->uploaded_to_gl = true;
    RegisterTexture(*texture);
  }

  void ReleaseTexture(Texture* texture) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->ReleaseTexture(texture);
      return;
    }
    std::shared_ptr<Texture::Storage> storage = texture->storage();
    if (storage == nullptr) {
      return;
    }
    texture_records_.erase(storage->id);
    storage->id = std::numeric_limits<uint32_t>::max();
    storage->uploaded_to_gl = false;
  }

  void ReadTextureData(const Texture& texture, const TextureReadDesc& desc, void* data) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->ReadTextureData(texture, desc, data);
      return;
    }
    std::shared_ptr<Texture::Storage> storage = texture.storage();
    const size_t subresource_index = desc.level < 0 ? 0 : TextureSubresourceIndex(texture.meta(), 0, desc.level);
    if (data == nullptr || storage == nullptr || desc.level < 0 || storage->cpu_levels.size() <= subresource_index) {
      if (data != nullptr && desc.size_in_bytes > 0) {
        std::memset(data, 0, desc.size_in_bytes);
      }
      return;
    }
    const std::vector<uint8_t>& source = storage->cpu_levels[subresource_index];
    const size_t bytes_to_copy = std::min(desc.size_in_bytes, source.size());
    if (bytes_to_copy > 0) {
      std::memcpy(data, source.data(), bytes_to_copy);
    }
    if (desc.size_in_bytes > bytes_to_copy) {
      std::memset(static_cast<uint8_t*>(data) + bytes_to_copy, 0, desc.size_in_bytes - bytes_to_copy);
    }
  }

  void DispatchCompute(const ComputeDispatchDesc& desc) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->DispatchCompute(desc);
      return;
    }
    ValidateDispatchState(desc.workgroup_count);
    last_dispatch_desc_ = desc;
  }

  uint32_t CreateFramebuffer() override {
    if (HasSceneCompatibilityDevice()) {
      return scene_compatibility_device_->CreateFramebuffer();
    }
    const uint32_t framebuffer = next_framebuffer_id_++;
    framebuffers_.emplace(framebuffer, FramebufferRecord{});
    return framebuffer;
  }

  void DeleteFramebuffer(uint32_t framebuffer) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->DeleteFramebuffer(framebuffer);
      return;
    }
    framebuffers_.erase(framebuffer);
    if (framebuffer_state_.framebuffer == static_cast<int32_t>(framebuffer)) {
      framebuffer_state_.framebuffer = 0;
    }
    if (framebuffer_state_.read_framebuffer == static_cast<int32_t>(framebuffer)) {
      framebuffer_state_.read_framebuffer = 0;
    }
    if (framebuffer_state_.draw_framebuffer == static_cast<int32_t>(framebuffer)) {
      framebuffer_state_.draw_framebuffer = 0;
    }
    if (read_framebuffer_ == framebuffer) {
      read_framebuffer_ = 0;
    }
    if (draw_framebuffer_ == framebuffer) {
      draw_framebuffer_ = 0;
    }
  }

  void ApplyFramebufferBinding(FramebufferBindPoint bind_point, uint32_t framebuffer) {
    switch (bind_point) {
      case FramebufferBindPoint::kAll:
        framebuffer_state_.read_framebuffer = static_cast<int32_t>(framebuffer);
        framebuffer_state_.draw_framebuffer = static_cast<int32_t>(framebuffer);
        framebuffer_state_.framebuffer = static_cast<int32_t>(framebuffer);
        read_framebuffer_ = framebuffer;
        draw_framebuffer_ = framebuffer;
        break;
      case FramebufferBindPoint::kRead:
        framebuffer_state_.read_framebuffer = static_cast<int32_t>(framebuffer);
        read_framebuffer_ = framebuffer;
        break;
      case FramebufferBindPoint::kDraw:
        framebuffer_state_.draw_framebuffer = static_cast<int32_t>(framebuffer);
        framebuffer_state_.framebuffer = static_cast<int32_t>(framebuffer);
        draw_framebuffer_ = framebuffer;
        break;
    }
  }

  bool ConfigureFramebuffer(uint32_t framebuffer, const FramebufferDesc& desc) override {
    if (HasSceneCompatibilityDevice()) {
      return scene_compatibility_device_->ConfigureFramebuffer(framebuffer, desc);
    }
    CGCHECK(framebuffer != 0) << "Default framebuffer cannot be configured via FramebufferDesc";

    const auto framebuffer_it = framebuffers_.find(framebuffer);
    CGCHECK(framebuffer_it != framebuffers_.end()) << "Unknown framebuffer id " << framebuffer;

    FramebufferRecord& framebuffer_record = framebuffer_it->second;
    framebuffer_record.attachments.clear();
    for (const FramebufferAttachmentDesc& attachment : desc.attachments) {
      CGCHECK(attachment.texture != nullptr) << "Framebuffer attachment texture must not be null";
      RegisterTexture(*attachment.texture);
      framebuffer_record.attachments[AttachmentKey(attachment.attachment_type, attachment.attachment_index)] =
          attachment.texture->id();
    }
    return IsFramebufferComplete(framebuffer_record);
  }

  FramebufferState CaptureFramebufferState() const override {
    if (HasSceneCompatibilityDevice()) {
      return scene_compatibility_device_->CaptureFramebufferState();
    }
    FramebufferState state = framebuffer_state_;
    state.read_framebuffer = static_cast<int32_t>(read_framebuffer_);
    state.draw_framebuffer = static_cast<int32_t>(draw_framebuffer_);
    state.framebuffer = static_cast<int32_t>(draw_framebuffer_);
    return state;
  }

  void RestoreFramebufferState(const FramebufferState& state) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->RestoreFramebufferState(state);
      return;
    }
    const bool legacy_state = state.read_framebuffer == 0 && state.draw_framebuffer == 0;
    const uint32_t restored_read_framebuffer =
        static_cast<uint32_t>(std::max(legacy_state ? state.framebuffer : state.read_framebuffer, 0));
    const uint32_t restored_draw_framebuffer =
        static_cast<uint32_t>(std::max(legacy_state ? state.framebuffer : state.draw_framebuffer, 0));
    if (restored_read_framebuffer == restored_draw_framebuffer) {
      ApplyFramebufferBinding(FramebufferBindPoint::kAll, restored_draw_framebuffer);
    } else {
      ApplyFramebufferBinding(FramebufferBindPoint::kRead, restored_read_framebuffer);
      ApplyFramebufferBinding(FramebufferBindPoint::kDraw, restored_draw_framebuffer);
    }
    framebuffer_state_.viewport = state.viewport;
  }

  void ApplyColorAttachmentClear(uint32_t attachment_index, const glm::vec4& color) {
    const uint32_t draw_framebuffer = CurrentDrawFramebuffer();
    if (draw_framebuffer == 0) {
      CGCHECK(attachment_index == 0)
          << "Default framebuffer only supports color attachment 0 for attachment-selective clear in standalone Vulkan scene mode";
    } else {
      CGCHECK(AttachedTextureId(draw_framebuffer, AttachmentType::kColor, attachment_index) != 0)
          << "Attachment-selective clear requires color attachment "
          << attachment_index
          << " to exist on framebuffer "
          << draw_framebuffer
          << " in standalone Vulkan scene mode";
    }
    cleared_color_attachments_[attachment_index] = color;
    if (TextureRecord* record =
            FindTextureRecord(AttachedTextureId(draw_framebuffer, AttachmentType::kColor, attachment_index));
        record != nullptr) {
      ClearTextureWithColor(record, color);
    }
  }

  void BeginRenderPass(const RenderPassDesc& desc) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->BeginRenderPass(desc);
      return;
    }
    CGCHECK(desc.viewport_size.x > 0 && desc.viewport_size.y > 0)
        << "BeginRenderPass() requires a positive viewport size in standalone Vulkan scene mode";
    if (desc.framebuffer != 0) {
      const auto framebuffer_it = framebuffers_.find(desc.framebuffer);
      CGCHECK(framebuffer_it != framebuffers_.end())
          << "Unknown framebuffer id " << desc.framebuffer << " in standalone Vulkan scene mode";
      CGCHECK(IsFramebufferComplete(framebuffer_it->second))
          << "BeginRenderPass() requires a complete non-default framebuffer in standalone Vulkan scene mode";
    }
    CGCHECK(!render_pass_active_) << "Nested render passes are not supported in standalone Vulkan scene mode";
    render_pass_active_ = true;
    active_render_pass_framebuffer_ = desc.framebuffer;
    ApplyFramebufferBinding(FramebufferBindPoint::kAll, desc.framebuffer);
    ApplyViewport(desc.viewport_origin, desc.viewport_size);
    const bool is_default_framebuffer = desc.framebuffer == 0;

    ClearDesc clear_desc;
    if (desc.clear_depth) {
      if (!is_default_framebuffer) {
        CGCHECK(AttachedTextureId(desc.framebuffer, AttachmentType::kDepth, 0) != 0)
            << "BeginRenderPass() requires depth attachment 0 when clear_depth is enabled on framebuffer "
            << desc.framebuffer
            << " in standalone Vulkan scene mode";
      }
      clear_desc.mask = clear_desc.mask | ClearMask::kDepth;
      clear_desc.depth_clear_value = desc.depth_clear_value;
    }
    if (desc.clear_stencil) {
      if (!is_default_framebuffer) {
        CGCHECK(AttachedTextureId(desc.framebuffer, AttachmentType::kStencil, 0) != 0)
            << "BeginRenderPass() requires stencil attachment 0 when clear_stencil is enabled on framebuffer "
            << desc.framebuffer
            << " in standalone Vulkan scene mode";
      }
      clear_desc.mask = clear_desc.mask | ClearMask::kStencil;
      clear_desc.stencil_clear_value = desc.stencil_clear_value;
    }
    for (const RenderPassColorAttachmentDesc& color_attachment : desc.color_attachments) {
      if (is_default_framebuffer) {
        CGCHECK(color_attachment.attachment_index == 0)
            << "Default framebuffer only supports color attachment 0 in render pass abstraction";
      } else {
        CGCHECK(AttachedTextureId(desc.framebuffer, AttachmentType::kColor, color_attachment.attachment_index) != 0)
            << "BeginRenderPass() requires declared color attachment "
            << color_attachment.attachment_index
            << " to exist on framebuffer "
            << desc.framebuffer
            << " in standalone Vulkan scene mode";
      }
      if (color_attachment.clear) {
        if (is_default_framebuffer) {
          clear_desc.mask = clear_desc.mask | ClearMask::kColor;
          clear_desc.clear_color = color_attachment.clear_color;
        } else {
          ApplyColorAttachmentClear(color_attachment.attachment_index, color_attachment.clear_color);
        }
      }
    }

    if (clear_desc.mask != ClearMask::kNone) {
      Clear(clear_desc);
    }
  }

  void EndRenderPass() override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->EndRenderPass();
      return;
    }
    CGCHECK(render_pass_active_) << "EndRenderPass() requires an active render pass in standalone Vulkan scene mode";
    if (active_render_pass_framebuffer_ == 0) {
      PresentDefaultColorBuffer();
    }
    render_pass_active_ = false;
    active_render_pass_framebuffer_ = 0;
  }

  void BlitFramebuffer(const BlitFramebufferDesc& desc) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->BlitFramebuffer(desc);
      return;
    }
    ValidateBlitFramebufferDesc(desc);
    read_framebuffer_ = desc.read_framebuffer;
    draw_framebuffer_ = desc.draw_framebuffer;
    last_blit_size_ = desc.size;
    last_clear_mask_ = desc.mask;
    last_blit_filter_ = desc.filter;
    if (HasAnyFlag(desc.mask, ClearMask::kColor)) {
      if (desc.read_framebuffer == 0 && desc.draw_framebuffer == 0) {
        CopyDefaultColorBuffer(
            desc.read_buffer, desc.draw_buffer, desc.read_origin, desc.draw_origin, desc.size, desc.draw_size, desc.filter);
      } else if (desc.read_framebuffer == 0 && desc.draw_framebuffer != 0) {
        CopyDefaultColorBufferToTexture(
            AttachedTextureId(desc.draw_framebuffer, AttachmentType::kColor, desc.draw_color_attachment_index),
            desc.read_buffer,
            desc.read_origin,
            desc.draw_origin,
            desc.size,
            desc.draw_size,
            desc.filter);
      } else if (desc.read_framebuffer != 0 && desc.draw_framebuffer == 0) {
        CopyTextureToDefaultColorBuffer(
            AttachedTextureId(desc.read_framebuffer, AttachmentType::kColor, desc.read_color_attachment_index),
            desc.draw_buffer,
            desc.read_origin,
            desc.draw_origin,
            desc.size,
            desc.draw_size,
            desc.filter);
      } else {
        CopyTextureAttachment(
            AttachedTextureId(desc.read_framebuffer, AttachmentType::kColor, desc.read_color_attachment_index),
            AttachedTextureId(desc.draw_framebuffer, AttachmentType::kColor, desc.draw_color_attachment_index),
            desc.read_origin,
            desc.draw_origin,
            desc.size,
            desc.draw_size,
            desc.filter);
      }
    }
    if (HasAnyFlag(desc.mask, ClearMask::kDepth)) {
      if (desc.read_framebuffer == 0 && desc.draw_framebuffer == 0) {
        CopyDefaultDepthBuffer(desc.read_origin, desc.draw_origin, desc.size, desc.draw_size);
      } else if (desc.read_framebuffer == 0 && desc.draw_framebuffer != 0) {
        CopyDefaultDepthBufferToTexture(
            AttachedTextureId(desc.draw_framebuffer, AttachmentType::kDepth, 0),
            desc.read_origin,
            desc.draw_origin,
            desc.size,
            desc.draw_size);
      } else if (desc.read_framebuffer != 0 && desc.draw_framebuffer == 0) {
        CopyTextureToDefaultDepthBuffer(
            AttachedTextureId(desc.read_framebuffer, AttachmentType::kDepth, 0),
            desc.read_origin,
            desc.draw_origin,
            desc.size,
            desc.draw_size);
      } else {
        CopyTextureAttachment(AttachedTextureId(desc.read_framebuffer, AttachmentType::kDepth, 0),
                              AttachedTextureId(desc.draw_framebuffer, AttachmentType::kDepth, 0),
                              desc.read_origin,
                              desc.draw_origin,
                              desc.size,
                              desc.draw_size,
                              desc.filter);
      }
    }
    if (HasAnyFlag(desc.mask, ClearMask::kStencil)) {
      if (desc.read_framebuffer == 0 && desc.draw_framebuffer == 0) {
        CopyDefaultStencilBuffer(desc.read_origin, desc.draw_origin, desc.size, desc.draw_size);
      } else if (desc.read_framebuffer == 0 && desc.draw_framebuffer != 0) {
        CopyDefaultStencilBufferToTexture(AttachedTextureId(desc.draw_framebuffer, AttachmentType::kStencil, 0),
                                          desc.read_origin,
                                          desc.draw_origin,
                                          desc.size,
                                          desc.draw_size);
      } else if (desc.read_framebuffer != 0 && desc.draw_framebuffer == 0) {
        CopyTextureToDefaultStencilBuffer(AttachedTextureId(desc.read_framebuffer, AttachmentType::kStencil, 0),
                                          desc.read_origin,
                                          desc.draw_origin,
                                          desc.size,
                                          desc.draw_size);
      } else {
        CopyTextureAttachment(AttachedTextureId(desc.read_framebuffer, AttachmentType::kStencil, 0),
                              AttachedTextureId(desc.draw_framebuffer, AttachmentType::kStencil, 0),
                              desc.read_origin,
                              desc.draw_origin,
                              desc.size,
                              desc.draw_size,
                              desc.filter);
      }
    }
  }

  void Draw(const DrawDesc& desc) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->Draw(desc);
      return;
    }
    if (desc.vertex_array != nullptr) {
      auto* vertex_array = dynamic_cast<VulkanVertexArray*>(desc.vertex_array);
      CGCHECK(vertex_array != nullptr) << "DrawDesc.vertex_array must originate from Vulkan standalone device.";
      (void)vertex_array;
    }
    if (desc.kind == DrawKind::kElements && desc.index_buffer != nullptr) {
      auto* index_buffer = dynamic_cast<VulkanBuffer*>(desc.index_buffer);
      CGCHECK(index_buffer != nullptr) << "DrawDesc.index_buffer must originate from Vulkan standalone device.";
      (void)index_buffer;
    }
    ValidateDrawState(desc);
    last_draw_call_ = DrawCallRecord{
        .kind = desc.kind == DrawKind::kElements ? DrawCallRecord::Kind::kElements : DrawCallRecord::Kind::kArrays,
        .topology = desc.topology,
        .first = desc.first,
        .count = desc.count,
        .instance_count = desc.instance_count,
    };
  }

  void ReadPixels(const ReadPixelsDesc& desc, void* data) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->ReadPixels(desc, data);
      return;
    }
    last_read_pixels_request_ = desc;
    if (data == nullptr) {
      return;
    }
    ValidateReadPixelsDesc(desc);

    const size_t destination_size = PixelPayloadSizeInBytes(desc.size, desc.format, desc.type);
    std::memset(data, 0, destination_size);

    if (desc.framebuffer == 0) {
      switch (desc.attachment_type) {
        case AttachmentType::kColor:
          ReadDefaultColorBuffer(desc, data);
          break;
        case AttachmentType::kDepth:
          ReadDefaultDepthBuffer(desc, data);
          break;
        case AttachmentType::kStencil:
          ReadDefaultStencilBuffer(desc, data);
          break;
      }
      return;
    }

    const TextureRecord* record =
        FindTextureRecord(AttachedTextureId(desc.framebuffer, desc.attachment_type, desc.attachment_index));
    const std::vector<uint8_t>* source = TextureSubresourceBytes(record, 0, 0);
    if (record == nullptr || source == nullptr) {
      return;
    }

    const glm::ivec2 texture_size = TextureLevelSize(record->meta, 0);
    if (desc.origin.x < 0 || desc.origin.y < 0 || desc.origin.x + desc.size.x > texture_size.x ||
        desc.origin.y + desc.size.y > texture_size.y) {
      return;
    }

    if (desc.attachment_type == AttachmentType::kColor) {
      const Texture::Meta destination_meta = SyntheticColorMeta(desc.format, desc.type);
      if (record->meta.pixel_format == desc.format && record->meta.pixel_type == desc.type) {
        const size_t pixel_size = PixelFormatChannelCount(desc.format) * PixelTypeSizeInBytes(desc.type);
        const size_t row_size = static_cast<size_t>(desc.size.x) * pixel_size;
        const size_t source_row_size = TextureRowSizeInBytes(record->meta, 0);
        for (int row = 0; row < desc.size.y; ++row) {
          const size_t source_offset = static_cast<size_t>(desc.origin.y + row) * source_row_size +
                                       static_cast<size_t>(desc.origin.x) * pixel_size;
          const size_t destination_offset = static_cast<size_t>(row) * row_size;
          std::memcpy(static_cast<uint8_t*>(data) + destination_offset, source->data() + source_offset, row_size);
        }
        return;
      }
      if (!SupportsColorAttachmentConversion(record->meta) || !SupportsColorAttachmentConversion(destination_meta)) {
        return;
      }
      const size_t destination_pixel_size = PixelFormatChannelCount(desc.format) * PixelTypeSizeInBytes(desc.type);
      const size_t destination_row_size = static_cast<size_t>(desc.size.x) * destination_pixel_size;
      const size_t source_row_size = TextureRowSizeInBytes(record->meta, 0);
      for (int row = 0; row < desc.size.y; ++row) {
        const uint8_t* source_row = source->data() + static_cast<size_t>(desc.origin.y + row) * source_row_size;
        uint8_t* destination_row = static_cast<uint8_t*>(data) + static_cast<size_t>(row) * destination_row_size;
        for (int column = 0; column < desc.size.x; ++column) {
          WriteColorPixel(destination_meta,
                          destination_row,
                          column,
                          ReadColorPixel(record->meta, source_row, desc.origin.x + column));
        }
      }
      return;
    }

    if (record->meta.pixel_format != desc.format || record->meta.pixel_type != desc.type) {
      return;
    }

    const size_t pixel_size = PixelFormatChannelCount(desc.format) * PixelTypeSizeInBytes(desc.type);
    const size_t row_size = static_cast<size_t>(desc.size.x) * pixel_size;
    const size_t source_row_size = TextureRowSizeInBytes(record->meta, 0);
    for (int row = 0; row < desc.size.y; ++row) {
      const size_t source_offset = static_cast<size_t>(desc.origin.y + row) * source_row_size +
                                   static_cast<size_t>(desc.origin.x) * pixel_size;
      const size_t destination_offset = static_cast<size_t>(row) * row_size;
      std::memcpy(static_cast<uint8_t*>(data) + destination_offset, source->data() + source_offset, row_size);
    }
  }

  void ApplyViewport(const glm::ivec2& origin, const glm::ivec2& size) {
    framebuffer_state_.viewport = glm::ivec4(origin.x, origin.y, size.x, size.y);
    if (draw_framebuffer_ == 0) {
      EnsureDefaultBuffersSized(size);
    }
  }

  void Clear(const ClearDesc& desc) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->Clear(desc);
      return;
    }
    last_clear_mask_ = desc.mask;
    if (draw_framebuffer_ == 0) {
      if (HasAnyFlag(desc.mask, ClearMask::kColor)) {
        ClearDefaultColorBuffer(desc.clear_color);
      }
      if (HasAnyFlag(desc.mask, ClearMask::kDepth)) {
        ClearDefaultDepthBuffer(desc.depth_clear_value);
      }
      if (HasAnyFlag(desc.mask, ClearMask::kStencil)) {
        ClearDefaultStencilBuffer(desc.stencil_clear_value);
      }
    }
    ClearFramebufferAttachments(draw_framebuffer_, desc);
  }

  void ApplyRenderState(const RenderStateDesc& state) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->ApplyRenderState(state);
      return;
    }
    if (state.depth_test_enabled.has_value()) {
      ApplyDepthTestEnabled(*state.depth_test_enabled);
    }
    if (state.cull_enabled.has_value()) {
      ApplyCullEnabled(*state.cull_enabled);
    }
    if (state.cull_mode.has_value()) {
      ApplyCullMode(*state.cull_mode);
    }
    if (state.front_face.has_value()) {
      ApplyFrontFace(*state.front_face);
    }
  }

  RenderState CaptureRenderState() const override {
    if (HasSceneCompatibilityDevice()) {
      return scene_compatibility_device_->CaptureRenderState();
    }
    return {
        .depth_test_enabled = depth_test_enabled_,
        .cull_enabled = cull_enabled_,
        .cull_mode = cull_mode_,
        .front_face = front_face_,
    };
  }

  void RestoreRenderState(const RenderState& state) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->RestoreRenderState(state);
      return;
    }
    depth_test_enabled_ = state.depth_test_enabled;
    cull_enabled_ = state.cull_enabled;
    cull_mode_ = state.cull_mode;
    front_face_ = state.front_face;
  }

 protected:
  void DidApplyBufferBindings(const std::vector<BufferBindingDesc>& bindings) override {
    (void)bindings;
  }

  void ApplyDepthTestEnabled(bool enabled) {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->ApplyRenderState({
          .depth_test_enabled = enabled,
      });
      return;
    }
    depth_test_enabled_ = enabled;
  }

  void ApplyCullEnabled(bool enabled) {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->ApplyRenderState({
          .cull_enabled = enabled,
      });
      return;
    }
    cull_enabled_ = enabled;
  }

  void ApplyCullMode(CullMode mode) {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->ApplyRenderState({
          .cull_mode = mode,
      });
      return;
    }
    cull_mode_ = mode;
  }

  void ApplyFrontFace(FrontFace winding) {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->ApplyRenderState({
          .front_face = winding,
      });
      return;
    }
    front_face_ = winding;
  }

 private:
  struct FramebufferRecord {
    std::unordered_map<uint64_t, uint32_t> attachments;
  };

  struct TextureRecord {
    Texture::Meta meta;
    std::shared_ptr<Texture::Storage> storage;
  };

  struct StorageTextureBinding {
    uint32_t texture_id = 0;
    TextureAccess access = TextureAccess::kReadOnly;
  };

  struct DefaultColorBuffer {
    glm::ivec2 size = glm::ivec2(0);
    std::vector<uint8_t> rgba8_pixels;
  };

  struct DefaultDepthBuffer {
    glm::ivec2 size = glm::ivec2(0);
    std::vector<float> depth32f_pixels;
  };

  struct DefaultStencilBuffer {
    glm::ivec2 size = glm::ivec2(0);
    std::vector<uint8_t> stencil8_pixels;
  };

  struct DrawCallRecord {
    enum class Kind {
      kNone = 0,
      kArrays = 1,
      kElements = 2,
    };

    Kind kind = Kind::kNone;
    PrimitiveTopology topology = PrimitiveTopology::kTriangles;
    uint32_t first = 0;
    uint32_t count = 0;
    uint32_t instance_count = 1;
  };

  bool HasSceneCompatibilityDevice() const {
    return scene_compatibility_device_ != nullptr;
  }

  void RecordStorageTextureBinding(uint32_t texture_unit, const Texture& texture, TextureAccess access) {
    RegisterTexture(texture);
    storage_texture_bindings_[texture_unit] = StorageTextureBinding{
        .texture_id = texture.id(),
        .access = access,
    };
  }

  void ValidateDrawState(const DrawDesc& desc) const {
    CGCHECK(render_pass_active_) << "Draw submission requires an active render pass in standalone Vulkan scene mode";
    CGCHECK(active_program_id_ != 0)
        << "Draw submission requires Program::ApplyBindings() before Draw() in standalone Vulkan scene mode";
    CGCHECK(active_program_kind_ == ProgramPipelineKind::kRender)
        << "Draw submission requires an active render program in standalone Vulkan scene mode";
    CGCHECK(desc.vertex_array != nullptr)
        << "Draw submission requires DrawDesc.vertex_array in standalone Vulkan scene mode";
    if (desc.kind == DrawKind::kElements) {
      CGCHECK(desc.index_buffer != nullptr)
          << "Indexed DrawDesc submission requires DrawDesc.index_buffer in standalone Vulkan scene mode";
    }
  }

  void ValidateDispatchState(const glm::uvec3& workgroup_count) const {
    CGCHECK(!render_pass_active_)
        << "DispatchCompute() must execute outside an active render pass in standalone Vulkan scene mode";
    CGCHECK(active_program_id_ != 0)
        << "DispatchCompute() requires Program::ApplyBindings() before dispatch in standalone Vulkan scene mode";
    CGCHECK(active_program_kind_ == ProgramPipelineKind::kCompute)
        << "DispatchCompute() requires an active compute program in standalone Vulkan scene mode";
    CGCHECK(workgroup_count.x > 0 && workgroup_count.y > 0 && workgroup_count.z > 0)
        << "DispatchCompute() requires non-zero workgroup counts in standalone Vulkan scene mode";
  }

  TextureRecord* FindTextureRecord(uint32_t texture_id) {
    auto it = texture_records_.find(texture_id);
    return it == texture_records_.end() ? nullptr : &it->second;
  }

  const TextureRecord* FindTextureRecord(uint32_t texture_id) const {
    auto it = texture_records_.find(texture_id);
    return it == texture_records_.end() ? nullptr : &it->second;
  }

  uint32_t CurrentDrawFramebuffer() const {
    return draw_framebuffer_;
  }

  static uint64_t AttachmentKey(AttachmentType attachment_type, uint32_t attachment_index) {
    return (static_cast<uint64_t>(attachment_index) << 32) | static_cast<uint32_t>(attachment_type);
  }

  uint32_t AttachedTextureId(uint32_t framebuffer, AttachmentType attachment_type, uint32_t attachment_index) const {
    const auto framebuffer_it = framebuffers_.find(framebuffer);
    if (framebuffer_it == framebuffers_.end()) {
      return 0;
    }
    const auto attachment_it =
        framebuffer_it->second.attachments.find(AttachmentKey(attachment_type, attachment_index));
    return attachment_it == framebuffer_it->second.attachments.end() ? 0u : attachment_it->second;
  }

  void ValidateReadPixelsDesc(const ReadPixelsDesc& desc) const {
    ValidateFramebufferAttachmentAccess(
        desc.framebuffer, desc.attachment_type, desc.attachment_index, "ReadPixels()", "read");
  }

  void ValidateBlitFramebufferDesc(const BlitFramebufferDesc& desc) const {
    if (HasAnyFlag(desc.mask, ClearMask::kColor)) {
      ValidateFramebufferAttachmentAccess(desc.read_framebuffer,
                                          AttachmentType::kColor,
                                          desc.read_color_attachment_index,
                                          "BlitFramebuffer()",
                                          "source");
      ValidateFramebufferAttachmentAccess(desc.draw_framebuffer,
                                          AttachmentType::kColor,
                                          desc.draw_color_attachment_index,
                                          "BlitFramebuffer()",
                                          "destination");
    }
    if (HasAnyFlag(desc.mask, ClearMask::kDepth)) {
      ValidateFramebufferAttachmentAccess(
          desc.read_framebuffer, AttachmentType::kDepth, 0, "BlitFramebuffer()", "source");
      ValidateFramebufferAttachmentAccess(
          desc.draw_framebuffer, AttachmentType::kDepth, 0, "BlitFramebuffer()", "destination");
    }
    if (HasAnyFlag(desc.mask, ClearMask::kStencil)) {
      ValidateFramebufferAttachmentAccess(
          desc.read_framebuffer, AttachmentType::kStencil, 0, "BlitFramebuffer()", "source");
      ValidateFramebufferAttachmentAccess(
          desc.draw_framebuffer, AttachmentType::kStencil, 0, "BlitFramebuffer()", "destination");
    }
  }

  void ValidateFramebufferAttachmentAccess(uint32_t framebuffer,
                                           AttachmentType attachment_type,
                                           uint32_t attachment_index,
                                           const char* operation_name,
                                           const char* attachment_role) const {
    if (framebuffer == 0) {
      CGCHECK(attachment_index == 0)
          << "Default framebuffer only supports " << AttachmentTypeName(attachment_type)
          << " attachment 0 for " << operation_name << " in standalone Vulkan scene mode";
      return;
    }
    CGCHECK(AttachedTextureId(framebuffer, attachment_type, attachment_index) != 0)
        << operation_name << " requires " << attachment_role << " "
        << AttachmentTypeName(attachment_type) << " attachment " << attachment_index
        << " to exist on framebuffer " << framebuffer << " in standalone Vulkan scene mode";
  }

  static const char* AttachmentTypeName(AttachmentType attachment_type) {
    switch (attachment_type) {
      case AttachmentType::kColor:
        return "color";
      case AttachmentType::kDepth:
        return "depth";
      case AttachmentType::kStencil:
        return "stencil";
    }
    return "unknown";
  }

  bool IsFramebufferComplete(const FramebufferRecord& framebuffer) const {
    if (framebuffer.attachments.empty()) {
      return false;
    }

    glm::ivec2 attachment_size(-1, -1);
    for (const auto& attachment : framebuffer.attachments) {
      const TextureRecord* record = FindTextureRecord(attachment.second);
      if (record == nullptr) {
        return false;
      }
      const glm::ivec2 size = TextureLevelSize(record->meta, 0);
      if (attachment_size.x < 0) {
        attachment_size = size;
      } else if (attachment_size != size) {
        return false;
      }
    }
    return true;
  }

  void RegisterTexture(const Texture& texture) {
    texture_records_[texture.id()] = TextureRecord{
        .meta = texture.meta(),
        .storage = texture.storage(),
    };
  }

  std::vector<uint8_t>* MutableTextureSubresource(TextureRecord* record, int face, int level) {
    if (record == nullptr || record->storage == nullptr) {
      return nullptr;
    }
    const size_t subresource_index = TextureSubresourceIndex(record->meta, face, level);
    if (record->storage->cpu_levels.size() <= subresource_index) {
      record->storage->cpu_levels.resize(subresource_index + 1);
    }
    std::vector<uint8_t>* bytes = &record->storage->cpu_levels[subresource_index];
    const size_t expected_size =
        PixelPayloadSizeInBytes(TextureLevelSize(record->meta, level), record->meta.pixel_format, record->meta.pixel_type);
    if (bytes->size() != expected_size) {
      bytes->resize(expected_size);
    }
    return bytes;
  }

  const std::vector<uint8_t>* TextureSubresourceBytes(const TextureRecord* record, int face, int level) const {
    if (record == nullptr || record->storage == nullptr) {
      return nullptr;
    }
    const size_t subresource_index = TextureSubresourceIndex(record->meta, face, level);
    if (record->storage->cpu_levels.size() <= subresource_index) {
      return nullptr;
    }
    return &record->storage->cpu_levels[subresource_index];
  }

  void ClearTextureWithColor(TextureRecord* record, const glm::vec4& color) {
    std::vector<uint8_t>* bytes = MutableTextureSubresource(record, 0, 0);
    if (bytes == nullptr) {
      return;
    }
    if (record->meta.pixel_format == PixelFormat::kRGBA && record->meta.pixel_type == PixelType::kUInt8) {
      const std::array<uint8_t, 4> pixel = {
          QuantizeFloatToUNorm8(color.r),
          QuantizeFloatToUNorm8(color.g),
          QuantizeFloatToUNorm8(color.b),
          QuantizeFloatToUNorm8(color.a),
      };
      for (size_t offset = 0; offset + pixel.size() <= bytes->size(); offset += pixel.size()) {
        std::memcpy(bytes->data() + offset, pixel.data(), pixel.size());
      }
      return;
    }
    if (record->meta.pixel_format == PixelFormat::kRGBA && record->meta.pixel_type == PixelType::kFloat32) {
      const std::array<float, 4> pixel = {color.r, color.g, color.b, color.a};
      for (size_t offset = 0; offset + sizeof(pixel) <= bytes->size(); offset += sizeof(pixel)) {
        std::memcpy(bytes->data() + offset, pixel.data(), sizeof(pixel));
      }
      return;
    }
    if (record->meta.pixel_format == PixelFormat::kRG && record->meta.pixel_type == PixelType::kFloat32) {
      const std::array<float, 2> pixel = {color.r, color.g};
      for (size_t offset = 0; offset + sizeof(pixel) <= bytes->size(); offset += sizeof(pixel)) {
        std::memcpy(bytes->data() + offset, pixel.data(), sizeof(pixel));
      }
      return;
    }
    if (record->meta.pixel_format == PixelFormat::kRedInteger && record->meta.pixel_type == PixelType::kUInt32) {
      const uint32_t value = static_cast<uint32_t>(std::max(color.r, 0.0f));
      for (size_t offset = 0; offset + sizeof(value) <= bytes->size(); offset += sizeof(value)) {
        std::memcpy(bytes->data() + offset, &value, sizeof(value));
      }
    }
  }

  void ClearTextureWithDepth(TextureRecord* record, float depth_value) {
    std::vector<uint8_t>* bytes = MutableTextureSubresource(record, 0, 0);
    if (bytes == nullptr) {
      return;
    }
    if (record->meta.pixel_format != PixelFormat::kDepthComponent || record->meta.pixel_type != PixelType::kFloat32) {
      return;
    }
    for (size_t offset = 0; offset + sizeof(depth_value) <= bytes->size(); offset += sizeof(depth_value)) {
      std::memcpy(bytes->data() + offset, &depth_value, sizeof(depth_value));
    }
  }

  void ClearTextureWithStencil(TextureRecord* record, int stencil_value) {
    std::vector<uint8_t>* bytes = MutableTextureSubresource(record, 0, 0);
    if (bytes == nullptr) {
      return;
    }
    if (record->meta.pixel_format != PixelFormat::kStencilIndex || record->meta.pixel_type != PixelType::kUInt8) {
      return;
    }
    const uint8_t value = static_cast<uint8_t>(std::clamp(stencil_value, 0, 255));
    std::fill(bytes->begin(), bytes->end(), value);
  }

  void ClearFramebufferAttachments(uint32_t framebuffer, const ClearDesc& desc) {
    const auto framebuffer_it = framebuffers_.find(framebuffer);
    if (framebuffer_it == framebuffers_.end()) {
      return;
    }
    if (HasAnyFlag(desc.mask, ClearMask::kColor)) {
      for (const auto& attachment : framebuffer_it->second.attachments) {
        const AttachmentType attachment_type = static_cast<AttachmentType>(attachment.first & 0xffffffffu);
        if (attachment_type != AttachmentType::kColor) {
          continue;
        }
        if (TextureRecord* record = FindTextureRecord(attachment.second); record != nullptr) {
          ClearTextureWithColor(record, desc.clear_color);
        }
      }
    }
    if (HasAnyFlag(desc.mask, ClearMask::kDepth)) {
      if (TextureRecord* record = FindTextureRecord(AttachedTextureId(framebuffer, AttachmentType::kDepth, 0));
          record != nullptr) {
        ClearTextureWithDepth(record, desc.depth_clear_value);
      }
    }
    if (HasAnyFlag(desc.mask, ClearMask::kStencil)) {
      if (TextureRecord* record = FindTextureRecord(AttachedTextureId(framebuffer, AttachmentType::kStencil, 0));
          record != nullptr) {
        ClearTextureWithStencil(record, desc.stencil_clear_value);
      }
    }
  }

  void CopyTextureAttachment(uint32_t source_texture_id,
                             uint32_t destination_texture_id,
                             const glm::ivec2& source_origin,
                             const glm::ivec2& destination_origin,
                             const glm::ivec2& source_size_request,
                             const glm::ivec2& destination_size_request,
                             FilterMode filter) {
    const TextureRecord* source = FindTextureRecord(source_texture_id);
    TextureRecord* destination = FindTextureRecord(destination_texture_id);
    if (source == nullptr || destination == nullptr || source->storage == nullptr || destination->storage == nullptr) {
      return;
    }
    const std::vector<uint8_t>* source_bytes = TextureSubresourceBytes(source, 0, 0);
    std::vector<uint8_t>* destination_bytes = MutableTextureSubresource(destination, 0, 0);
    if (source_bytes == nullptr || destination_bytes == nullptr) {
      return;
    }

    const glm::ivec2 source_size = TextureLevelSize(source->meta, 0);
    const glm::ivec2 destination_size = TextureLevelSize(destination->meta, 0);
    const BlitExtents extents = ResolveBlitExtents(
        source_size_request, destination_size_request, source_origin, source_size, destination_origin, destination_size);
    if (extents.source.x <= 0 || extents.source.y <= 0 || extents.destination.x <= 0 || extents.destination.y <= 0) {
      return;
    }

    const bool same_format = source->meta.pixel_format == destination->meta.pixel_format &&
                             source->meta.pixel_type == destination->meta.pixel_type;
    const bool use_sampled_color_copy =
        SupportsColorAttachmentConversion(source->meta) && SupportsColorAttachmentConversion(destination->meta) &&
        (!same_format || (IsScaledBlit(extents) && filter != FilterMode::kNearest));

    if (same_format && !use_sampled_color_copy) {
      const size_t pixel_size =
          PixelFormatChannelCount(source->meta.pixel_format) * PixelTypeSizeInBytes(source->meta.pixel_type);
      const size_t source_row_size = TextureRowSizeInBytes(source->meta, 0);
      const size_t destination_row_size = TextureRowSizeInBytes(destination->meta, 0);
      const bool same_storage = source_texture_id == destination_texture_id;
      std::vector<uint8_t> source_snapshot;
      const uint8_t* source_copy_bytes = source_bytes->data();
      size_t source_copy_row_size = source_row_size;
      glm::ivec2 source_copy_origin = extents.source_origin;
      if (same_storage) {
        source_snapshot =
            SnapshotPixelRect(source_bytes->data(),
                              source_row_size,
                              static_cast<int>(pixel_size),
                              extents.source_origin,
                              extents.source);
        source_copy_bytes = source_snapshot.data();
        source_copy_row_size = static_cast<size_t>(extents.source.x) * pixel_size;
        source_copy_origin = {0, 0};
      }
      CopyExactPixelRect(source_copy_bytes,
                         source_copy_row_size,
                         static_cast<int>(pixel_size),
                         source_copy_origin,
                         extents.source,
                         destination_bytes->data(),
                         destination_row_size,
                         extents.destination_origin,
                         extents.destination);
      return;
    }

    if (!use_sampled_color_copy) {
      return;
    }

    const size_t pixel_size =
        PixelFormatChannelCount(source->meta.pixel_format) * PixelTypeSizeInBytes(source->meta.pixel_type);
    const size_t source_row_size = TextureRowSizeInBytes(source->meta, 0);
    const size_t destination_row_size = TextureRowSizeInBytes(destination->meta, 0);
    const bool same_storage = source_texture_id == destination_texture_id;
    std::vector<uint8_t> source_snapshot;
    const uint8_t* sampled_source_bytes = source_bytes->data();
    size_t sampled_source_row_size = source_row_size;
    glm::ivec2 sampled_source_origin = extents.source_origin;
    if (same_storage) {
      source_snapshot =
          SnapshotPixelRect(source_bytes->data(),
                            source_row_size,
                            static_cast<int>(pixel_size),
                            extents.source_origin,
                            extents.source);
      sampled_source_bytes = source_snapshot.data();
      sampled_source_row_size = static_cast<size_t>(extents.source.x) * pixel_size;
      sampled_source_origin = {0, 0};
    }
    CopySampledColorRect(source->meta,
                         sampled_source_bytes,
                         sampled_source_row_size,
                         sampled_source_origin,
                         extents.source,
                         destination->meta,
                         destination_bytes->data(),
                         destination_row_size,
                         extents.destination_origin,
                         extents.destination,
                         filter);
  }

  void EnsureDefaultBuffersSized(const glm::ivec2& size) {
    EnsureDefaultColorBufferSized(size);
    EnsureDefaultDepthBufferSized(size);
    EnsureDefaultStencilBufferSized(size);
  }

  static void ResizeDefaultColorBuffer(DefaultColorBuffer* buffer, const glm::ivec2& size) {
    if (buffer == nullptr || size.x <= 0 || size.y <= 0) {
      return;
    }
    if (buffer->size == size &&
        buffer->rgba8_pixels.size() == PixelPayloadSizeInBytes(size, PixelFormat::kRGBA, PixelType::kUInt8)) {
      return;
    }
    buffer->size = size;
    buffer->rgba8_pixels.resize(PixelPayloadSizeInBytes(size, PixelFormat::kRGBA, PixelType::kUInt8));
  }

  void EnsureDefaultColorBufferSized(const glm::ivec2& size) {
    ResizeDefaultColorBuffer(&default_color_back_buffer_, size);
    ResizeDefaultColorBuffer(&default_color_front_buffer_, size);
  }

  DefaultColorBuffer& DefaultColorBackBuffer() {
    return default_color_back_buffer_;
  }

  const DefaultColorBuffer& DefaultColorBackBuffer() const {
    return default_color_back_buffer_;
  }

  DefaultColorBuffer& DefaultColorWriteBuffer(ReadBuffer draw_buffer) {
    return draw_buffer == ReadBuffer::kFront ? default_color_front_buffer_ : default_color_back_buffer_;
  }

  const DefaultColorBuffer& DefaultColorReadBuffer(ReadBuffer read_buffer) const {
    return read_buffer == ReadBuffer::kFront ? default_color_front_buffer_ : default_color_back_buffer_;
  }

  void PresentDefaultColorBuffer() {
    EnsureDefaultColorBufferSized(DefaultColorBackBuffer().size);
    default_color_front_buffer_ = DefaultColorBackBuffer();
  }

  void MaybePresentDefaultColorBuffer(ReadBuffer draw_buffer) {
    if (draw_buffer == ReadBuffer::kBack && !(render_pass_active_ && active_render_pass_framebuffer_ == 0)) {
      PresentDefaultColorBuffer();
    }
  }

  void EnsureDefaultDepthBufferSized(const glm::ivec2& size) {
    if (size.x <= 0 || size.y <= 0) {
      return;
    }
    const size_t pixel_count = static_cast<size_t>(size.x) * static_cast<size_t>(size.y);
    if (default_depth_buffer_.size == size && default_depth_buffer_.depth32f_pixels.size() == pixel_count) {
      return;
    }
    default_depth_buffer_.size = size;
    default_depth_buffer_.depth32f_pixels.resize(pixel_count);
  }

  void EnsureDefaultStencilBufferSized(const glm::ivec2& size) {
    if (size.x <= 0 || size.y <= 0) {
      return;
    }
    const size_t pixel_count = static_cast<size_t>(size.x) * static_cast<size_t>(size.y);
    if (default_stencil_buffer_.size == size && default_stencil_buffer_.stencil8_pixels.size() == pixel_count) {
      return;
    }
    default_stencil_buffer_.size = size;
    default_stencil_buffer_.stencil8_pixels.resize(pixel_count);
  }

  void ClearDefaultColorBuffer(const glm::vec4& clear_color) {
    EnsureDefaultColorBufferSized({framebuffer_state_.viewport.z, framebuffer_state_.viewport.w});
    DefaultColorBuffer& back_buffer = DefaultColorBackBuffer();
    const std::array<uint8_t, 4> pixel = {
        QuantizeFloatToUNorm8(clear_color.r),
        QuantizeFloatToUNorm8(clear_color.g),
        QuantizeFloatToUNorm8(clear_color.b),
        QuantizeFloatToUNorm8(clear_color.a),
    };
    for (size_t offset = 0; offset + pixel.size() <= back_buffer.rgba8_pixels.size(); offset += pixel.size()) {
      std::memcpy(back_buffer.rgba8_pixels.data() + offset, pixel.data(), pixel.size());
    }
    MaybePresentDefaultColorBuffer(ReadBuffer::kBack);
  }

  void ClearDefaultDepthBuffer(float clear_depth_value) {
    EnsureDefaultDepthBufferSized({framebuffer_state_.viewport.z, framebuffer_state_.viewport.w});
    std::fill(default_depth_buffer_.depth32f_pixels.begin(), default_depth_buffer_.depth32f_pixels.end(), clear_depth_value);
  }

  void ClearDefaultStencilBuffer(int clear_stencil_value) {
    EnsureDefaultStencilBufferSized({framebuffer_state_.viewport.z, framebuffer_state_.viewport.w});
    std::fill(default_stencil_buffer_.stencil8_pixels.begin(),
              default_stencil_buffer_.stencil8_pixels.end(),
              static_cast<uint8_t>(std::clamp(clear_stencil_value, 0, 255)));
  }

  void CopyTextureToDefaultColorBuffer(uint32_t source_texture_id,
                                       ReadBuffer draw_buffer,
                                       const glm::ivec2& source_origin,
                                       const glm::ivec2& destination_origin,
                                       const glm::ivec2& source_size_request,
                                       const glm::ivec2& destination_size_request,
                                       FilterMode filter) {
    const TextureRecord* source = FindTextureRecord(source_texture_id);
    const std::vector<uint8_t>* source_bytes = TextureSubresourceBytes(source, 0, 0);
    if (source == nullptr || source_bytes == nullptr) {
      return;
    }
    const glm::ivec2 source_size = TextureLevelSize(source->meta, 0);
    DefaultColorBuffer& destination_buffer = DefaultColorWriteBuffer(draw_buffer);
    if (destination_buffer.size.x <= 0 || destination_buffer.size.y <= 0) {
      EnsureDefaultColorBufferSized(
          ResolveUninitializedBlitDestinationSize(
              source_size_request, destination_size_request, source_origin, source_size, destination_origin));
    }
    const BlitExtents extents = ResolveBlitExtents(
        source_size_request, destination_size_request, source_origin, source_size, destination_origin, destination_buffer.size);
    if (extents.source.x <= 0 || extents.source.y <= 0 || extents.destination.x <= 0 || extents.destination.y <= 0) {
      return;
    }
    const Texture::Meta destination_meta = SyntheticColorMeta(PixelFormat::kRGBA, PixelType::kUInt8);
    const size_t destination_row_size = static_cast<size_t>(destination_buffer.size.x) * 4;
    if (source->meta.pixel_format == PixelFormat::kRGBA && source->meta.pixel_type == PixelType::kUInt8) {
      const size_t source_row_size = TextureRowSizeInBytes(source->meta, 0);
      if (filter == FilterMode::kNearest || !IsScaledBlit(extents)) {
        CopyExactPixelRect(source_bytes->data(),
                           source_row_size,
                           4,
                           extents.source_origin,
                           extents.source,
                           destination_buffer.rgba8_pixels.data(),
                           destination_row_size,
                           extents.destination_origin,
                           extents.destination);
      } else {
        CopySampledColorRect(source->meta,
                             source_bytes->data(),
                             source_row_size,
                             extents.source_origin,
                             extents.source,
                             destination_meta,
                             destination_buffer.rgba8_pixels.data(),
                             destination_row_size,
                             extents.destination_origin,
                             extents.destination,
                             filter);
      }
      MaybePresentDefaultColorBuffer(draw_buffer);
      return;
    }
    if (!SupportsColorAttachmentConversion(source->meta)) {
      return;
    }
    CopySampledColorRect(source->meta,
                         source_bytes->data(),
                         TextureRowSizeInBytes(source->meta, 0),
                         extents.source_origin,
                         extents.source,
                         destination_meta,
                         destination_buffer.rgba8_pixels.data(),
                         destination_row_size,
                         extents.destination_origin,
                         extents.destination,
                         filter);
    MaybePresentDefaultColorBuffer(draw_buffer);
  }

  void CopyTextureToDefaultDepthBuffer(uint32_t source_texture_id,
                                       const glm::ivec2& source_origin,
                                       const glm::ivec2& destination_origin,
                                       const glm::ivec2& source_size_request,
                                       const glm::ivec2& destination_size_request) {
    const TextureRecord* source = FindTextureRecord(source_texture_id);
    const std::vector<uint8_t>* source_bytes = TextureSubresourceBytes(source, 0, 0);
    if (source == nullptr || source_bytes == nullptr || source->meta.pixel_format != PixelFormat::kDepthComponent ||
        source->meta.pixel_type != PixelType::kFloat32) {
      return;
    }
    const glm::ivec2 source_size = TextureLevelSize(source->meta, 0);
    if (default_depth_buffer_.size.x <= 0 || default_depth_buffer_.size.y <= 0) {
      EnsureDefaultDepthBufferSized(
          ResolveUninitializedBlitDestinationSize(
              source_size_request, destination_size_request, source_origin, source_size, destination_origin));
    }
    const BlitExtents extents = ResolveBlitExtents(
        source_size_request, destination_size_request, source_origin, source_size, destination_origin, default_depth_buffer_.size);
    if (extents.source.x <= 0 || extents.source.y <= 0 || extents.destination.x <= 0 || extents.destination.y <= 0) {
      return;
    }
    const size_t source_row_size = TextureRowSizeInBytes(source->meta, 0);
    const size_t destination_row_size = static_cast<size_t>(default_depth_buffer_.size.x) * sizeof(float);
    CopyExactPixelRect(source_bytes->data(),
                       source_row_size,
                       static_cast<int>(sizeof(float)),
                       extents.source_origin,
                       extents.source,
                       reinterpret_cast<uint8_t*>(default_depth_buffer_.depth32f_pixels.data()),
                       destination_row_size,
                       extents.destination_origin,
                       extents.destination);
  }

  void CopyTextureToDefaultStencilBuffer(uint32_t source_texture_id,
                                         const glm::ivec2& source_origin,
                                         const glm::ivec2& destination_origin,
                                         const glm::ivec2& source_size_request,
                                         const glm::ivec2& destination_size_request) {
    const TextureRecord* source = FindTextureRecord(source_texture_id);
    const std::vector<uint8_t>* source_bytes = TextureSubresourceBytes(source, 0, 0);
    if (source == nullptr || source_bytes == nullptr || source->meta.pixel_format != PixelFormat::kStencilIndex ||
        source->meta.pixel_type != PixelType::kUInt8) {
      return;
    }
    const glm::ivec2 source_size = TextureLevelSize(source->meta, 0);
    if (default_stencil_buffer_.size.x <= 0 || default_stencil_buffer_.size.y <= 0) {
      EnsureDefaultStencilBufferSized(
          ResolveUninitializedBlitDestinationSize(
              source_size_request, destination_size_request, source_origin, source_size, destination_origin));
    }
    const BlitExtents extents = ResolveBlitExtents(source_size_request,
                                                   destination_size_request,
                                                   source_origin,
                                                   source_size,
                                                   destination_origin,
                                                   default_stencil_buffer_.size);
    if (extents.source.x <= 0 || extents.source.y <= 0 || extents.destination.x <= 0 || extents.destination.y <= 0) {
      return;
    }
    const size_t source_row_size = TextureRowSizeInBytes(source->meta, 0);
    const size_t destination_row_size = static_cast<size_t>(default_stencil_buffer_.size.x);
    CopyExactPixelRect(source_bytes->data(),
                       source_row_size,
                       1,
                       extents.source_origin,
                       extents.source,
                       default_stencil_buffer_.stencil8_pixels.data(),
                       destination_row_size,
                       extents.destination_origin,
                       extents.destination);
  }

  void CopyDefaultColorBufferToTexture(uint32_t destination_texture_id,
                                       ReadBuffer read_buffer,
                                       const glm::ivec2& source_origin,
                                       const glm::ivec2& destination_origin,
                                       const glm::ivec2& source_size_request,
                                       const glm::ivec2& destination_size_request,
                                       FilterMode filter) {
    TextureRecord* destination = FindTextureRecord(destination_texture_id);
    if (destination == nullptr) {
      return;
    }
    std::vector<uint8_t>* destination_bytes = MutableTextureSubresource(destination, 0, 0);
    const DefaultColorBuffer& source_buffer = DefaultColorReadBuffer(read_buffer);
    if (destination_bytes == nullptr || source_buffer.rgba8_pixels.empty()) {
      return;
    }
    const glm::ivec2 destination_size = TextureLevelSize(destination->meta, 0);
    const BlitExtents extents = ResolveBlitExtents(
        source_size_request, destination_size_request, source_origin, source_buffer.size, destination_origin, destination_size);
    if (extents.source.x <= 0 || extents.source.y <= 0 || extents.destination.x <= 0 || extents.destination.y <= 0) {
      return;
    }
    if (destination->meta.pixel_format == PixelFormat::kRGBA && destination->meta.pixel_type == PixelType::kUInt8) {
      const size_t source_row_size = static_cast<size_t>(source_buffer.size.x) * 4;
      const size_t destination_row_size = static_cast<size_t>(destination_size.x) * 4;
      if (filter == FilterMode::kNearest || !IsScaledBlit(extents)) {
        CopyExactPixelRect(source_buffer.rgba8_pixels.data(),
                           source_row_size,
                           4,
                           extents.source_origin,
                           extents.source,
                           destination_bytes->data(),
                           destination_row_size,
                           extents.destination_origin,
                           extents.destination);
      } else {
        const Texture::Meta source_meta = SyntheticColorMeta(PixelFormat::kRGBA, PixelType::kUInt8);
        CopySampledColorRect(source_meta,
                             source_buffer.rgba8_pixels.data(),
                             source_row_size,
                             extents.source_origin,
                             extents.source,
                             destination->meta,
                             destination_bytes->data(),
                             destination_row_size,
                             extents.destination_origin,
                             extents.destination,
                             filter);
      }
      return;
    }
    if (!SupportsColorAttachmentConversion(destination->meta)) {
      return;
    }
    const Texture::Meta source_meta = SyntheticColorMeta(PixelFormat::kRGBA, PixelType::kUInt8);
    CopySampledColorRect(source_meta,
                         source_buffer.rgba8_pixels.data(),
                         static_cast<size_t>(source_buffer.size.x) * 4,
                         extents.source_origin,
                         extents.source,
                         destination->meta,
                         destination_bytes->data(),
                         TextureRowSizeInBytes(destination->meta, 0),
                         extents.destination_origin,
                         extents.destination,
                         filter);
  }

  void CopyDefaultColorBuffer(ReadBuffer read_buffer,
                              ReadBuffer draw_buffer,
                              const glm::ivec2& source_origin,
                              const glm::ivec2& destination_origin,
                              const glm::ivec2& source_size_request,
                              const glm::ivec2& destination_size_request,
                              FilterMode filter) {
    const DefaultColorBuffer& source_buffer = DefaultColorReadBuffer(read_buffer);
    if (source_buffer.rgba8_pixels.empty()) {
      return;
    }
    DefaultColorBuffer& destination_buffer = DefaultColorWriteBuffer(draw_buffer);
    if (destination_buffer.size.x <= 0 || destination_buffer.size.y <= 0) {
      EnsureDefaultColorBufferSized(
          ResolveUninitializedBlitDestinationSize(
              source_size_request, destination_size_request, source_origin, source_buffer.size, destination_origin));
    }
    const BlitExtents extents = ResolveBlitExtents(
        source_size_request, destination_size_request, source_origin, source_buffer.size, destination_origin, destination_buffer.size);
    if (extents.source.x <= 0 || extents.source.y <= 0 || extents.destination.x <= 0 || extents.destination.y <= 0) {
      return;
    }
    const size_t source_row_size = static_cast<size_t>(source_buffer.size.x) * 4;
    const size_t destination_row_size = static_cast<size_t>(destination_buffer.size.x) * 4;
    const bool same_buffer = &source_buffer == &destination_buffer;
    std::vector<uint8_t> source_snapshot;
    const uint8_t* source_copy_bytes = source_buffer.rgba8_pixels.data();
    size_t source_copy_row_size = source_row_size;
    glm::ivec2 source_copy_origin = extents.source_origin;
    if (same_buffer) {
      source_snapshot =
          SnapshotPixelRect(source_buffer.rgba8_pixels.data(), source_row_size, 4, extents.source_origin, extents.source);
      source_copy_bytes = source_snapshot.data();
      source_copy_row_size = static_cast<size_t>(extents.source.x) * 4;
      source_copy_origin = {0, 0};
    }
    if (filter == FilterMode::kNearest || !IsScaledBlit(extents)) {
      CopyExactPixelRect(source_copy_bytes,
                         source_copy_row_size,
                         4,
                         source_copy_origin,
                         extents.source,
                         destination_buffer.rgba8_pixels.data(),
                         destination_row_size,
                         extents.destination_origin,
                         extents.destination);
    } else {
      const Texture::Meta rgba8_meta = SyntheticColorMeta(PixelFormat::kRGBA, PixelType::kUInt8);
      CopySampledColorRect(rgba8_meta,
                           source_copy_bytes,
                           source_copy_row_size,
                           source_copy_origin,
                           extents.source,
                           rgba8_meta,
                           destination_buffer.rgba8_pixels.data(),
                           destination_row_size,
                           extents.destination_origin,
                           extents.destination,
                           filter);
    }
    MaybePresentDefaultColorBuffer(draw_buffer);
  }

  void CopyDefaultDepthBuffer(const glm::ivec2& source_origin,
                              const glm::ivec2& destination_origin,
                              const glm::ivec2& source_size_request,
                              const glm::ivec2& destination_size_request) {
    if (default_depth_buffer_.depth32f_pixels.empty()) {
      return;
    }
    const BlitExtents extents = ResolveBlitExtents(source_size_request,
                                                   destination_size_request,
                                                   source_origin,
                                                   default_depth_buffer_.size,
                                                   destination_origin,
                                                   default_depth_buffer_.size);
    if (extents.source.x <= 0 || extents.source.y <= 0 || extents.destination.x <= 0 || extents.destination.y <= 0) {
      return;
    }
    const size_t source_row_size = static_cast<size_t>(default_depth_buffer_.size.x) * sizeof(float);
    const size_t destination_row_size = static_cast<size_t>(default_depth_buffer_.size.x) * sizeof(float);
    const std::vector<uint8_t> source_snapshot = SnapshotPixelRect(
        reinterpret_cast<const uint8_t*>(default_depth_buffer_.depth32f_pixels.data()),
        source_row_size,
        static_cast<int>(sizeof(float)),
        extents.source_origin,
        extents.source);
    CopyExactPixelRect(source_snapshot.data(),
                       static_cast<size_t>(extents.source.x) * sizeof(float),
                       static_cast<int>(sizeof(float)),
                       {0, 0},
                       extents.source,
                       reinterpret_cast<uint8_t*>(default_depth_buffer_.depth32f_pixels.data()),
                       destination_row_size,
                       extents.destination_origin,
                       extents.destination);
  }

  void CopyDefaultStencilBuffer(const glm::ivec2& source_origin,
                                const glm::ivec2& destination_origin,
                                const glm::ivec2& source_size_request,
                                const glm::ivec2& destination_size_request) {
    if (default_stencil_buffer_.stencil8_pixels.empty()) {
      return;
    }
    const BlitExtents extents = ResolveBlitExtents(source_size_request,
                                                   destination_size_request,
                                                   source_origin,
                                                   default_stencil_buffer_.size,
                                                   destination_origin,
                                                   default_stencil_buffer_.size);
    if (extents.source.x <= 0 || extents.source.y <= 0 || extents.destination.x <= 0 || extents.destination.y <= 0) {
      return;
    }
    const size_t source_row_size = static_cast<size_t>(default_stencil_buffer_.size.x);
    const size_t destination_row_size = static_cast<size_t>(default_stencil_buffer_.size.x);
    const std::vector<uint8_t> source_snapshot =
        SnapshotPixelRect(
            default_stencil_buffer_.stencil8_pixels.data(), source_row_size, 1, extents.source_origin, extents.source);
    CopyExactPixelRect(source_snapshot.data(),
                       static_cast<size_t>(extents.source.x),
                       1,
                       {0, 0},
                       extents.source,
                       default_stencil_buffer_.stencil8_pixels.data(),
                       destination_row_size,
                       extents.destination_origin,
                       extents.destination);
  }

  void CopyDefaultDepthBufferToTexture(uint32_t destination_texture_id,
                                       const glm::ivec2& source_origin,
                                       const glm::ivec2& destination_origin,
                                       const glm::ivec2& source_size_request,
                                       const glm::ivec2& destination_size_request) {
    TextureRecord* destination = FindTextureRecord(destination_texture_id);
    if (destination == nullptr || destination->meta.pixel_format != PixelFormat::kDepthComponent ||
        destination->meta.pixel_type != PixelType::kFloat32) {
      return;
    }
    std::vector<uint8_t>* destination_bytes = MutableTextureSubresource(destination, 0, 0);
    if (destination_bytes == nullptr || default_depth_buffer_.depth32f_pixels.empty()) {
      return;
    }
    const glm::ivec2 destination_size = TextureLevelSize(destination->meta, 0);
    const BlitExtents extents = ResolveBlitExtents(source_size_request,
                                                   destination_size_request,
                                                   source_origin,
                                                   default_depth_buffer_.size,
                                                   destination_origin,
                                                   destination_size);
    if (extents.source.x <= 0 || extents.source.y <= 0 || extents.destination.x <= 0 || extents.destination.y <= 0) {
      return;
    }
    const size_t source_row_size = static_cast<size_t>(default_depth_buffer_.size.x) * sizeof(float);
    const size_t destination_row_size = static_cast<size_t>(destination_size.x) * sizeof(float);
    CopyExactPixelRect(reinterpret_cast<const uint8_t*>(default_depth_buffer_.depth32f_pixels.data()),
                       source_row_size,
                       static_cast<int>(sizeof(float)),
                       extents.source_origin,
                       extents.source,
                       destination_bytes->data(),
                       destination_row_size,
                       extents.destination_origin,
                       extents.destination);
  }

  void CopyDefaultStencilBufferToTexture(uint32_t destination_texture_id,
                                         const glm::ivec2& source_origin,
                                         const glm::ivec2& destination_origin,
                                         const glm::ivec2& source_size_request,
                                         const glm::ivec2& destination_size_request) {
    TextureRecord* destination = FindTextureRecord(destination_texture_id);
    if (destination == nullptr || destination->meta.pixel_format != PixelFormat::kStencilIndex ||
        destination->meta.pixel_type != PixelType::kUInt8) {
      return;
    }
    std::vector<uint8_t>* destination_bytes = MutableTextureSubresource(destination, 0, 0);
    if (destination_bytes == nullptr || default_stencil_buffer_.stencil8_pixels.empty()) {
      return;
    }
    const glm::ivec2 destination_size = TextureLevelSize(destination->meta, 0);
    const BlitExtents extents = ResolveBlitExtents(source_size_request,
                                                   destination_size_request,
                                                   source_origin,
                                                   default_stencil_buffer_.size,
                                                   destination_origin,
                                                   destination_size);
    if (extents.source.x <= 0 || extents.source.y <= 0 || extents.destination.x <= 0 || extents.destination.y <= 0) {
      return;
    }
    const size_t source_row_size = static_cast<size_t>(default_stencil_buffer_.size.x);
    const size_t destination_row_size = static_cast<size_t>(destination_size.x);
    CopyExactPixelRect(default_stencil_buffer_.stencil8_pixels.data(),
                       source_row_size,
                       1,
                       extents.source_origin,
                       extents.source,
                       destination_bytes->data(),
                       destination_row_size,
                       extents.destination_origin,
                       extents.destination);
  }

  void ReadDefaultColorBuffer(const ReadPixelsDesc& desc, void* data) const {
    const DefaultColorBuffer& read_buffer = DefaultColorReadBuffer(desc.read_buffer);
    if (read_buffer.size.x <= 0 || read_buffer.size.y <= 0) {
      return;
    }
    if (desc.origin.x < 0 || desc.origin.y < 0 || desc.origin.x + desc.size.x > read_buffer.size.x ||
        desc.origin.y + desc.size.y > read_buffer.size.y) {
      return;
    }

    if (desc.format == PixelFormat::kRGBA && desc.type == PixelType::kUInt8) {
      const size_t pixel_size = 4;
      const size_t row_size = static_cast<size_t>(desc.size.x) * pixel_size;
      const size_t source_row_size = static_cast<size_t>(read_buffer.size.x) * pixel_size;
      for (int row = 0; row < desc.size.y; ++row) {
        const size_t source_offset = static_cast<size_t>(desc.origin.y + row) * source_row_size +
                                     static_cast<size_t>(desc.origin.x) * pixel_size;
        const size_t destination_offset = static_cast<size_t>(row) * row_size;
        std::memcpy(static_cast<uint8_t*>(data) + destination_offset,
                    read_buffer.rgba8_pixels.data() + source_offset,
                    row_size);
      }
      return;
    }

    const Texture::Meta destination_meta = SyntheticColorMeta(desc.format, desc.type);
    if (!SupportsColorAttachmentConversion(destination_meta)) {
      return;
    }

    const Texture::Meta source_meta = SyntheticColorMeta(PixelFormat::kRGBA, PixelType::kUInt8);
    const size_t destination_pixel_size = PixelFormatChannelCount(desc.format) * PixelTypeSizeInBytes(desc.type);
    const size_t destination_row_size = static_cast<size_t>(desc.size.x) * destination_pixel_size;
    const size_t source_row_size = static_cast<size_t>(read_buffer.size.x) * 4;
    for (int row = 0; row < desc.size.y; ++row) {
      const uint8_t* source_row =
          read_buffer.rgba8_pixels.data() + static_cast<size_t>(desc.origin.y + row) * source_row_size;
      uint8_t* destination_row = static_cast<uint8_t*>(data) + static_cast<size_t>(row) * destination_row_size;
      for (int column = 0; column < desc.size.x; ++column) {
        WriteColorPixel(destination_meta,
                        destination_row,
                        column,
                        ReadColorPixel(source_meta, source_row, desc.origin.x + column));
      }
    }
  }

  void ReadDefaultDepthBuffer(const ReadPixelsDesc& desc, void* data) const {
    if (desc.format != PixelFormat::kDepthComponent || desc.type != PixelType::kFloat32 ||
        default_depth_buffer_.size.x <= 0 || default_depth_buffer_.size.y <= 0) {
      return;
    }
    if (desc.origin.x < 0 || desc.origin.y < 0 || desc.origin.x + desc.size.x > default_depth_buffer_.size.x ||
        desc.origin.y + desc.size.y > default_depth_buffer_.size.y) {
      return;
    }
    const size_t row_size = static_cast<size_t>(desc.size.x) * sizeof(float);
    const size_t source_row_size = static_cast<size_t>(default_depth_buffer_.size.x) * sizeof(float);
    for (int row = 0; row < desc.size.y; ++row) {
      const size_t source_offset =
          static_cast<size_t>(desc.origin.y + row) * source_row_size + static_cast<size_t>(desc.origin.x) * sizeof(float);
      const size_t destination_offset = static_cast<size_t>(row) * row_size;
      std::memcpy(static_cast<uint8_t*>(data) + destination_offset,
                  reinterpret_cast<const uint8_t*>(default_depth_buffer_.depth32f_pixels.data()) + source_offset,
                  row_size);
    }
  }

  void ReadDefaultStencilBuffer(const ReadPixelsDesc& desc, void* data) const {
    if (desc.format != PixelFormat::kStencilIndex || desc.type != PixelType::kUInt8 ||
        default_stencil_buffer_.size.x <= 0 || default_stencil_buffer_.size.y <= 0) {
      return;
    }
    if (desc.origin.x < 0 || desc.origin.y < 0 || desc.origin.x + desc.size.x > default_stencil_buffer_.size.x ||
        desc.origin.y + desc.size.y > default_stencil_buffer_.size.y) {
      return;
    }
    const size_t row_size = static_cast<size_t>(desc.size.x);
    const size_t source_row_size = static_cast<size_t>(default_stencil_buffer_.size.x);
    for (int row = 0; row < desc.size.y; ++row) {
      const size_t source_offset =
          static_cast<size_t>(desc.origin.y + row) * source_row_size + static_cast<size_t>(desc.origin.x);
      const size_t destination_offset = static_cast<size_t>(row) * row_size;
      std::memcpy(static_cast<uint8_t*>(data) + destination_offset,
                  default_stencil_buffer_.stencil8_pixels.data() + source_offset,
                  row_size);
    }
  }

  std::shared_ptr<VulkanContext> context_;
  std::unique_ptr<Device> scene_compatibility_device_;
  Capabilities capabilities_;
  std::unordered_map<uint32_t, TextureRecord> texture_records_;
  std::unordered_map<uint32_t, FramebufferRecord> framebuffers_;
  uint32_t next_framebuffer_id_ = 1;
  uint32_t next_texture_id_ = 1;
  FramebufferState framebuffer_state_;
  uint32_t read_framebuffer_ = 0;
  uint32_t draw_framebuffer_ = 0;
  ClearMask last_clear_mask_ = ClearMask::kNone;
  FilterMode last_blit_filter_ = FilterMode::kNearest;
  glm::ivec2 last_blit_size_ = glm::ivec2(0);
  ComputeDispatchDesc last_dispatch_desc_;
  bool depth_test_enabled_ = false;
  bool cull_enabled_ = false;
  CullMode cull_mode_ = CullMode::kBack;
  FrontFace front_face_ = FrontFace::kCounterClockwise;
  bool render_pass_active_ = false;
  uint32_t active_program_id_ = 0;
  ProgramPipelineKind active_program_kind_ = ProgramPipelineKind::kUnknown;
  DrawCallRecord last_draw_call_;
  ReadPixelsDesc last_read_pixels_request_;
  std::unordered_map<uint32_t, glm::vec4> cleared_color_attachments_;
  std::unordered_map<uint32_t, StorageTextureBinding> storage_texture_bindings_;
  uint32_t active_render_pass_framebuffer_ = 0;
  DefaultColorBuffer default_color_back_buffer_;
  DefaultColorBuffer default_color_front_buffer_;
  DefaultDepthBuffer default_depth_buffer_;
  DefaultStencilBuffer default_stencil_buffer_;
};

}  // namespace

std::unique_ptr<Device> CreateVulkanDevice(bool enable_opengl_scene_compatibility) {
  return std::make_unique<VulkanDevice>(enable_opengl_scene_compatibility);
}

void InitializeVulkanDevice(bool enable_opengl_scene_compatibility) {
  SetDevice(CreateVulkanDevice(enable_opengl_scene_compatibility));
}

}  // namespace cg::rhi

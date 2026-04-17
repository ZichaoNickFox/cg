#include "rhi/device.h"

#include <algorithm>
#include <cstring>
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

uint32_t NextSyntheticProgramId() {
  static uint32_t next_id = 1;
  return next_id++;
}

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
    capabilities.scene_backend = BackendType::kUnknown;
    capabilities.scene_api_name = "Unavailable (presentation-only Vulkan RHI)";
    capabilities.shader_language_name = "Unavailable";
    capabilities.supports_glsl_450 = false;
    capabilities.supports_storage_buffers = false;
    capabilities.supports_storage_images = false;
    capabilities.supports_compute = false;
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
      void* mapped_ptr = Map(MapAccess::kReadOnly);
      std::memcpy(mapped_ptr, data, size_in_bytes);
      Unmap();
    }
  }

  void UpdateData(size_t offset_in_bytes, size_t size_in_bytes, const void* data) override {
    CGCHECK(data != nullptr);
    CGCHECK(offset_in_bytes + size_in_bytes <= size_in_bytes_);
    void* mapped_ptr = Map(MapAccess::kReadOnly);
    std::memcpy(static_cast<std::byte*>(mapped_ptr) + offset_in_bytes, data, size_in_bytes);
    Unmap();
  }

  void Bind() override {}

  void Unbind() override {}

  void BindBase(uint32_t binding_point) override {
    binding_point_ = binding_point;
  }

  void* Map(MapAccess /*access*/) override {
    CGCHECK(buffer_memory_ != VK_NULL_HANDLE) << "Vulkan buffer was not allocated before Map().";
    if (mapped_ptr_ == nullptr) {
      CheckVkResult(vkMapMemory(context_->device, buffer_memory_, 0, size_in_bytes_, 0, &mapped_ptr_),
                    "vkMapMemory");
    }
    return mapped_ptr_;
  }

  void Unmap() override {
    if (mapped_ptr_ != nullptr) {
      vkUnmapMemory(context_->device, buffer_memory_);
      mapped_ptr_ = nullptr;
    }
  }

 private:
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
  void Bind() override {}
  void Unbind() override {}
  void EnableAttribute(uint32_t index) override {}
  void SetFloatAttribute(uint32_t index,
                         int component_count,
                         int stride_in_bytes,
                         size_t offset_in_bytes) override {}
  void SetAttributeDivisor(uint32_t index, uint32_t divisor) override {}
};

class VulkanProgram final : public Program {
 public:
  explicit VulkanProgram(std::string name)
      : name_(std::move(name)),
        synthetic_id_(NextSyntheticProgramId()) {}

  VulkanProgram(std::string name, std::shared_ptr<Program> delegate_program)
      : delegate_program_(std::move(delegate_program)),
        name_(std::move(name)),
        synthetic_id_(NextSyntheticProgramId()) {}

  uint32_t id() const override {
    if (delegate_program_ != nullptr) {
      return delegate_program_->id();
    }
    return synthetic_id_;
  }

  void Use() const override {
    if (delegate_program_ != nullptr) {
      delegate_program_->Use();
    }
  }

  void SetBool(const std::string& location_name, bool value) const override {
    if (delegate_program_ != nullptr) {
      delegate_program_->SetBool(location_name, value);
    }
  }

  void SetFloat(const std::string& location_name, float value) const override {
    if (delegate_program_ != nullptr) {
      delegate_program_->SetFloat(location_name, value);
    }
  }

  void SetInt(const std::string& location_name, int value) const override {
    if (delegate_program_ != nullptr) {
      delegate_program_->SetInt(location_name, value);
    }
  }

  int BindTexture(const std::string& location_name, const Texture& value) const override {
    if (delegate_program_ != nullptr) {
      return delegate_program_->BindTexture(location_name, value);
    }
    auto it = texture_2_unit_.find(value.id());
    if (it != texture_2_unit_.end()) {
      return it->second;
    }
    const int unit = static_cast<int>(texture_2_unit_.size());
    texture_2_unit_[value.id()] = unit;
    return unit;
  }

  void SetMat4(const std::string& location_name, const glm::mat4& value) const override {
    if (delegate_program_ != nullptr) {
      delegate_program_->SetMat4(location_name, value);
    }
  }

  void SetVec4(const std::string& location_name, const glm::vec4& value) const override {
    if (delegate_program_ != nullptr) {
      delegate_program_->SetVec4(location_name, value);
    }
  }

  void SetVec3(const std::string& location_name, const glm::vec3& value) const override {
    if (delegate_program_ != nullptr) {
      delegate_program_->SetVec3(location_name, value);
    }
  }

  void SetVec2(const std::string& location_name, const glm::vec2& value) const override {
    if (delegate_program_ != nullptr) {
      delegate_program_->SetVec2(location_name, value);
    }
  }

 private:
  std::shared_ptr<Program> delegate_program_;
  std::string name_;
  uint32_t synthetic_id_ = 0;
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

  std::shared_ptr<Program> CreateRenderProgram(const std::string& name,
                                               const std::vector<ShaderCodePart>& vs,
                                               const std::vector<ShaderCodePart>& fs,
                                               const std::vector<ShaderCodePart>& gs,
                                               const std::vector<ShaderCodePart>& ts) override {
    if (HasSceneCompatibilityDevice()) {
      return std::make_shared<VulkanProgram>(name, scene_compatibility_device_->CreateRenderProgram(name, vs, fs, gs, ts));
    }
    return std::make_shared<VulkanProgram>(name);
  }

  std::shared_ptr<Program> CreateComputeProgram(const std::string& name,
                                                const std::vector<ShaderCodePart>& cs) override {
    if (HasSceneCompatibilityDevice()) {
      return std::make_shared<VulkanProgram>(name, scene_compatibility_device_->CreateComputeProgram(name, cs));
    }
    return std::make_shared<VulkanProgram>(name);
  }

  void EnsureTextureUploaded(Texture* texture) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->EnsureTextureUploaded(texture);
      return;
    }
    std::shared_ptr<Texture::Storage> storage = texture->storage();
    CGCHECK(storage != nullptr) << "Texture storage is unavailable.";
    if (storage->uploaded_to_gl) {
      return;
    }
    if (storage->id == std::numeric_limits<uint32_t>::max()) {
      storage->id = next_texture_id_++;
    }
    storage->uploaded_to_gl = true;
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
    storage->id = std::numeric_limits<uint32_t>::max();
    storage->uploaded_to_gl = false;
  }

  void ReadTextureData(const Texture& texture, int level, void* data, size_t size_in_bytes) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->ReadTextureData(texture, level, data, size_in_bytes);
      return;
    }
    std::shared_ptr<Texture::Storage> storage = texture.storage();
    if (data == nullptr || storage == nullptr || level < 0 ||
        storage->cpu_levels.size() <= static_cast<size_t>(level)) {
      if (data != nullptr && size_in_bytes > 0) {
        std::memset(data, 0, size_in_bytes);
      }
      return;
    }
    const std::vector<uint8_t>& source = storage->cpu_levels[static_cast<size_t>(level)];
    const size_t bytes_to_copy = std::min(size_in_bytes, source.size());
    if (bytes_to_copy > 0) {
      std::memcpy(data, source.data(), bytes_to_copy);
    }
    if (size_in_bytes > bytes_to_copy) {
      std::memset(static_cast<uint8_t*>(data) + bytes_to_copy, 0, size_in_bytes - bytes_to_copy);
    }
  }

  void BindStorageTexture(uint32_t texture_unit, const Texture& texture, TextureAccess access) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->BindStorageTexture(texture_unit, texture, access);
      return;
    }
    storage_texture_bindings_[texture_unit] = StorageTextureBinding{
        .texture_id = texture.id(),
        .access = access,
    };
  }

  void DispatchCompute(const glm::uvec3& workgroup_count) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->DispatchCompute(workgroup_count);
      return;
    }
    last_dispatch_workgroup_count_ = workgroup_count;
  }

  void MemoryBarrier(rhi::MemoryBarrier barrier) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->MemoryBarrier(barrier);
      return;
    }
    last_memory_barrier_ = barrier;
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
    if (read_framebuffer_ == framebuffer) {
      read_framebuffer_ = 0;
    }
    if (draw_framebuffer_ == framebuffer) {
      draw_framebuffer_ = 0;
    }
  }

  void BindFramebuffer(FramebufferBindPoint bind_point, uint32_t framebuffer) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->BindFramebuffer(bind_point, framebuffer);
      return;
    }

    switch (bind_point) {
      case FramebufferBindPoint::kAll:
        framebuffer_state_.framebuffer = static_cast<int32_t>(framebuffer);
        read_framebuffer_ = framebuffer;
        draw_framebuffer_ = framebuffer;
        break;
      case FramebufferBindPoint::kRead:
        read_framebuffer_ = framebuffer;
        break;
      case FramebufferBindPoint::kDraw:
        draw_framebuffer_ = framebuffer;
        break;
    }
  }

  void AttachFramebufferTexture2D(AttachmentType attachment_type,
                                  uint32_t attachment_index,
                                  const Texture& texture) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->AttachFramebufferTexture2D(attachment_type, attachment_index, texture);
      return;
    }
    const auto framebuffer_it = framebuffers_.find(CurrentFramebuffer());
    if (framebuffer_it == framebuffers_.end()) {
      return;
    }
    framebuffer_it->second.attachments[AttachmentKey(attachment_type, attachment_index)] = texture.id();
  }

  bool CheckFramebufferComplete() override {
    if (HasSceneCompatibilityDevice()) {
      return scene_compatibility_device_->CheckFramebufferComplete();
    }
    return true;
  }

  FramebufferState CaptureFramebufferState() const override {
    if (HasSceneCompatibilityDevice()) {
      return scene_compatibility_device_->CaptureFramebufferState();
    }
    return framebuffer_state_;
  }

  void RestoreFramebufferState(const FramebufferState& state) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->RestoreFramebufferState(state);
      return;
    }
    framebuffer_state_ = state;
    read_framebuffer_ = static_cast<uint32_t>(std::max(state.framebuffer, 0));
    draw_framebuffer_ = static_cast<uint32_t>(std::max(state.framebuffer, 0));
  }

  void ClearColorAttachment(uint32_t attachment_index, const glm::vec4& color) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->ClearColorAttachment(attachment_index, color);
      return;
    }
    cleared_color_attachments_[attachment_index] = color;
  }

  void SetDrawBuffers(const std::vector<uint32_t>& color_attachment_indices) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->SetDrawBuffers(color_attachment_indices);
      return;
    }
    draw_buffers_ = color_attachment_indices;
  }

  void BlitFramebuffer(uint32_t read_framebuffer,
                       uint32_t draw_framebuffer,
                       const glm::ivec2& size,
                       ClearMask mask,
                       FilterMode filter) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->BlitFramebuffer(read_framebuffer, draw_framebuffer, size, mask, filter);
      return;
    }
    read_framebuffer_ = read_framebuffer;
    draw_framebuffer_ = draw_framebuffer;
    last_blit_size_ = size;
    last_clear_mask_ = mask;
    last_blit_filter_ = filter;
  }

  void DrawArrays(PrimitiveTopology topology,
                  uint32_t first,
                  uint32_t count,
                  uint32_t instance_count) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->DrawArrays(topology, first, count, instance_count);
      return;
    }
    last_draw_call_ = DrawCallRecord{
        .kind = DrawCallRecord::Kind::kArrays,
        .topology = topology,
        .first = first,
        .count = count,
        .instance_count = instance_count,
    };
  }

  void DrawElements(PrimitiveTopology topology,
                    uint32_t count,
                    uint32_t instance_count) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->DrawElements(topology, count, instance_count);
      return;
    }
    last_draw_call_ = DrawCallRecord{
        .kind = DrawCallRecord::Kind::kElements,
        .topology = topology,
        .first = 0,
        .count = count,
        .instance_count = instance_count,
    };
  }

  void SetReadBuffer(ReadBuffer buffer) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->SetReadBuffer(buffer);
      return;
    }
    read_buffer_ = buffer;
  }

  void ReadPixels(const glm::ivec2& origin,
                  const glm::ivec2& size,
                  PixelFormat format,
                  PixelType type,
                  void* data) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->ReadPixels(origin, size, format, type, data);
      return;
    }
    last_read_pixels_request_ = ReadPixelsRequest{
        .origin = origin,
        .size = size,
        .format = format,
        .type = type,
    };
    if (data != nullptr) {
      std::memset(data, 0, PixelPayloadSizeInBytes(size, format, type));
    }
  }

  void SetViewport(const glm::ivec2& origin, const glm::ivec2& size) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->SetViewport(origin, size);
      return;
    }
    framebuffer_state_.viewport = glm::ivec4(origin.x, origin.y, size.x, size.y);
  }

  void SetClearColor(const glm::vec4& color) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->SetClearColor(color);
      return;
    }
    clear_color_ = color;
  }

  void Clear(ClearMask mask) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->Clear(mask);
      return;
    }
    last_clear_mask_ = mask;
  }

  void SetDepthTestEnabled(bool enabled) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->SetDepthTestEnabled(enabled);
      return;
    }
    depth_test_enabled_ = enabled;
  }

  void SetCullEnabled(bool enabled) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->SetCullEnabled(enabled);
      return;
    }
    cull_enabled_ = enabled;
  }

  void SetCullMode(CullMode mode) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->SetCullMode(mode);
      return;
    }
    cull_mode_ = mode;
  }

  void SetFrontFace(FrontFace winding) override {
    if (HasSceneCompatibilityDevice()) {
      scene_compatibility_device_->SetFrontFace(winding);
      return;
    }
    front_face_ = winding;
  }

 private:
  struct FramebufferRecord {
    std::unordered_map<uint64_t, uint32_t> attachments;
  };

  struct StorageTextureBinding {
    uint32_t texture_id = 0;
    TextureAccess access = TextureAccess::kReadOnly;
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

  struct ReadPixelsRequest {
    glm::ivec2 origin = glm::ivec2(0);
    glm::ivec2 size = glm::ivec2(0);
    PixelFormat format = PixelFormat::kUnknown;
    PixelType type = PixelType::kUnknown;
  };

  bool HasSceneCompatibilityDevice() const {
    return scene_compatibility_device_ != nullptr;
  }

  uint32_t CurrentFramebuffer() const {
    return framebuffer_state_.framebuffer <= 0 ? 0u : static_cast<uint32_t>(framebuffer_state_.framebuffer);
  }

  static uint64_t AttachmentKey(AttachmentType attachment_type, uint32_t attachment_index) {
    return (static_cast<uint64_t>(attachment_index) << 32) | static_cast<uint32_t>(attachment_type);
  }

  std::shared_ptr<VulkanContext> context_;
  std::unique_ptr<Device> scene_compatibility_device_;
  Capabilities capabilities_;
  std::unordered_map<uint32_t, FramebufferRecord> framebuffers_;
  uint32_t next_framebuffer_id_ = 1;
  uint32_t next_texture_id_ = 1;
  FramebufferState framebuffer_state_;
  uint32_t read_framebuffer_ = 0;
  uint32_t draw_framebuffer_ = 0;
  ReadBuffer read_buffer_ = ReadBuffer::kBack;
  glm::vec4 clear_color_ = glm::vec4(0.0f);
  ClearMask last_clear_mask_ = ClearMask::kNone;
  rhi::MemoryBarrier last_memory_barrier_ = rhi::MemoryBarrier::kAll;
  FilterMode last_blit_filter_ = FilterMode::kNearest;
  glm::ivec2 last_blit_size_ = glm::ivec2(0);
  glm::uvec3 last_dispatch_workgroup_count_ = glm::uvec3(0);
  bool depth_test_enabled_ = false;
  bool cull_enabled_ = false;
  CullMode cull_mode_ = CullMode::kBack;
  FrontFace front_face_ = FrontFace::kCounterClockwise;
  std::vector<uint32_t> draw_buffers_;
  DrawCallRecord last_draw_call_;
  ReadPixelsRequest last_read_pixels_request_;
  std::unordered_map<uint32_t, glm::vec4> cleared_color_attachments_;
  std::unordered_map<uint32_t, StorageTextureBinding> storage_texture_bindings_;
};

}  // namespace

std::unique_ptr<Device> CreateVulkanDevice(bool enable_opengl_scene_compatibility) {
  return std::make_unique<VulkanDevice>(enable_opengl_scene_compatibility);
}

void InitializeVulkanDevice(bool enable_opengl_scene_compatibility) {
  SetDevice(CreateVulkanDevice(enable_opengl_scene_compatibility));
}

}  // namespace cg::rhi

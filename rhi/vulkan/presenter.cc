#include "rhi/vulkan/presenter.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "backends/imgui_impl_vulkan.h"

#include "base/debug.h"

namespace cg::rhi::vulkan {
namespace {

constexpr const char* kVertexShaderPath = "rhi/vulkan/shader/texture_present.vert.spv";
constexpr const char* kFragmentShaderPath = "rhi/vulkan/shader/texture_present.frag.spv";

void CheckVkResult(VkResult result, const char* operation_name) {
  CGCHECK(result == VK_SUCCESS) << operation_name << " failed with VkResult=" << static_cast<int>(result);
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

std::vector<std::uint32_t> LoadBinaryFile(const char* path) {
  std::ifstream stream(path, std::ios::binary | std::ios::ate);
  CGCHECK(stream.is_open()) << "Failed to open Vulkan shader: " << path;
  const std::streamsize size_in_bytes = stream.tellg();
  CGCHECK(size_in_bytes > 0);
  CGCHECK(size_in_bytes % 4 == 0) << "SPIR-V size must be a multiple of 4 bytes: " << path;
  stream.seekg(0);

  std::vector<std::uint32_t> bytes(static_cast<std::size_t>(size_in_bytes) / sizeof(std::uint32_t));
  stream.read(reinterpret_cast<char*>(bytes.data()), size_in_bytes);
  CGCHECK(stream.good()) << "Failed to read Vulkan shader: " << path;
  return bytes;
}

VkShaderModule CreateShaderModule(VkDevice device, const char* path) {
  const std::vector<std::uint32_t> code = LoadBinaryFile(path);
  VkShaderModuleCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = code.size() * sizeof(std::uint32_t);
  create_info.pCode = code.data();

  VkShaderModule shader_module = VK_NULL_HANDLE;
  CheckVkResult(vkCreateShaderModule(device, &create_info, nullptr, &shader_module), "vkCreateShaderModule");
  return shader_module;
}

uint32_t FindMemoryType(VkPhysicalDevice physical_device,
                        uint32_t type_bits,
                        VkMemoryPropertyFlags required_flags) {
  VkPhysicalDeviceMemoryProperties memory_properties{};
  vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);
  for (uint32_t index = 0; index < memory_properties.memoryTypeCount; ++index) {
    const bool type_matches = (type_bits & (1u << index)) != 0;
    const bool flags_match =
        (memory_properties.memoryTypes[index].propertyFlags & required_flags) == required_flags;
    if (type_matches && flags_match) {
      return index;
    }
  }
  CGCHECK(false) << "Failed to find matching Vulkan memory type.";
  return 0;
}

VkSurfaceFormatKHR SelectSurfaceFormat(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
  constexpr VkFormat kPreferredFormats[] = {
      VK_FORMAT_B8G8R8A8_UNORM,
      VK_FORMAT_R8G8B8A8_UNORM,
  };
  return ImGui_ImplVulkanH_SelectSurfaceFormat(
      physical_device,
      surface,
      kPreferredFormats,
      static_cast<int>(std::size(kPreferredFormats)),
      VK_COLORSPACE_SRGB_NONLINEAR_KHR);
}

VkPresentModeKHR SelectPresentMode(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
  constexpr VkPresentModeKHR kPresentModes[] = {
      VK_PRESENT_MODE_FIFO_KHR,
  };
  return ImGui_ImplVulkanH_SelectPresentMode(
      physical_device, surface, kPresentModes, static_cast<int>(std::size(kPresentModes)));
}

struct QueueSelection {
  VkPhysicalDevice physical_device = VK_NULL_HANDLE;
  uint32_t queue_family_index = std::numeric_limits<uint32_t>::max();
};

QueueSelection ChoosePhysicalDeviceAndQueue(VkInstance instance, VkSurfaceKHR surface) {
  uint32_t physical_device_count = 0;
  CheckVkResult(vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr),
                "vkEnumeratePhysicalDevices(count)");
  CGCHECK(physical_device_count > 0) << "No Vulkan physical device was found.";

  std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
  CheckVkResult(vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data()),
                "vkEnumeratePhysicalDevices(data)");

  for (VkPhysicalDevice physical_device : physical_devices) {
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

    for (uint32_t queue_family_index = 0; queue_family_index < queue_family_count; ++queue_family_index) {
      if ((queue_families[queue_family_index].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) {
        continue;
      }
      VkBool32 supports_present = VK_FALSE;
      CheckVkResult(vkGetPhysicalDeviceSurfaceSupportKHR(
                        physical_device, queue_family_index, surface, &supports_present),
                    "vkGetPhysicalDeviceSurfaceSupportKHR");
      if (supports_present == VK_TRUE) {
        return {physical_device, queue_family_index};
      }
    }
  }

  CGCHECK(false) << "Failed to find a Vulkan graphics queue that can present to the GLFW surface.";
  return {};
}

void CreateBuffer(VkDevice device,
                  VkPhysicalDevice physical_device,
                  VkDeviceSize size,
                  VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags memory_properties,
                  VkBuffer* buffer,
                  VkDeviceMemory* buffer_memory) {
  VkBufferCreateInfo buffer_create_info{};
  buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_create_info.size = size;
  buffer_create_info.usage = usage;
  buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  CheckVkResult(vkCreateBuffer(device, &buffer_create_info, nullptr, buffer), "vkCreateBuffer");

  VkMemoryRequirements requirements{};
  vkGetBufferMemoryRequirements(device, *buffer, &requirements);

  VkMemoryAllocateInfo allocate_info{};
  allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocate_info.allocationSize = requirements.size;
  allocate_info.memoryTypeIndex =
      FindMemoryType(physical_device, requirements.memoryTypeBits, memory_properties);
  CheckVkResult(vkAllocateMemory(device, &allocate_info, nullptr, buffer_memory), "vkAllocateMemory(buffer)");
  CheckVkResult(vkBindBufferMemory(device, *buffer, *buffer_memory, 0), "vkBindBufferMemory");
}

void TransitionImageLayout(VkCommandBuffer command_buffer,
                           VkImage image,
                           VkImageLayout old_layout,
                           VkImageLayout new_layout,
                           VkAccessFlags src_access_mask,
                           VkAccessFlags dst_access_mask,
                           VkPipelineStageFlags src_stage,
                           VkPipelineStageFlags dst_stage) {
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = old_layout;
  barrier.newLayout = new_layout;
  barrier.srcAccessMask = src_access_mask;
  barrier.dstAccessMask = dst_access_mask;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  vkCmdPipelineBarrier(command_buffer,
                       src_stage,
                       dst_stage,
                       0,
                       0,
                       nullptr,
                       0,
                       nullptr,
                       1,
                       &barrier);
}

}  // namespace

class Presenter::Impl {
 public:
  Impl(int width, int height, const char* title) {
    CGCHECK(glfwVulkanSupported()) << "GLFW reports that Vulkan is not supported on this machine.";

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
    CGCHECK(window_ != nullptr) << "Failed to create GLFW Vulkan window.";

    CreateInstance();
    CreateSurface();
    SelectDevice();
    CreateDevice();
    CreateDescriptorPool();
    CreateSwapchainWindow(width, height);
    CreateDescriptorSetLayout();
    CreatePipelineLayout();
    AllocateDescriptorSet();
    RecreateUploadResources(width, height);
    CreatePipeline();
  }

  ~Impl() {
    if (device_ != VK_NULL_HANDLE) {
      vkDeviceWaitIdle(device_);
    }
    DestroyPipeline();
    DestroyUploadResources();
    if (descriptor_set_layout_ != VK_NULL_HANDLE) {
      vkDestroyDescriptorSetLayout(device_, descriptor_set_layout_, nullptr);
    }
    if (pipeline_layout_ != VK_NULL_HANDLE) {
      vkDestroyPipelineLayout(device_, pipeline_layout_, nullptr);
    }
    if (descriptor_pool_ != VK_NULL_HANDLE) {
      vkDestroyDescriptorPool(device_, descriptor_pool_, nullptr);
    }
    if (window_data_.Surface != VK_NULL_HANDLE) {
      ImGui_ImplVulkanH_DestroyWindow(instance_, device_, &window_data_, nullptr);
    }
    if (device_ != VK_NULL_HANDLE) {
      vkDestroyDevice(device_, nullptr);
    }
    if (instance_ != VK_NULL_HANDLE) {
      vkDestroyInstance(instance_, nullptr);
    }
    if (window_ != nullptr) {
      glfwDestroyWindow(window_);
    }
  }

  void Present(const PresenterFrame& frame) {
    CGCHECK(frame.width > 0 && frame.height > 0);
    CGCHECK(frame.rgba_pixels != nullptr);
    CGCHECK(frame.size_in_bytes >= static_cast<std::size_t>(frame.width) * static_cast<std::size_t>(frame.height) * 4);

    if (frame.width != upload_width_ || frame.height != upload_height_) {
      vkDeviceWaitIdle(device_);
      RecreateUploadResources(frame.width, frame.height);
    }

    const VkResult acquire_result = vkAcquireNextImageKHR(device_,
                                                          window_data_.Swapchain,
                                                          UINT64_MAX,
                                                          window_data_.FrameSemaphores[window_data_.SemaphoreIndex]
                                                              .ImageAcquiredSemaphore,
                                                          VK_NULL_HANDLE,
                                                          &window_data_.FrameIndex);
    CGCHECK(acquire_result == VK_SUCCESS || acquire_result == VK_SUBOPTIMAL_KHR || acquire_result == VK_ERROR_OUT_OF_DATE_KHR)
        << "vkAcquireNextImageKHR failed with " << static_cast<int>(acquire_result);
    if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR || acquire_result == VK_SUBOPTIMAL_KHR) {
      RecreateSwapchain();
      return;
    }

    ImGui_ImplVulkanH_Frame* frame_data = &window_data_.Frames[window_data_.FrameIndex];
    CheckVkResult(vkWaitForFences(device_, 1, &frame_data->Fence, VK_TRUE, UINT64_MAX), "vkWaitForFences");
    CheckVkResult(vkResetFences(device_, 1, &frame_data->Fence), "vkResetFences");
    CheckVkResult(vkResetCommandPool(device_, frame_data->CommandPool, 0), "vkResetCommandPool");

    void* mapped_ptr = nullptr;
    CheckVkResult(vkMapMemory(device_, staging_buffer_memory_, 0, staging_buffer_size_, 0, &mapped_ptr),
                  "vkMapMemory(staging)");
    std::memcpy(mapped_ptr, frame.rgba_pixels, frame.size_in_bytes);
    vkUnmapMemory(device_, staging_buffer_memory_);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    CheckVkResult(vkBeginCommandBuffer(frame_data->CommandBuffer, &begin_info), "vkBeginCommandBuffer");

    TransitionImageLayout(frame_data->CommandBuffer,
                          upload_image_,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_ACCESS_SHADER_READ_BIT,
                          VK_ACCESS_TRANSFER_WRITE_BIT,
                          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                          VK_PIPELINE_STAGE_TRANSFER_BIT);

    VkBufferImageCopy copy_region{};
    copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.imageSubresource.mipLevel = 0;
    copy_region.imageSubresource.baseArrayLayer = 0;
    copy_region.imageSubresource.layerCount = 1;
    copy_region.imageExtent.width = static_cast<uint32_t>(frame.width);
    copy_region.imageExtent.height = static_cast<uint32_t>(frame.height);
    copy_region.imageExtent.depth = 1;
    vkCmdCopyBufferToImage(frame_data->CommandBuffer,
                           staging_buffer_,
                           upload_image_,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &copy_region);

    TransitionImageLayout(frame_data->CommandBuffer,
                          upload_image_,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                          VK_ACCESS_TRANSFER_WRITE_BIT,
                          VK_ACCESS_SHADER_READ_BIT,
                          VK_PIPELINE_STAGE_TRANSFER_BIT,
                          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    VkRenderPassBeginInfo render_pass_begin_info{};
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.renderPass = window_data_.RenderPass;
    render_pass_begin_info.framebuffer = frame_data->Framebuffer;
    render_pass_begin_info.renderArea.extent.width = static_cast<uint32_t>(window_data_.Width);
    render_pass_begin_info.renderArea.extent.height = static_cast<uint32_t>(window_data_.Height);
    render_pass_begin_info.clearValueCount = 1;
    render_pass_begin_info.pClearValues = &window_data_.ClearValue;
    vkCmdBeginRenderPass(frame_data->CommandBuffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.width = static_cast<float>(window_data_.Width);
    viewport.height = static_cast<float>(window_data_.Height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(frame_data->CommandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.extent.width = static_cast<uint32_t>(window_data_.Width);
    scissor.extent.height = static_cast<uint32_t>(window_data_.Height);
    vkCmdSetScissor(frame_data->CommandBuffer, 0, 1, &scissor);

    vkCmdBindPipeline(frame_data->CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);
    vkCmdBindDescriptorSets(frame_data->CommandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline_layout_,
                            0,
                            1,
                            &descriptor_set_,
                            0,
                            nullptr);
    vkCmdDraw(frame_data->CommandBuffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(frame_data->CommandBuffer);

    CheckVkResult(vkEndCommandBuffer(frame_data->CommandBuffer), "vkEndCommandBuffer");

    const VkSemaphore image_acquired_semaphore =
        window_data_.FrameSemaphores[window_data_.SemaphoreIndex].ImageAcquiredSemaphore;
    const VkSemaphore render_complete_semaphore =
        window_data_.FrameSemaphores[window_data_.SemaphoreIndex].RenderCompleteSemaphore;
    const VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &image_acquired_semaphore;
    submit_info.pWaitDstStageMask = &wait_stage;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &frame_data->CommandBuffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &render_complete_semaphore;
    CheckVkResult(vkQueueSubmit(queue_, 1, &submit_info, frame_data->Fence), "vkQueueSubmit");

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &render_complete_semaphore;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &window_data_.Swapchain;
    present_info.pImageIndices = &window_data_.FrameIndex;
    const VkResult present_result = vkQueuePresentKHR(queue_, &present_info);
    CGCHECK(present_result == VK_SUCCESS || present_result == VK_SUBOPTIMAL_KHR || present_result == VK_ERROR_OUT_OF_DATE_KHR)
        << "vkQueuePresentKHR failed with " << static_cast<int>(present_result);
    if (present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR) {
      RecreateSwapchain();
    }

    window_data_.SemaphoreIndex = (window_data_.SemaphoreIndex + 1) % window_data_.ImageCount;
  }

  GLFWwindow* window() const {
    return window_;
  }

 private:
  void CreateInstance() {
    uint32_t required_extension_count = 0;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&required_extension_count);
    CGCHECK(glfw_extensions != nullptr);

    std::vector<const char*> instance_extensions(glfw_extensions, glfw_extensions + required_extension_count);
    const std::vector<VkExtensionProperties> available_extensions = EnumerateInstanceExtensions();
#if defined(CG_PLATFORM_MACOS)
    if (HasExtension(available_extensions, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
      instance_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
      enable_portability_enumeration_ = true;
    }
#endif

    VkApplicationInfo application_info{};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName = "cg";
    application_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    application_info.pEngineName = "cg";
    application_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    application_info.apiVersion = VK_API_VERSION_1_1;

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &application_info;
    create_info.enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size());
    create_info.ppEnabledExtensionNames = instance_extensions.data();
#if defined(CG_PLATFORM_MACOS)
    if (enable_portability_enumeration_) {
      create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }
#endif

    CheckVkResult(vkCreateInstance(&create_info, nullptr, &instance_), "vkCreateInstance");
  }

  void CreateSurface() {
    CheckVkResult(glfwCreateWindowSurface(instance_, window_, nullptr, &window_data_.Surface),
                  "glfwCreateWindowSurface");
  }

  void SelectDevice() {
    const QueueSelection selection = ChoosePhysicalDeviceAndQueue(instance_, window_data_.Surface);
    physical_device_ = selection.physical_device;
    queue_family_index_ = selection.queue_family_index;
  }

  void CreateDevice() {
    const std::vector<VkExtensionProperties> device_extensions = EnumerateDeviceExtensions(physical_device_);
    std::vector<const char*> enabled_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
#if defined(CG_PLATFORM_MACOS)
#if defined(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)
    if (HasExtension(device_extensions, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
      enabled_extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
    }
#endif
#endif

    const float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = queue_family_index_;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;

    VkPhysicalDeviceFeatures enabled_features{};
    VkDeviceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = 1;
    create_info.pQueueCreateInfos = &queue_create_info;
    create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());
    create_info.ppEnabledExtensionNames = enabled_extensions.data();
    create_info.pEnabledFeatures = &enabled_features;

    CheckVkResult(vkCreateDevice(physical_device_, &create_info, nullptr, &device_), "vkCreateDevice");
    vkGetDeviceQueue(device_, queue_family_index_, 0, &queue_);
  }

  void CreateDescriptorPool() {
    const VkDescriptorPoolSize pool_size = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1};
    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = &pool_size;
    pool_info.maxSets = 1;
    CheckVkResult(vkCreateDescriptorPool(device_, &pool_info, nullptr, &descriptor_pool_),
                  "vkCreateDescriptorPool");
  }

  void CreateSwapchainWindow(int width, int height) {
    window_data_.SurfaceFormat = SelectSurfaceFormat(physical_device_, window_data_.Surface);
    window_data_.PresentMode = SelectPresentMode(physical_device_, window_data_.Surface);
    window_data_.ClearValue.color.float32[0] = 0.05f;
    window_data_.ClearValue.color.float32[1] = 0.07f;
    window_data_.ClearValue.color.float32[2] = 0.09f;
    window_data_.ClearValue.color.float32[3] = 1.0f;
    min_image_count_ = 2;
    ImGui_ImplVulkanH_CreateOrResizeWindow(
        instance_, physical_device_, device_, &window_data_, queue_family_index_, nullptr, width, height, min_image_count_);
  }

  void RecreateSwapchain() {
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window_, &width, &height);
    if (width <= 0 || height <= 0) {
      return;
    }
    vkDeviceWaitIdle(device_);
    ImGui_ImplVulkanH_CreateOrResizeWindow(
        instance_, physical_device_, device_, &window_data_, queue_family_index_, nullptr, width, height, min_image_count_);
    DestroyPipeline();
    CreatePipeline();
  }

  void CreateDescriptorSetLayout() {
    const VkDescriptorSetLayoutBinding binding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = nullptr,
    };

    VkDescriptorSetLayoutCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    create_info.bindingCount = 1;
    create_info.pBindings = &binding;
    CheckVkResult(vkCreateDescriptorSetLayout(device_, &create_info, nullptr, &descriptor_set_layout_),
                  "vkCreateDescriptorSetLayout");
  }

  void CreatePipelineLayout() {
    VkPipelineLayoutCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    create_info.setLayoutCount = 1;
    create_info.pSetLayouts = &descriptor_set_layout_;
    CheckVkResult(vkCreatePipelineLayout(device_, &create_info, nullptr, &pipeline_layout_),
                  "vkCreatePipelineLayout");
  }

  void AllocateDescriptorSet() {
    VkDescriptorSetAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.descriptorPool = descriptor_pool_;
    allocate_info.descriptorSetCount = 1;
    allocate_info.pSetLayouts = &descriptor_set_layout_;
    CheckVkResult(vkAllocateDescriptorSets(device_, &allocate_info, &descriptor_set_), "vkAllocateDescriptorSets");
  }

  void DestroyUploadResources() {
    if (device_ == VK_NULL_HANDLE) {
      return;
    }
    if (staging_buffer_ != VK_NULL_HANDLE) {
      vkDestroyBuffer(device_, staging_buffer_, nullptr);
      staging_buffer_ = VK_NULL_HANDLE;
    }
    if (staging_buffer_memory_ != VK_NULL_HANDLE) {
      vkFreeMemory(device_, staging_buffer_memory_, nullptr);
      staging_buffer_memory_ = VK_NULL_HANDLE;
    }
    if (upload_image_view_ != VK_NULL_HANDLE) {
      vkDestroyImageView(device_, upload_image_view_, nullptr);
      upload_image_view_ = VK_NULL_HANDLE;
    }
    if (upload_sampler_ != VK_NULL_HANDLE) {
      vkDestroySampler(device_, upload_sampler_, nullptr);
      upload_sampler_ = VK_NULL_HANDLE;
    }
    if (upload_image_ != VK_NULL_HANDLE) {
      vkDestroyImage(device_, upload_image_, nullptr);
      upload_image_ = VK_NULL_HANDLE;
    }
    if (upload_image_memory_ != VK_NULL_HANDLE) {
      vkFreeMemory(device_, upload_image_memory_, nullptr);
      upload_image_memory_ = VK_NULL_HANDLE;
    }
    staging_buffer_size_ = 0;
    upload_width_ = 0;
    upload_height_ = 0;
  }

  void RecreateUploadResources(int width, int height) {
    DestroyUploadResources();

    upload_width_ = width;
    upload_height_ = height;
    staging_buffer_size_ = static_cast<VkDeviceSize>(width) * static_cast<VkDeviceSize>(height) * 4;

    CreateBuffer(device_,
                 physical_device_,
                 staging_buffer_size_,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 &staging_buffer_,
                 &staging_buffer_memory_);

    VkImageCreateInfo image_create_info{};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.width = static_cast<uint32_t>(width);
    image_create_info.extent.height = static_cast<uint32_t>(height);
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    CheckVkResult(vkCreateImage(device_, &image_create_info, nullptr, &upload_image_), "vkCreateImage");

    VkMemoryRequirements image_requirements{};
    vkGetImageMemoryRequirements(device_, upload_image_, &image_requirements);
    VkMemoryAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = image_requirements.size;
    allocate_info.memoryTypeIndex =
        FindMemoryType(physical_device_, image_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    CheckVkResult(vkAllocateMemory(device_, &allocate_info, nullptr, &upload_image_memory_),
                  "vkAllocateMemory(image)");
    CheckVkResult(vkBindImageMemory(device_, upload_image_, upload_image_memory_, 0), "vkBindImageMemory");

    VkImageViewCreateInfo view_create_info{};
    view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_create_info.image = upload_image_;
    view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_create_info.subresourceRange.baseMipLevel = 0;
    view_create_info.subresourceRange.levelCount = 1;
    view_create_info.subresourceRange.baseArrayLayer = 0;
    view_create_info.subresourceRange.layerCount = 1;
    CheckVkResult(vkCreateImageView(device_, &view_create_info, nullptr, &upload_image_view_),
                  "vkCreateImageView");

    VkSamplerCreateInfo sampler_create_info{};
    sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_create_info.magFilter = VK_FILTER_LINEAR;
    sampler_create_info.minFilter = VK_FILTER_LINEAR;
    sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_create_info.minLod = 0.0f;
    sampler_create_info.maxLod = 1.0f;
    CheckVkResult(vkCreateSampler(device_, &sampler_create_info, nullptr, &upload_sampler_),
                  "vkCreateSampler");

    VkDescriptorImageInfo image_info{};
    image_info.sampler = upload_sampler_;
    image_info.imageView = upload_image_view_;
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = descriptor_set_;
    write.dstBinding = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = &image_info;
    vkUpdateDescriptorSets(device_, 1, &write, 0, nullptr);

    ImGui_ImplVulkanH_Frame* frame_data = &window_data_.Frames[window_data_.FrameIndex];
    CheckVkResult(vkResetCommandPool(device_, frame_data->CommandPool, 0), "vkResetCommandPool(init)");
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    CheckVkResult(vkBeginCommandBuffer(frame_data->CommandBuffer, &begin_info), "vkBeginCommandBuffer(init)");
    TransitionImageLayout(frame_data->CommandBuffer,
                          upload_image_,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                          0,
                          VK_ACCESS_SHADER_READ_BIT,
                          VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    CheckVkResult(vkEndCommandBuffer(frame_data->CommandBuffer), "vkEndCommandBuffer(init)");

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &frame_data->CommandBuffer;
    CheckVkResult(vkQueueSubmit(queue_, 1, &submit_info, VK_NULL_HANDLE), "vkQueueSubmit(init)");
    CheckVkResult(vkQueueWaitIdle(queue_), "vkQueueWaitIdle(init)");
  }

  void DestroyPipeline() {
    if (device_ == VK_NULL_HANDLE) {
      return;
    }
    if (pipeline_ != VK_NULL_HANDLE) {
      vkDestroyPipeline(device_, pipeline_, nullptr);
      pipeline_ = VK_NULL_HANDLE;
    }
  }

  void CreatePipeline() {
    DestroyPipeline();

    const VkShaderModule vert_shader = CreateShaderModule(device_, kVertexShaderPath);
    const VkShaderModule frag_shader = CreateShaderModule(device_, kFragmentShaderPath);

    const VkPipelineShaderStageCreateInfo shader_stages[] = {
        {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .stage = VK_SHADER_STAGE_VERTEX_BIT,
         .module = vert_shader,
         .pName = "main"},
        {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
         .module = frag_shader,
         .pName = "main"},
    };

    VkPipelineVertexInputStateCreateInfo vertex_input_state{};
    vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state{};
    input_assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterization_state{};
    rasterization_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_state.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization_state.cullMode = VK_CULL_MODE_NONE;
    rasterization_state.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterization_state.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisample_state{};
    multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState blend_attachment{};
    blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    blend_attachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo color_blend_state{};
    color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state.attachmentCount = 1;
    color_blend_state.pAttachments = &blend_attachment;

    const VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = static_cast<uint32_t>(std::size(dynamic_states));
    dynamic_state.pDynamicStates = dynamic_states;

    VkGraphicsPipelineCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    create_info.stageCount = static_cast<uint32_t>(std::size(shader_stages));
    create_info.pStages = shader_stages;
    create_info.pVertexInputState = &vertex_input_state;
    create_info.pInputAssemblyState = &input_assembly_state;
    create_info.pViewportState = &viewport_state;
    create_info.pRasterizationState = &rasterization_state;
    create_info.pMultisampleState = &multisample_state;
    create_info.pColorBlendState = &color_blend_state;
    create_info.pDynamicState = &dynamic_state;
    create_info.layout = pipeline_layout_;
    create_info.renderPass = window_data_.RenderPass;
    create_info.subpass = 0;

    CheckVkResult(vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline_),
                  "vkCreateGraphicsPipelines");

    vkDestroyShaderModule(device_, frag_shader, nullptr);
    vkDestroyShaderModule(device_, vert_shader, nullptr);
  }

  GLFWwindow* window_ = nullptr;
  bool enable_portability_enumeration_ = false;

  VkInstance instance_ = VK_NULL_HANDLE;
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
  VkDevice device_ = VK_NULL_HANDLE;
  uint32_t queue_family_index_ = std::numeric_limits<uint32_t>::max();
  VkQueue queue_ = VK_NULL_HANDLE;
  VkDescriptorPool descriptor_pool_ = VK_NULL_HANDLE;
  ImGui_ImplVulkanH_Window window_data_;
  uint32_t min_image_count_ = 2;

  VkDescriptorSetLayout descriptor_set_layout_ = VK_NULL_HANDLE;
  VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;
  VkDescriptorSet descriptor_set_ = VK_NULL_HANDLE;
  VkPipeline pipeline_ = VK_NULL_HANDLE;

  VkImage upload_image_ = VK_NULL_HANDLE;
  VkDeviceMemory upload_image_memory_ = VK_NULL_HANDLE;
  VkImageView upload_image_view_ = VK_NULL_HANDLE;
  VkSampler upload_sampler_ = VK_NULL_HANDLE;
  VkBuffer staging_buffer_ = VK_NULL_HANDLE;
  VkDeviceMemory staging_buffer_memory_ = VK_NULL_HANDLE;
  VkDeviceSize staging_buffer_size_ = 0;
  int upload_width_ = 0;
  int upload_height_ = 0;
};

Presenter::Presenter(int width, int height, const char* title)
    : impl_(std::make_unique<Impl>(width, height, title)) {}

Presenter::~Presenter() = default;

GLFWwindow* Presenter::window() const {
  return impl_->window();
}

bool Presenter::ShouldClose() const {
  return glfwWindowShouldClose(window()) != 0;
}

void Presenter::Present(const PresenterFrame& frame) {
  impl_->Present(frame);
}

const char* Presenter::runtime_name() const {
  return "Vulkan";
}

bool IsSupported() {
  return glfwVulkanSupported() == GLFW_TRUE;
}

}  // namespace cg::rhi::vulkan

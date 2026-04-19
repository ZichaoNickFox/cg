#include "playground/playground.h"

#include <cstdlib>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include "imgui.h"

#include "base/debug.h"
#include "base/util.h"
#include "playground/geometry_scene/fitting_scene.h"
#include "playground/geometry_scene/implot_demo_scene.h"
#include "playground/physics_scene/angry_bunny_scene.h"
#include "playground/renderer_scene/geometry_scene.h"
#include "playground/renderer_scene/model_scene.h"
#include "playground/renderer_scene/path_tracing_geometry_scene.h"
#include "playground/renderer_scene/path_tracing_scene.h"
#include "playground/renderer_scene/ray_tracing_scene.h"
#include "playground/renderer_scene/rtrt_scene.h"
#include "playground/renderer_scene/sample_scene.h"
#include "playground/renderer_scene/shadow_scene.h"
#include "playground/renderer_scene/ssao_scene.h"
#include "rhi/device.h"

namespace {

enum class SceneCategory {
  kRendering = 0,
  kPhysics = 1,
  kAnimation = 2,
  kGeometry = 3,
};

struct SceneDescriptor {
  std::string id;
  std::string display_name;
  std::string description;
  SceneCategory category = SceneCategory::kRendering;
  bool requires_glsl_450 = false;
  bool requires_compute = false;
  bool requires_storage_buffers = false;
  std::function<std::unique_ptr<cg::Scene>()> create;
};

enum class SceneBackendOption {
  kOpenGL = 0,
  kVulkan = 1,
};

struct SceneBackendAvailability {
  bool available = false;
  std::string reason;
};

const std::string kConfigPath = util::FileJoin(CG_PROJECT_SOURCE_DIR, "playground/config.pb.txt");
const std::string kDefaultSceneId = "FittingScene";
const SceneCategory kSceneCategories[] = {
    SceneCategory::kRendering,
    SceneCategory::kPhysics,
    SceneCategory::kAnimation,
    SceneCategory::kGeometry,
};

const std::vector<SceneDescriptor>& GetSceneCatalog() {
  static const std::vector<SceneDescriptor> kScenes = {
      {"FittingScene", "Fitting Scene", "Curve fitting and interpolation playground",
       SceneCategory::kGeometry, false, false, false, [] { return std::make_unique<FittingScene>(); }},
      {"ImPlotDemoScene", "ImPlot Demo", "ImPlot reference demo window",
       SceneCategory::kGeometry, false, false, false, [] { return std::make_unique<ImPlotDemoScene>(); }},
      {"ModelScene", "Model Scene", "Phong shading and material toggles on a loaded model",
       SceneCategory::kRendering, true, false, true, [] { return std::make_unique<ModelScene>(); }},
      {"ShadowScene", "Shadow Scene", "Shadow map, PCF, and PCSS comparison",
       SceneCategory::kRendering, true, false, true, [] { return std::make_unique<ShadowScene>(); }},
      {"SampleScene", "Sample Scene", "Raster sample scene backed by storage buffers",
       SceneCategory::kRendering, true, false, true, [] { return std::make_unique<SampleScene>(); }},
      {"GeometryScene", "Geometry Scene", "Compute-based ray queries against triangles and AABBs",
       SceneCategory::kGeometry, true, true, true, [] { return std::make_unique<GeometryScene>(); }},
      {"RayTracingScene", "Ray Tracing Scene", "Compute ray tracing against analytic spheres",
       SceneCategory::kRendering, true, true, true, [] { return std::make_unique<RayTracingScene>(); }},
      {"PathTracingGeometryScene", "Path Tracing Geometry", "Compute path tracing against analytic spheres",
       SceneCategory::kGeometry, true, true, false, [] { return std::make_unique<PathTracingGeometryScene>(); }},
      {"PathTracingScene", "Path Tracing Scene", "Compute path tracing through BVH-backed scene geometry",
       SceneCategory::kRendering, true, true, true, [] { return std::make_unique<PathTracingScene>(); }},
      {"RTRTScene", "RTRT Scene", "Raster + compute hybrid real-time path tracing prototype",
       SceneCategory::kRendering, true, true, true, [] { return std::make_unique<RTRTScene>(); }},
      {"SSAOScene", "SSAO Scene", "Screen-space ambient occlusion with compute post-process",
       SceneCategory::kRendering, true, true, false, [] { return std::make_unique<SSAOScene>(); }},
      {"AngryBunnyScene", "Angry Bunny", "Rigid body / physics playground",
       SceneCategory::kPhysics, true, false, true, [] { return std::make_unique<AngryBunnyScene>(); }},
  };
  return kScenes;
}

const SceneDescriptor* FindSceneDescriptor(const std::string& scene_id) {
  for (const SceneDescriptor& descriptor : GetSceneCatalog()) {
    if (descriptor.id == scene_id) {
      return &descriptor;
    }
  }
  return nullptr;
}

const char* SceneCategoryLabel(SceneCategory category) {
  switch (category) {
    case SceneCategory::kRendering:
      return "Rendering";
    case SceneCategory::kPhysics:
      return "Physics";
    case SceneCategory::kAnimation:
      return "Animation";
    case SceneCategory::kGeometry:
      return "Geometry";
  }
  return "Unknown";
}

int SceneCategoryIndex(SceneCategory category) {
  for (int i = 0; i < static_cast<int>(std::size(kSceneCategories)); ++i) {
    if (kSceneCategories[i] == category) {
      return i;
    }
  }
  return 0;
}

int CountScenesInCategory(SceneCategory category) {
  int count = 0;
  for (const SceneDescriptor& descriptor : GetSceneCatalog()) {
    if (descriptor.category == category) {
      ++count;
    }
  }
  return count;
}

std::string JoinReasons(const std::vector<std::string>& reasons) {
  std::string result;
  for (size_t i = 0; i < reasons.size(); ++i) {
    if (i > 0) {
      result += " + ";
    }
    result += reasons[i];
  }
  return result;
}

bool IsSceneAvailable(const SceneDescriptor& descriptor, const cg::rhi::Capabilities& caps, std::string* reason) {
  std::vector<std::string> missing_requirements;
  if (descriptor.requires_glsl_450 && !caps.supports_glsl_450) {
    missing_requirements.push_back("GLSL 450");
  }
  if (descriptor.requires_compute && !caps.supports_compute) {
    missing_requirements.push_back("compute shaders");
  }
  if (descriptor.requires_storage_buffers && !caps.supports_storage_buffers) {
    missing_requirements.push_back("storage buffers");
  }

  if (missing_requirements.empty()) {
    if (reason != nullptr) {
      reason->clear();
    }
    return true;
  }

  if (reason != nullptr) {
    *reason = "Requires " + JoinReasons(missing_requirements);
  }
  return false;
}

const char* SceneBackendLabel(SceneBackendOption backend) {
  switch (backend) {
    case SceneBackendOption::kOpenGL:
      return "OpenGL";
    case SceneBackendOption::kVulkan:
      return "Vulkan";
  }
  return "Unknown";
}

const char* BackendTypeLabel(cg::rhi::BackendType backend) {
  switch (backend) {
    case cg::rhi::BackendType::kUnknown:
      return "Unknown";
    case cg::rhi::BackendType::kOpenGL:
      return "OpenGL";
    case cg::rhi::BackendType::kVulkan:
      return "Vulkan";
    case cg::rhi::BackendType::kMetal:
      return "Metal";
  }
  return "Unknown";
}

std::string ActiveRendererLabel(const cg::rhi::Capabilities& caps) {
  if (!caps.scene_api_name.empty()) {
    return caps.scene_api_name;
  }
  if (!caps.graphics_api_name.empty()) {
    return caps.graphics_api_name;
  }
  return BackendTypeLabel(caps.scene_backend == cg::rhi::BackendType::kUnknown ? caps.backend : caps.scene_backend);
}

std::string ActiveRuntimeApiLabel(const cg::rhi::Capabilities& caps) {
  if (!caps.graphics_api_name.empty()) {
    return caps.graphics_api_name;
  }
  return BackendTypeLabel(caps.backend);
}

bool IsBackendRuntimeActive(SceneBackendOption backend,
                            const cg::rhi::Capabilities& active_caps) {
  const cg::rhi::BackendType active_scene_backend =
      active_caps.scene_backend == cg::rhi::BackendType::kUnknown ? active_caps.backend : active_caps.scene_backend;
  switch (backend) {
    case SceneBackendOption::kOpenGL:
      return active_scene_backend == cg::rhi::BackendType::kOpenGL;
    case SceneBackendOption::kVulkan:
      return active_scene_backend == cg::rhi::BackendType::kVulkan;
  }
  return false;
}

SceneBackendAvailability GetSceneBackendAvailability(const SceneDescriptor& descriptor,
                                                     SceneBackendOption backend,
                                                     const cg::rhi::Capabilities& active_caps) {
  SceneBackendAvailability availability;
  switch (backend) {
    case SceneBackendOption::kOpenGL: {
      if (!IsBackendRuntimeActive(backend, active_caps)) {
        availability.reason = util::Format("OpenGL scene backend is not active; current scene renderer is {}",
                                           ActiveRendererLabel(active_caps));
        return availability;
      }
      availability.available = IsSceneAvailable(descriptor, active_caps, &availability.reason);
      return availability;
    }
    case SceneBackendOption::kVulkan: {
#if defined(CG_HAS_VULKAN_RHI)
      if (!IsBackendRuntimeActive(backend, active_caps)) {
        availability.reason = util::Format("Vulkan scene backend is not active; current scene renderer is {}",
                                           ActiveRendererLabel(active_caps));
        return availability;
      }
      availability.available = IsSceneAvailable(descriptor, active_caps, &availability.reason);
      return availability;
#else
      availability.reason = "Vulkan backend was not built";
      return availability;
#endif
    }
  }
  availability.reason = "Unknown backend";
  return availability;
}

bool IsSceneAvailableOnAnyBackend(const SceneDescriptor& descriptor,
                                  const cg::rhi::Capabilities& active_caps) {
  return GetSceneBackendAvailability(descriptor,
                                     SceneBackendOption::kOpenGL,
                                     active_caps).available ||
         GetSceneBackendAvailability(descriptor,
                                     SceneBackendOption::kVulkan,
                                     active_caps).available;
}

std::string SceneEntryLabel(const SceneDescriptor& descriptor, bool /*is_current_scene*/) {
  return descriptor.display_name;
}

bool DrawBackendButton(const SceneDescriptor& descriptor,
                       const cg::rhi::Capabilities& active_caps,
                       SceneBackendOption backend) {
  const SceneBackendAvailability availability = GetSceneBackendAvailability(descriptor, backend, active_caps);
  if (!availability.available) {
    ImGui::BeginDisabled();
  }
  const bool clicked = ImGui::Button(SceneBackendLabel(backend), ImVec2(74.0f, 0.0f));
  const bool hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled);
  if (!availability.available) {
    ImGui::EndDisabled();
  }
  if (hovered) {
    if (availability.available) {
      ImGui::SetTooltip("Launch %s with %s", descriptor.display_name.c_str(), SceneBackendLabel(backend));
    } else {
      ImGui::SetTooltip("%s unavailable: %s", SceneBackendLabel(backend), availability.reason.c_str());
    }
  }
  return availability.available && clicked;
}

std::string ChooseInitialSceneId() {
  const char* env_scene = std::getenv("CG_SCENE");
  if (env_scene == nullptr || env_scene[0] == '\0') {
    return kDefaultSceneId;
  }
  return env_scene;
}

int ChooseInitialCategoryIndex(const std::string& scene_id) {
  const cg::rhi::Capabilities& caps = cg::rhi::GetCapabilities();
  const SceneDescriptor* descriptor = FindSceneDescriptor(scene_id);
  if (descriptor != nullptr && IsSceneAvailableOnAnyBackend(*descriptor, caps)) {
    return SceneCategoryIndex(descriptor->category);
  }

  if (descriptor != nullptr) {
    for (int i = 0; i < static_cast<int>(std::size(kSceneCategories)); ++i) {
      const SceneCategory category = kSceneCategories[i];
      for (const SceneDescriptor& candidate : GetSceneCatalog()) {
        if (candidate.category == category && IsSceneAvailableOnAnyBackend(candidate, caps)) {
          return i;
        }
      }
    }
    return SceneCategoryIndex(descriptor->category);
  }

  return SceneCategoryIndex(SceneCategory::kRendering);
}

}  // namespace

Playground::Playground() {
  config_.Init(kConfigPath);
  pending_scene_name_ = ChooseInitialSceneId();
  selected_scene_category_index_ = ChooseInitialCategoryIndex(pending_scene_name_);
  scene_selector_popup_requested_ = true;
}

void Playground::SetPresentationRuntimeName(std::string runtime_name) {
  presentation_runtime_name_ = std::move(runtime_name);
  selected_scene_category_index_ =
      ChooseInitialCategoryIndex(current_scene_name_.empty() ? pending_scene_name_ : current_scene_name_);
}

void Playground::EnsureScene() {
  if (current_scene_ == nullptr) {
    SwitchToScene(pending_scene_name_.empty() ? kDefaultSceneId : pending_scene_name_);
    pending_scene_name_.clear();
    force_reload_scene_ = false;
  }
}

void Playground::SwitchToScene(const std::string& scene_id) {
  const SceneDescriptor* descriptor = FindSceneDescriptor(scene_id);
  if (descriptor == nullptr) {
    CGLOG(ERROR) << "Unknown scene id: " << scene_id;
    if (current_scene_ == nullptr && scene_id != kDefaultSceneId) {
      SwitchToScene(kDefaultSceneId);
    }
    return;
  }

  std::string unavailable_reason;
  if (!IsSceneAvailable(*descriptor, cg::rhi::GetCapabilities(), &unavailable_reason)) {
    CGLOG(ERROR) << "Cannot switch to scene " << descriptor->id << ": " << unavailable_reason;
    if (current_scene_ == nullptr && descriptor->id != kDefaultSceneId) {
      SwitchToScene(kDefaultSceneId);
    }
    return;
  }

  if (current_scene_ != nullptr) {
    current_scene_->Exit();
    current_scene_.reset();
  }

  current_scene_name_ = descriptor->id;
  current_scene_ = descriptor->create();
  current_scene_->Enter(descriptor->display_name, &config_, &io_, &frame_stat_);
}

void Playground::DrawSceneSelector() {
  const cg::rhi::Capabilities& caps = cg::rhi::GetCapabilities();
  const bool has_distinct_presentation_runtime =
      !presentation_runtime_name_.empty() && presentation_runtime_name_ != ActiveRendererLabel(caps);
  ImGui::Begin("Scene Launcher", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
  const SceneDescriptor* current_descriptor = FindSceneDescriptor(current_scene_name_);
  ImGui::Text("Current scene: %s", current_descriptor == nullptr ? "None" : current_descriptor->display_name.c_str());
  if (ImGui::Button("Choose Scene")) {
    scene_selector_popup_requested_ = true;
    selected_scene_category_index_ =
        ChooseInitialCategoryIndex(current_scene_name_.empty() ? pending_scene_name_ : current_scene_name_);
    show_unsupported_scenes_ = false;
  }
  if (current_descriptor != nullptr) {
    ImGui::SameLine();
    if (ImGui::Button("Reload Current Scene")) {
      pending_scene_name_ = current_scene_name_;
      force_reload_scene_ = true;
    }
  }
  ImGui::Text("Scene renderer API: %s", ActiveRendererLabel(caps).c_str());
  if (has_distinct_presentation_runtime) {
    ImGui::Text("Runtime RHI API: %s", ActiveRuntimeApiLabel(caps).c_str());
    ImGui::Text("Presentation runtime: %s", presentation_runtime_name_.c_str());
  }
  ImGui::End();

  if (scene_selector_popup_requested_) {
    ImGui::OpenPopup("Select Scene");
    scene_selector_popup_requested_ = false;
  }

  ImGui::SetNextWindowSize(ImVec2(880, 480), ImGuiCond_Appearing);
  if (!ImGui::BeginPopupModal("Select Scene", nullptr,
                              ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
    return;
  }

  ImGui::Text("Select a category and sample scene.");
  ImGui::Text("Scene renderer API: %s", ActiveRendererLabel(caps).c_str());
  if (has_distinct_presentation_runtime) {
    ImGui::Text("Runtime RHI API: %s", ActiveRuntimeApiLabel(caps).c_str());
    ImGui::Text("Presentation runtime: %s", presentation_runtime_name_.c_str());
    ImGui::TextWrapped("Scene availability is based on the renderer API above. This mode presents with %s but still "
                       "renders scenes with %s.",
                       presentation_runtime_name_.c_str(),
                       ActiveRendererLabel(caps).c_str());
  }
  if (!caps.shader_language_name.empty()) {
    ImGui::Text("Shader language: %s", caps.shader_language_name.c_str());
  }
  ImGui::Text("GLSL 450: %s", caps.supports_glsl_450 ? "available" : "unavailable");
  ImGui::Text("Compute: %s", caps.supports_compute ? "available" : "unavailable");
  ImGui::Text("Storage buffers: %s", caps.supports_storage_buffers ? "available" : "unavailable");
  ImGui::Checkbox("Show unsupported scenes", &show_unsupported_scenes_);
  ImGui::Separator();

  ImGui::BeginChild("scene_categories", ImVec2(180, -ImGui::GetFrameHeightWithSpacing() - 8.0f), true);
  for (int i = 0; i < static_cast<int>(std::size(kSceneCategories)); ++i) {
    const SceneCategory category = kSceneCategories[i];
    int available_count = 0;
    const int total_count = CountScenesInCategory(category);
    for (const SceneDescriptor& descriptor : GetSceneCatalog()) {
      if (descriptor.category == category &&
          IsSceneAvailableOnAnyBackend(descriptor, caps)) {
        ++available_count;
      }
    }
    const std::string category_label = util::Format("{} ({}/{})",
                                                    SceneCategoryLabel(category),
                                                    available_count,
                                                    total_count);
    if (ImGui::Selectable(category_label.c_str(), selected_scene_category_index_ == i)) {
      selected_scene_category_index_ = i;
    }
  }
  ImGui::EndChild();

  ImGui::SameLine();
  ImGui::BeginChild("scene_samples", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 8.0f), true);
  const SceneCategory selected_category = kSceneCategories[selected_scene_category_index_];
  bool found_scene = false;
  int hidden_scene_count = 0;
  for (const SceneDescriptor& descriptor : GetSceneCatalog()) {
    if (descriptor.category != selected_category) {
      continue;
    }

    const SceneBackendAvailability opengl_availability =
        GetSceneBackendAvailability(descriptor, SceneBackendOption::kOpenGL, caps);
    const SceneBackendAvailability vulkan_availability =
        GetSceneBackendAvailability(descriptor, SceneBackendOption::kVulkan, caps);
    const bool available_on_any_backend = opengl_availability.available || vulkan_availability.available;
    if (!available_on_any_backend && !show_unsupported_scenes_) {
      ++hidden_scene_count;
      continue;
    }
    found_scene = true;
    ImGui::PushID(descriptor.id.c_str());
    const bool is_current_scene = current_scene_name_ == descriptor.id;
    const std::string entry_label = SceneEntryLabel(descriptor, is_current_scene);

    ImGui::BeginGroup();
    ImGui::AlignTextToFramePadding();
    if (available_on_any_backend) {
      ImGui::TextUnformatted(entry_label.c_str());
    } else {
      ImGui::TextDisabled("%s", entry_label.c_str());
    }
    if (ImGui::IsItemHovered()) {
      std::string tooltip = descriptor.description;
      if (!opengl_availability.available) {
        tooltip += util::Format("\nOpenGL: {}", opengl_availability.reason);
      }
      if (!vulkan_availability.available) {
        tooltip += util::Format("\nVulkan: {}", vulkan_availability.reason);
      }
      ImGui::SetTooltip("%s", tooltip.c_str());
    }

    ImGui::SameLine(0.0f, 12.0f);
    if (DrawBackendButton(descriptor, caps, SceneBackendOption::kOpenGL)) {
      pending_scene_name_ = descriptor.id;
      force_reload_scene_ = false;
      ImGui::CloseCurrentPopup();
    }

    ImGui::SameLine(0.0f, 6.0f);
    if (DrawBackendButton(descriptor, caps, SceneBackendOption::kVulkan)) {
      pending_scene_name_ = descriptor.id;
      force_reload_scene_ = false;
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndGroup();
    ImGui::PopID();
  }

  if (!found_scene) {
    if (hidden_scene_count > 0) {
      ImGui::TextDisabled("No scenes in this category are available on any selectable backend.");
      ImGui::TextDisabled("Enable \"Show unsupported scenes\" to inspect them.");
    } else {
      ImGui::TextDisabled("No scenes in this category yet.");
    }
  }
  ImGui::EndChild();

  if (ImGui::Button("Close")) {
    ImGui::CloseCurrentPopup();
  }
  if (current_descriptor != nullptr) {
    ImGui::SameLine();
    if (ImGui::Button("Reload Current Scene##popup")) {
      pending_scene_name_ = current_scene_name_;
      force_reload_scene_ = true;
      ImGui::CloseCurrentPopup();
    }
  }

  ImGui::End();
}

void Playground::BeginFrame() {
  EnsureScene();
  frame_start_time_ = std::chrono::high_resolution_clock::now();
}

void Playground::Update() {
  EnsureScene();
  DrawSceneSelector();

  const bool should_switch_scene =
      !pending_scene_name_.empty() && (force_reload_scene_ || pending_scene_name_ != current_scene_name_);
  if (should_switch_scene) {
    SwitchToScene(pending_scene_name_);
  }
  pending_scene_name_.clear();
  force_reload_scene_ = false;

  if (current_scene_) {
    current_scene_->Update();
  }
}

void Playground::Render() {
  if (current_scene_) {
    current_scene_->Render();
  }
}

void Playground::EndFrame() {
  io_.ClearKeyInput();
  util::Time frame_end_time = std::chrono::high_resolution_clock::now();
  int64_t frame_interval_millisecond = util::DurationMillisecond(frame_start_time_, frame_end_time);
  frame_stat_.OnFrame(frame_interval_millisecond);
}

void Playground::Destoy() {
  if (current_scene_) {
    current_scene_->Exit();
    current_scene_.reset();
  }
}

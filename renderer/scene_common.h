#pragma once

#include "renderer/scene.h"
#include "renderer/texture.h"

namespace renderer {
// Put in OnUpdate before everything
class OnUpdateCommon {
 public:
  OnUpdateCommon(Scene* scene, const std::string& title);
  ~OnUpdateCommon();

 private:
  void GuiFps(Scene* scene);
  void InspectCamera(Scene* scene);
  void MoveCamera(Scene* scene);
  void InSpectCursor(Scene* scene);
  void ReloadShaders(Scene* scene);
  void InSpectObjects(Scene* scene);
  void InspectLights(Scene* scene);

  void InspectMesh(Scene* scene, const Object& object);
  void InspcetMaterial(Scene* scene, const Object& object);
};

// Put in OnRender after everything
class OnRenderCommon {
 public:
  OnRenderCommon(Scene* scene);

 private:
  void DrawViewCoord(Scene* scene);
};

class RaytracingDebugCommon {
 public:
  struct LightPath {
    LightPath();
    glm::vec4 light_path[20];
  };
  RaytracingDebugCommon(const renderer::Texture& fullscreen_texture, const Scene& scene, const LightPath& light_path);
};

template<typename ChannelType>
class TextureDebugValue {
 public:
  TextureDebugValue(const renderer::Texture& in) {
    std::vector<ChannelType> texture_data;
    in.GetTextureData<ChannelType>(&texture_data);
    int index_by_channel = 0;
    for (int width = 0; width < in.width(); ++width) {
      for (int height = 0; height < in.height(); ++height) {
        std::string pixel_info;
        pixel_info += util::Format("w~{} ", width);
        pixel_info += util::Format("h~{} ", height);
        for (int channel = 0; channel < in.channel_num(); ++channel) {
          if (channel == 0) pixel_info += util::Format("r~{} ", texture_data[index_by_channel]);
          if (channel == 1) pixel_info += util::Format("g~{} ", texture_data[index_by_channel]);
          if (channel == 2) pixel_info += util::Format("b~{} ", texture_data[index_by_channel]);
          if (channel == 3) pixel_info += util::Format("a~{} ", texture_data[index_by_channel]);
          index_by_channel++;
        }
        LOG(ERROR) << pixel_info;
      }
    }
  }
};
} // namespace renderer
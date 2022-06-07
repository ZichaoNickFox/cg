#pragma once

#include "engine/texture.h"
#include "playground/context.h"

// Put in OnUpdate before everything
class OnUpdateCommon {
 public:
  OnUpdateCommon(Context* context, const std::string& title);
  ~OnUpdateCommon();

 private:
  void GuiFps(Context* context);
  void InspectCamera(Context* context);
  void MoveCamera(Context* context);
  void InSpectCursor(Context* context);
  void ReloadShaders(Context* context);
};

// Put in OnRender after everything
class OnRenderCommon {
 public:
  OnRenderCommon(Context* context);

 private:
  void DrawWorldCoordAndViewCoord(Context* context);
};

class RaytracingDebugCommon {
 public:
  struct LightPath {
    glm::vec4 light_path[20];
  };
  RaytracingDebugCommon(const engine::Texture& in, Context* context, const LightPath& light_path);
};

template<typename ChannelType>
class TextureDebugValue {
 public:
  TextureDebugValue(const engine::Texture& in) {
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
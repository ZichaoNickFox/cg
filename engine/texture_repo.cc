#include "engine/texture_repo.h"

#include <glog/logging.h>

#include "engine/util.h"

namespace engine {
void TextureRepo::Init(const Config& config) {
  for (const TextureConfig& config : config.texture_config()) {
    State state;
    state.paths = util::ProtoRepeatedToVector(config.paths());
    state.texture_type = (Texture::Type)config.texture_type();
    textures_[config.name()] = state;
    LOG(ERROR) << "Init texture : " << config.name() << " (" << TextureType_Name(config.texture_type()) << ")";
  }
}

std::shared_ptr<Texture> TextureRepo::GetOrLoadTexture(const std::string& name) {
  CHECK(textures_.count(name) > 0) << "Cannot find texture : " << name;
  State* state = &textures_[name];
  if (!state->loaded) {
    if (state->texture_type == Texture::Texture2D) {
      CHECK(state->paths.size() == 1) << "Texture2D has 1 texture : " << name;
      state->texture = texture::LoadTexture2D(state->paths[0], true);
    } else if (state->texture_type == Texture::CubeMap) {
      state->texture = texture::LoadCubeMap(state->paths);
    } else {
      CHECK(false) << "Unsupported Texture Type";
    }
    state->loaded = true;
    LOG(ERROR) << "Loaded Texture : " << state->texture->id() << " " << name;
  }
  return state->texture;
}
}
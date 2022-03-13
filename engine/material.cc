#include "engine/material.h"

namespace engine {

void Material::AddTextureLocation(const TextureLocation& texture_location) {
  texture_locations_.push_back(texture_location);
}

}
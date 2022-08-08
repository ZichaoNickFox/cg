#pragma once

#include <string>

#include "renderer/config.h"
#include "renderer/material.h"
#include "renderer/mesh.h"

namespace cg {

bool LoadModel(const Config& config, const std::string& name, MeshRepo* mesh_repo,
               MaterialRepo* material_repo, TextureRepo* texture_repo);

} // namespace cg
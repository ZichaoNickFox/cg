#include "engine/repo/mesh_repo.h"

#include <algorithm>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <glog/logging.h>
#include <math.h>
#include <set>
#include "SOIL2/SOIL2.h"
#include "SOIL2/stb_image.h"

#include "engine/debug.h"
#include "engine/primitive/cube_mesh.h"
#include "engine/primitive/plane_mesh.h"
#include "engine/primitive/sphere_mesh.h"
#include "engine/util.h"

namespace engine {
void MeshRepo::Init(const Config& config) {
  CGLOG(ERROR) << "Init Mesh : cube";
  meshes_["cube"] = std::make_shared<const CubeMesh>();
  CGLOG(ERROR) << "Init Mesh : Plane";
  meshes_["plane"] = std::make_shared<const PlaneMesh>();
  CGLOG(ERROR) << "Init Mesh : Sphere";
  meshes_["sphere"] = std::make_shared<const SphereMesh>();
}

std::shared_ptr<const engine::Mesh> MeshRepo::GetOrLoadMesh(const std::string& name) {
  CGCHECK(meshes_.count(name) > 0) << " Cannot find in mesh repo : " << name;
  return meshes_[name];
}
} // namespace engine
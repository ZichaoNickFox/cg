#include "playground/mesh_repo.h"

#include <algorithm>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <glog/logging.h>
#include <math.h>
#include <set>
#include "SOIL2/SOIL2.h"
#include "SOIL2/stb_image.h"

#include "engine/debug.h"
#include "playground/primitive/cube_mesh.h"
#include "playground/primitive/plane_mesh.h"
#include "playground/primitive/sphere_mesh.h"
#include "playground/util.h"

void MeshRepo::Init(const Config& config) {
  CGLOG(ERROR) << "Init Mesh : cube";
  meshes_["cube"] = std::make_shared<CubeMesh>();
  meshes_["cube"]->Setup();
  CGLOG(ERROR) << "Init Mesh : Plane";
  meshes_["plane"] = std::make_shared<PlaneMesh>();
  meshes_["plane"]->Setup();
  CGLOG(ERROR) << "Init Mesh : Sphere";
  meshes_["sphere"] = std::make_shared<SphereMesh>();
  meshes_["sphere"]->Setup();
}

std::shared_ptr<engine::Mesh> MeshRepo::GetOrLoadMesh(const std::string& name) {
  CGCHECK(meshes_.count(name) > 0) << " Cannot find in mesh repo : " << name;
  return meshes_[name];
}
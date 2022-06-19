#include "playground/object/object.h"

#include <glm/gtx/string_cast.hpp>

bool Object::Intersect(Context* context, const glm::vec3& origin_ws, const glm::vec3& direction_ws,
                       Object::IntersectResult* result) {
  glm::mat4 model_matrix = transform().GetModelMatrix();
  glm::mat4 inverse_model = glm::inverse(model_matrix);
  glm::vec3 origin_ls = inverse_model * glm::vec4(origin_ws, 1.0);
  glm::vec3 direction_ls = inverse_model * glm::vec4(direction_ws, 0.0);
  glm::vec3 vertex0_ls, vertex1_ls, vertex2_ls, normal_ls, position_ls;
  float distance_ls;
  if (!CGCHECK_NOTNULL(GetMesh(context))->Intersect(origin_ls, direction_ls, &position_ls, &normal_ls, &distance_ls,
                                                 &vertex0_ls, &vertex1_ls, &vertex2_ls)) {
    return false;
  }
  result->position_ws = model_matrix * glm::vec4(position_ls, 1.0);
  result->normal_ws = model_matrix * glm::vec4(normal_ls, 0.0);
  result->distance_ws = distance_ls;
  result->vertex0_ws = model_matrix * glm::vec4(vertex0_ls, 1.0);
  result->vertex1_ws = model_matrix * glm::vec4(vertex1_ls, 1.0);
  result->vertex2_ws = model_matrix * glm::vec4(vertex2_ls, 1.0);
  return true;
}

void Object::GetPrimitives(Context* context, engine::Primitives* primitives) {
  std::shared_ptr<const engine::Mesh> mesh = GetMesh(context);
  const std::vector<glm::vec3>& positions = mesh->positions();
  std::vector<glm::vec3> world_positions(positions.size());
  glm::mat4 model = transform_.GetModelMatrix();
  for (int i = 0; i < positions.size(); ++i) {
    world_positions[i] = model * glm::vec4(positions[i], 1.0);
  }
  if (mesh->indices().size() > 0) {
    CGCHECK(mesh->indices().size() % 3 == 0);
    int index = 0;
    for (int i = 0, j = 1, k = 2; k < mesh->indices().size(); i += 3, j += 3, k += 3) {
      int mesh_index_i = mesh->indices()[i];
      int mesh_index_j = mesh->indices()[j];
      int mesh_index_k = mesh->indices()[k];
      engine::Triangle triangle{world_positions[mesh_index_i], world_positions[mesh_index_j],
                                world_positions[mesh_index_k]};
      primitives->PushTriangle(triangle);
    }
  } else {
    int index = 0;
    for (int i = 0, j = 1, k = 2; k < mesh->positions().size(); i += 3, j += 3, k += 3) {
      engine::Triangle triangle{world_positions[i], world_positions[j], world_positions[k]};
      primitives->PushTriangle(triangle);
    }
  }
}
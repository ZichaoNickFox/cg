#include "playground/object/object.h"

#include <glm/gtx/string_cast.hpp>

bool Object::Intersect(Context* context, const glm::vec3& origin_ws, const glm::vec3& direction_ws,
                       Object::IntersectResult* result) {
  glm::mat4 model_matrix = transform().GetModelMatrix();
  glm::mat4 inverse_model = glm::inverse(model_matrix);
  glm::vec3 origin_ls = inverse_model * glm::vec4(origin_ws, 1.0);
  glm::vec3 direction_ls = inverse_model * glm::vec4(origin_ws + direction_ws, 1.0) - glm::vec4(origin_ls,1.0);

  glm::vec3 vertex0_ls, vertex1_ls, vertex2_ls, normal_ls, position_ls;
  float distance_ls;
  if (!CGCHECK_NOTNULL(mesh(context))->Intersect(origin_ls, direction_ls, &position_ls, &normal_ls, &distance_ls,
                                                 &vertex0_ls, &vertex1_ls, &vertex2_ls)) {
    return false;
  }
  result->position_ws = model_matrix * glm::vec4(position_ls, 1.0);
  result->normal_ws = model_matrix * glm::vec4(position_ls + normal_ls, 1.0) - glm::vec4(result->position_ws, 1.0);
  result->distance_ws = distance_ls;
  result->vertex0_ws = model_matrix * glm::vec4(vertex0_ls, 1.0);
  result->vertex1_ws = model_matrix * glm::vec4(vertex1_ls, 1.0);
  result->vertex2_ws = model_matrix * glm::vec4(vertex2_ls, 1.0);
  return true;
}
#include "playground/object/coord.h"

Coord::Coord() {
  std::vector<glm::vec3> positions{glm::vec3(0, 0, 0), glm::vec3(2, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 2, 0),
                                   glm::vec3(0, 0, 0), glm::vec3(0, 0, 2)};
  std::vector<glm::vec3> colors{glm::vec3(1, 0, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0),
                                glm::vec3(0, 0, 1), glm::vec3(0, 0, 1)};
  lines_.SetData({positions, colors, GL_LINES, 5});
}

void Coord::OnUpdate(Context *context) {
  lines_.OnUpdate(context);
}

void Coord::OnRender(Context *context) {
  lines_.OnRender(context);
}

void Coord::OnDestory(Context *context) {
  lines_.OnDestory(context);
}
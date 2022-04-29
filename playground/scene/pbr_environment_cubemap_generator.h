#pragma once

#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/glm.hpp>

#include "engine/camera.h"
#include "engine/color_frame_buffer.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/cube.h"
#include "playground/object/empty_object.h"
#include "playground/scene.h"
#include "playground/shaders.h"

class PbrEnvironmentCubemapGenerator : public Scene {
 public:
  void OnEnter(Context* context);
  void OnUpdate(Context* context);
  void OnRender(Context* contexnt);
  void OnExit(Context* context);

 private:
  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();
  Cube cube_;

  engine::Camera cubemap_cameras_[6];
  std::vector<glm::quat> rotations_ = {
    glm::angleAxis(float(M_PI) / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f)), // left
    glm::angleAxis(float(M_PI) / 2.0f, glm::vec3(0.0f, -1.0f, 0.0f)), // right
    glm::angleAxis(float(M_PI) / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f)), // top
    glm::angleAxis(float(M_PI) / 2.0f, glm::vec3(-1.0f, 0.0f, 0.0f)), // bottom
    glm::angleAxis(float(M_PI), glm::vec3(0.0f, 1.0f, 0.0f)), // front
    glm::quat(), // back
  };

  engine::ColorFrameBuffer color_frame_buffer_;
};
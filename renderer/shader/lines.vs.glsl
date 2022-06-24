#include "renderer/shader/version.glsl"

#include "renderer/shader/camera.glsl"

layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 color;

uniform Camera camera;
uniform mat4 model;

out vec3 color_;
out vec3 pos_ws_;

void main()
{
  color_ = color.xyz;
  gl_Position = camera.project * camera.view * model * pos;
  pos_ws_ = vec3(model * pos);
}
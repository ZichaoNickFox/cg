#version 430 core

layout (location = 0) in vec3 position_ls;
layout (location = 1) in vec3 normal_ls;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in vec2 tangent;
layout (location = 4) in vec2 bitangent;
layout (location = 5) in vec4 model0;
layout (location = 6) in vec4 model1;
layout (location = 7) in vec4 model2;
layout (location = 8) in vec4 model3;

uniform mat4 view;
uniform mat4 project;

out vec2 texcoord_;

void main() {
  mat4 model = mat4(model0, model1, model2, model3);
  texcoord_ = texcoord;
  gl_Position = project * view * model * vec4(position_ls, 1.0);
}

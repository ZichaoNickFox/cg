#include "renderer/shader/version.glsl"

#include "renderer/shader/random.glsl"
#include "renderer/shader/sample.glsl"

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

layout (std430, binding = 0) buffer Samples { vec4 samples[500]; };

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;

out vec3 local_pos_;
out vec2 texcoord_;

void main()
{
  local_pos_ = pos;
  gl_Position = project * view * model * vec4(pos, 1.0);

  InitRNG(pos.xy);
  for (int i = 0; i < 500; ++i) {
    vec3 a_sample = SampleUnitHemisphereDir(normalize(vec3(0, 1, 0)));
    samples[i] = vec4(a_sample, 1.0);
  }

}
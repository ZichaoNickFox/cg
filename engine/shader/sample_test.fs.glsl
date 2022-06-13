#include "engine/shader/version.glsl"

#include "engine/shader/convert.glsl"
#include "engine/shader/sample.glsl"

layout (std430, binding = 0) buffer Samples { vec4 samples[500]; };

out vec4 FragColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;

in vec3 local_pos_;

void TestHammersley() {
  const float pi = 3.1415926;
  const uint N = 100;
  FragColor = vec4(0, 0, 1, 1);
  for (int i = 0; i < N; ++i) {
    vec2 s = Hammersley(i, N);
    vec2 uv = SpherePos2UV(local_pos_);
    if (distance(s, uv) < 0.005) {
      FragColor  = vec4(1, 0, 0, 1);
    }
  }
}

void TestSampleUnitHemisphereDir() {
  for (int i = 0; i < 500; ++i) {
    if (distance(samples[i].xyz, local_pos_) < 0.05) {
      FragColor = vec4(1, 0, 0, 1);
    }
  }
}

void main() {
//  TestHammersley();
  TestSampleUnitHemisphereDir();
}
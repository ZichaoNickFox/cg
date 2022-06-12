#include "engine/shader/version.glsl"

#include "engine/shader/convert.glsl"
#include "engine/shader/sample.glsl"

out vec4 FragColor;

uniform mat4 view;
uniform mat4 project;

in vec3 local_pos_;
in vec2 texcoord_;

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
  for(int i = 0; i < 100; ++i) {
    vec3 p_ws = SampleUnitHemisphereDir(vec3(-1, -1, -1));
    vec3 p_ss = PositionWS2SS(p_ws, view, project);
    if (distance(texcoord_, p_ss.xy) < 0.005) {
      FragColor  = vec4(1, 0, 0, 1);
    } else {
      FragColor = vec4(0, 1, 0, 1);
    }
  }
}

void main() {
//  TestHammersley();
  TestSampleUnitHemisphereDir();
}
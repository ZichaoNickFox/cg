#include "engine/shader/version.glsl"
#include "engine/shader/random.glsl"
#include "engine/shader/sample.glsl"

out vec4 FragColor;

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
  for(int i = 0; i < ) {
  }
}

void main() {
//  TestHammersley();
  TestSampleUnitHemisphereDir();
}
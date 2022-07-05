#include "renderer/shader/version.glsl"

#include "renderer/shader/camera.glsl"
#include "renderer/shader/definition.glsl"
#include "renderer/shader/random.glsl"
#include "renderer/shader/sample.glsl"
#include "renderer/shader/transform.glsl"

layout (location = 0) in vec3 pos;

layout (std430, binding = SSBO_USER_0) buffer Samples { vec4 samples[500]; };

uniform mat4 model;
uniform Camera camera;

out vec3 local_pos_;

void main()
{
  local_pos_ = pos;
  gl_Position = PositionLS2CS(model, camera.view, camera.project, pos);

  InitRNG(pos.xy);
  for (int i = 0; i < 500; ++i) {
    vec3 a_sample = SampleUnitHemisphereDir(normalize(vec3(0, 0, -1)));
    samples[i] = vec4(a_sample, 1.0);
  }
}
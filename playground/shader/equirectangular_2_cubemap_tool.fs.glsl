#include "renderer/shader/version.glsl"

#include "renderer/shader/random.glsl"
#include "renderer/shader/sample.glsl"

out vec4 FragColor;
in vec3 local_pos_;

uniform sampler2D texture2D0;

void main() {
  // After normalize, local_pos coverted to inscribed sphere
  vec2 uv = SpherePos2UV(normalize(local_pos_));
  vec3 color = texture(texture2D0, uv).rgb;
  FragColor = vec4(color, 1.0);
}